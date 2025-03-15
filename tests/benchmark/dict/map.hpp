#pragma once

#include <boost/interprocess/file_mapping.hpp>
#include <boost/interprocess/mapped_region.hpp>
#include <cstdint>
#include <fstream>
#include <ios>
#include <iostream>
#include <string>
#include <unordered_map>
#include <vector>

class MmapStringMap {
  struct FileHeader {
    uint64_t magic;           // Magic number for validation
    uint64_t count;           // Number of key-value pairs
    uint64_t strings_offset;  // Offset to strings pool
    uint64_t strings_size;    // Total size of strings pool
  };

  struct StringRef {
    size_t offset;  // Offset in strings pool
    size_t length;  // Length of string
  };

  struct KeyValuePair {
    StringRef key;
    StringRef value;
  };

  static constexpr uint64_t MAGIC = 0x4D4150535452494EUL;  // "MAPSTRIN" in hex

  std::string stringsPool_;
  std::vector<KeyValuePair> pairs_;
  std::unordered_map<std::string_view, const StringRef&> map_;

public:
  std::unordered_map<std::string, std::string> exportToMap() {
    std::unordered_map<std::string, std::string> result;
    result.reserve(pairs_.size());

    for (const auto& [key, value] : pairs_) {
      std::string strKey(stringsPool_.data() + key.offset, key.length);
      std::string strVal(stringsPool_.data() + value.offset, value.length);
      result.try_emplace(strKey, strVal);
    }
    return result;
  }

  void reserve(size_t size) {
    pairs_.reserve(size);
    map_.reserve(size);
  }

  void add(const std::string& key, const std::string& value) {
    size_t offset = stringsPool_.size();
    stringsPool_.append(key);
    stringsPool_.append(value);

    StringRef refVal = {offset + key.size(), value.size()};
    KeyValuePair pair = {
        {offset, key.size()},
        refVal,
    };

    pairs_.push_back(pair);

    std::string_view keyView(stringsPool_.data() + offset, key.size());
    map_.try_emplace(keyView, refVal);
  }

  void save(const std::string& filename) {
    const size_t totalPairs = pairs_.size();
    const size_t headerSize = sizeof(FileHeader);
    const size_t pairsSize = sizeof(KeyValuePair) * totalPairs;

    const size_t totalSize = headerSize + pairsSize + stringsPool_.size();

    // Create and size the file in one operation
    {
      std::ofstream file(filename, std::ios::binary | std::ios::trunc);
      file.exceptions(std::ios::failbit | std::ios::badbit);
      file.seekp(static_cast<std::streamoff>(totalSize - 1));
      file.put(0);
    }

    // Map the entire file at once
    boost::interprocess::file_mapping fileMap(filename.c_str(), boost::interprocess::read_write);
    boost::interprocess::mapped_region region(fileMap, boost::interprocess::read_write, 0,
                                              totalSize);
    char* const baseAddr = static_cast<char*>(region.get_address());

    // Write header
    auto* header = reinterpret_cast<FileHeader*>(baseAddr);
    header->magic = MAGIC;
    header->count = totalPairs;
    header->strings_offset = headerSize + pairsSize;
    header->strings_size = stringsPool_.size();

    char* strings = baseAddr + header->strings_offset;
    std::memcpy(strings, stringsPool_.data(), stringsPool_.size());
    std::memcpy(baseAddr + headerSize, pairs_.data(), pairsSize);

    region.flush();
  }

  void load(const std::string& filename) {
    boost::interprocess::mapped_region region(
        boost::interprocess::file_mapping(filename.c_str(), boost::interprocess::read_only),
        boost::interprocess::read_only);

    const auto* header = static_cast<const FileHeader*>(region.get_address());
    if (header->magic != MAGIC) {
      throw std::runtime_error("Invalid file format");
    }

    const char* stringsBase =
        static_cast<const char*>(region.get_address()) + header->strings_offset;
    stringsPool_.assign(stringsBase, header->strings_size);

    pairs_.reserve(header->count);

    const auto* pairs = reinterpret_cast<const KeyValuePair*>(
        static_cast<const char*>(region.get_address()) + sizeof(FileHeader));

    for (size_t i = 0; i < header->count; ++i) {
      const auto& pair = pairs[i];
      pairs_.push_back(pair);

      std::string_view keyView(stringsBase + pair.key.offset, pair.key.length);
      const auto& value = pair.value;
      map_.try_emplace(keyView, value);
    }
  }
};
