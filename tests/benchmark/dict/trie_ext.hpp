#include <marisa.h>

#include <cstdint>
#include <filesystem>
#include <fstream>
#include <ios>
#include <iostream>
#include <stdexcept>
#include <string>
#include <string_view>
#include <vector>
// mmap headers
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>

#include <yas/binary_iarchive.hpp>
#include <yas/binary_oarchive.hpp>
#include <yas/serialize.hpp>
#include <yas/std_types.hpp>

#include "trie.h"

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

  void loadBinaryFileYas(std::string_view filePath) {
    // Read the entire file into a buffer first
    std::ifstream ifs(std::string(filePath), std::ios::binary | std::ios::ate);
    if (!ifs) {
      throw std::runtime_error("Failed to open file");
    }

    const auto size = ifs.tellg();
    ifs.seekg(0);

    std::vector<char> buffer(size);
    if (!ifs.read(buffer.data(), size)) {
      throw std::runtime_error("Failed to read file");
    }

    // Use memory buffer for deserialization
    yas::mem_istream ms(buffer.data(), buffer.size());
    constexpr std::uint32_t YAS_FLAGS = yas::binary | yas::no_header;
    yas::binary_iarchive<yas::mem_istream, YAS_FLAGS> ia(ms);

    ia& getData();

    // Read and load trie
    std::vector<char> trieData;
    ia & trieData;

    ScopedTempFile tempFile{filePath};
    {
      std::ofstream trieOut(tempFile.path(), std::ios::binary);
      trieOut.write(trieData.data(), static_cast<std::streamsize>(trieData.size()));
    }
    getTrie().load(tempFile.path().c_str());
  }

  void saveToBinaryFileYas(std::string_view filePath) {
    // Use memory buffer for serialization
    yas::mem_ostream ms;
    constexpr std::uint32_t YAS_FLAGS = yas::binary | yas::no_header;
    yas::binary_oarchive<yas::mem_ostream, YAS_FLAGS> oa(ms);

    oa& getData();

    // Handle trie data
    ScopedTempFile tempFile{filePath};
    getTrie().save(tempFile.path().c_str());

    std::vector<char> trieData;
    {
      std::ifstream trieFile(tempFile.path(), std::ios::binary);
      trieData.assign(std::istreambuf_iterator<char>(trieFile), std::istreambuf_iterator<char>());
    }
    oa & trieData;

    // Write the serialized data to file
    auto buf = ms.get_shared_buffer();
    std::ofstream ofs(std::string(filePath), std::ios::binary);
    if (!ofs) {
      throw std::runtime_error("Failed to open file for writing");
    }
    ofs.write(buf.data.get(), static_cast<std::streamsize>(buf.size));
  }
};
