#include <marisa.h>

#include <filesystem>
#include <fstream>
#include <ios>
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>
// mmap headers
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>

#include "dicts/trie.h"

class TrieWithStringExt : public rime::Trie {
public:
  void saveToFiles(const std::string& dataPath) {
    const auto triePath = dataPath + ".trie";
    getTrie().save(triePath.c_str());

    // Save associated string data
    std::ofstream dataFile(dataPath, std::ios::binary);
    if (!dataFile) {
      throw std::runtime_error("Failed to open data file for writing");
    }

    size_t size = getData().size();
    dataFile.write(reinterpret_cast<const char*>(&size), sizeof(size));

    // Write each string with its length
    for (const auto& str : getData()) {
      size_t strLen = str.length();
      dataFile.write(reinterpret_cast<const char*>(&strLen), sizeof(strLen));
      dataFile.write(str.data(), static_cast<std::streamsize>(strLen));
    }
  }

  void loadFromFiles(const std::string& dataPath) {
    // Load trie data
    std::string triePath = dataPath + ".trie";
    getTrie().load(triePath.c_str());

    auto& data = getData();

    // Load associated string data
    std::ifstream dataFile(dataPath, std::ios::binary);
    if (!dataFile) {
      throw std::runtime_error("Failed to open data file for reading");
    }

    size_t size = 0;
    dataFile.read(reinterpret_cast<char*>(&size), sizeof(size));
    data.resize(size);

    // Read each string
    for (size_t i = 0; i < size; ++i) {
      size_t strLen = 0;
      dataFile.read(reinterpret_cast<char*>(&strLen), sizeof(strLen));

      std::string str(strLen, '\0');
      dataFile.read(str.data(), static_cast<std::streamsize>(strLen));
      data[i] = std::move(str);
    }

    if (dataFile.fail()) {
      throw std::runtime_error("Failed to read data from file");
    }
  }

  void loadFromSingleFile(const std::string& filePath) {
    std::ifstream file(filePath, std::ios::binary);
    if (!file) {
      throw std::runtime_error("Failed to open file for reading");
    }

    IOUtil::readVectorData(file, getData());

    // Read and load trie
    size_t trieSize = 0;
    file.read(reinterpret_cast<char*>(&trieSize), sizeof(trieSize));

    ScopedTempFile tempFile{filePath};
    {
      std::ofstream trieFile(tempFile.path(), std::ios::binary);
      std::vector<char> buffer(trieSize);
      file.read(buffer.data(), static_cast<std::streamsize>(trieSize));
      trieFile.write(buffer.data(), static_cast<std::streamsize>(trieSize));
    }

    getTrie().load(tempFile.path().c_str());

    if (file.fail()) {
      throw std::runtime_error("Failed to read data from file");
    }
  }
};
