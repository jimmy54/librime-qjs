#include "trie.h"

#include <boost/interprocess/file_mapping.hpp>
#include <boost/interprocess/mapped_region.hpp>
#include <filesystem>
#include <fstream>

namespace rime {

void Trie::loadBinaryFileMmap(std::string_view filePath) {
  boost::interprocess::file_mapping mapping(std::string(filePath).c_str(),
                                            boost::interprocess::read_only);
  boost::interprocess::mapped_region region(mapping, boost::interprocess::read_only);

  const char* current = static_cast<const char*>(region.get_address());
  const char* end = current + region.get_size();

  // Read data vector size
  size_t dataSize = 0;
  std::memcpy(&dataSize, current, sizeof(dataSize));
  current += sizeof(dataSize);

  data_.resize(dataSize);

  // Read strings
  for (size_t i = 0; i < dataSize && current < end; ++i) {
    size_t strLen = 0;
    std::memcpy(&strLen, current, sizeof(strLen));
    current += sizeof(strLen);

    if (current + strLen > end) {
      throw std::runtime_error("Corrupted data file");
    }

    data_[i].assign(current, strLen);
    current += strLen;
  }

  // Read and load trie
  size_t trieSize = 0;
  std::memcpy(&trieSize, current, sizeof(trieSize));
  current += sizeof(trieSize);

  off_t offset = current - static_cast<const char*>(region.get_address());
#ifdef _WIN32
  SetFilePointer(mapping.get_mapping_handle(), offset, nullptr, FILE_BEGIN);
#else
  ::lseek(mapping.get_mapping_handle().handle, offset, SEEK_SET);
#endif
  trie_.read(mapping.get_mapping_handle().handle);
}

void Trie::loadTextFile(const std::string& txtPath, size_t entrySize) {
  std::vector<std::pair<std::string, std::string>> items(entrySize);
  std::ifstream infile(txtPath);
  std::string line;
  size_t idx = 0;
  while (std::getline(infile, line)) {
    if (!line.empty() && line[0] == '#') {
      continue;
    }

    size_t tabPos = line.find('\t');
    if (tabPos != std::string::npos) {
      auto lineView = std::string_view(line);
      std::string_view key = lineView.substr(0, tabPos);
      std::string_view value = lineView.substr(tabPos + 1);

      if (!value.empty() && value.back() == '\r') {
        value.remove_suffix(1);
      }

      if (idx < entrySize) {
        items[idx++] = std::make_pair(std::string(key), std::string(value));
      } else {
        items.emplace_back(std::string(key), std::string(value));
      }
    }
  }
  build(items);
}

void Trie::saveToBinaryFile(const std::string& filePath) const {
  std::ofstream file(filePath, std::ios::binary);
  if (!file) {
    throw std::runtime_error("Failed to open file for writing");
  }

  IOUtil::writeVectorData(file, data_);

  // Handle trie data
  ScopedTempFile tempFile{filePath};
  trie_.save(tempFile.path().c_str());

  std::ifstream trieFile(tempFile.path(), std::ios::binary | std::ios::ate);
  const size_t trieSize = trieFile.tellg();
  trieFile.seekg(0);

  file.write(reinterpret_cast<const char*>(&trieSize), sizeof(trieSize));
  file << trieFile.rdbuf();
}

void Trie::add(const std::string& key, const std::string& value) {
  marisa::Keyset keyset;
  keyset.push_back(key.c_str(), key.length());

  // Build the trie
  trie_.build(keyset, MARISA_BINARY_TAIL);  // UTF-8 support

  // Get the ID for the key
  marisa::Agent agent;
  agent.set_query(key.c_str(), key.length());

  if (trie_.lookup(agent)) {
    std::size_t id = agent.key().id();

    // Resize data vector if necessary
    if (id >= data_.size()) {
      data_.resize(id + 1);
    }

    // Store the associated data
    data_[id] = value;
  } else {
    throw std::runtime_error("Failed to add key-value pair");
  }
}

void Trie::build(const std::vector<std::pair<std::string, std::string>>& items) {
  marisa::Keyset keyset;

  // First, add all keys to the keyset
  for (const auto& [key, _] : items) {
    keyset.push_back(key.c_str(), key.length());
  }

  // Build the trie
  trie_.build(keyset, MARISA_BINARY_TAIL);  // UTF-8 support

  // Resize data vector to accommodate all values
  data_.resize(items.size());

  // Store all values
  for (const auto& [key, value] : items) {
    marisa::Agent agent;
    agent.set_query(key.c_str(), key.length());

    if (trie_.lookup(agent)) {
      data_[agent.key().id()] = value;
    } else {
      throw std::runtime_error("Failed to add key-value pair");
    }
  }
}

std::optional<std::string> Trie::find(std::string_view key) const {
  marisa::Agent agent;
  agent.set_query(key.data(), key.length());

  if (trie_.lookup(agent)) {
    std::size_t id = agent.key().id();
    if (id < data_.size()) {
      return data_[id];
    }
  }
  return std::nullopt;
}

bool Trie::contains(std::string_view key) const {
  marisa::Agent agent;
  agent.set_query(key.data(), key.length());
  return trie_.lookup(agent);
}

std::vector<std::pair<std::string, std::string>> Trie::prefixSearch(std::string_view prefix) const {
  std::vector<std::pair<std::string, std::string>> results;
  marisa::Agent agent;
  agent.set_query(prefix.data(), prefix.length());

  while (trie_.predictive_search(agent)) {
    std::string key(agent.key().ptr(), agent.key().length());
    std::size_t id = agent.key().id();
    if (id < data_.size()) {
      results.emplace_back(key, data_[id]);
    }
  }
  return results;
}

}  // namespace rime
