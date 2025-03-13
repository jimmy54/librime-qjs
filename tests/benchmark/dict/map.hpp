#pragma once

#include <boost/interprocess/file_mapping.hpp>
#include <boost/interprocess/mapped_region.hpp>
#include <cstdint>
#include <fstream>
#include <string>
#include <unordered_map>

class MmapStringMap {
  struct FileHeader {
    uint64_t magic;           // Magic number for validation
    uint64_t count;           // Number of key-value pairs
    uint64_t strings_offset;  // Offset to strings pool
    uint64_t strings_size;    // Total size of strings pool
  };

  struct StringRef {
    uint64_t offset;  // Offset in strings pool
    uint32_t length;  // Length of string
  };

  struct KeyValuePair {
    StringRef key;
    StringRef value;
  };

  static constexpr uint64_t MAGIC = 0x4D4150535452494EUL;  // "MAPSTRIN" in hex

public:
  void save(const std::string& filename, const std::unordered_map<std::string, std::string>& data) {
    // Calculate required size in a single pass
    size_t strings_size = 0;
    const size_t total_pairs = data.size();
    const size_t header_size = sizeof(FileHeader);
    const size_t pairs_size = sizeof(KeyValuePair) * total_pairs;

    for (const auto& [key, value] : data) {
      strings_size += key.size() + value.size();
    }

    const size_t total_size = header_size + pairs_size + strings_size;

    // Create and size the file in one operation
    {
      std::ofstream file(filename, std::ios::binary | std::ios::trunc);
      file.exceptions(std::ios::failbit | std::ios::badbit);
      file.seekp(total_size - 1);
      file.put(0);
    }

    // Map the entire file at once
    boost::interprocess::file_mapping file_map(filename.c_str(), boost::interprocess::read_write);
    boost::interprocess::mapped_region region(file_map, boost::interprocess::read_write, 0,
                                              total_size);
    char* const base_addr = static_cast<char*>(region.get_address());

    // Write header
    auto* header = reinterpret_cast<FileHeader*>(base_addr);
    header->magic = MAGIC;
    header->count = total_pairs;
    header->strings_offset = header_size + pairs_size;
    header->strings_size = strings_size;

    // Setup pointers for direct memory access
    auto* pairs = reinterpret_cast<KeyValuePair*>(base_addr + header_size);
    char* strings = base_addr + header->strings_offset;
    uint64_t current_offset = 0;

    // Write data in a single pass
    size_t i = 0;
    for (const auto& [key, value] : data) {
      const size_t key_size = key.size();
      const size_t value_size = value.size();

      pairs[i].key = {current_offset, static_cast<uint32_t>(key_size)};
      std::memcpy(strings + current_offset, key.data(), key_size);
      current_offset += key_size;

      pairs[i].value = {current_offset, static_cast<uint32_t>(value_size)};
      std::memcpy(strings + current_offset, value.data(), value_size);
      current_offset += value_size;
      ++i;
    }

    region.flush();
  }

  std::unordered_map<std::string, std::string> load(const std::string& filename) {
    boost::interprocess::mapped_region region(
        boost::interprocess::file_mapping(filename.c_str(), boost::interprocess::read_only),
        boost::interprocess::read_only);

    auto* header = static_cast<const FileHeader*>(region.get_address());
    if (header->magic != MAGIC)
      throw std::runtime_error("Invalid file format");

    std::unordered_map<std::string, std::string> result;
    result.reserve(header->count);

    auto* pairs = reinterpret_cast<const KeyValuePair*>(
        static_cast<const char*>(region.get_address()) + sizeof(FileHeader));
    const char* strings = static_cast<const char*>(region.get_address()) + header->strings_offset;

    for (size_t i = 0; i < header->count; ++i) {
      result.emplace(std::string(strings + pairs[i].key.offset, pairs[i].key.length),
                     std::string(strings + pairs[i].value.offset, pairs[i].value.length));
    }
    return result;
  }
};
