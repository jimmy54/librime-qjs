#pragma once

#include <cstdint>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <vector>

#ifdef _WIN32
#include <windows.h>
#else
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>
#endif

class MmapStringMap {
public:
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

private:
  static constexpr uint64_t MAGIC = 0x4D4150535452494EUL;  // "MAPSTRIN" in hex

#ifdef _WIN32
  HANDLE file_handle_ = INVALID_HANDLE_VALUE;
  HANDLE mapping_handle_ = NULL;
#else
  int fd_ = -1;
#endif
  void* mapped_data_ = nullptr;
  size_t mapped_size_ = 0;

public:
  MmapStringMap() = default;
  ~MmapStringMap() { close(); }

  // Prevent copying
  MmapStringMap(const MmapStringMap&) = delete;
  MmapStringMap& operator=(const MmapStringMap&) = delete;

  void save(const std::string& filename, const std::unordered_map<std::string, std::string>& data) {
    // Calculate required size
    size_t strings_size = 0;
    for (const auto& [key, value] : data) {
      strings_size += key.size() + value.size();
    }

    const size_t header_size = sizeof(FileHeader);
    const size_t pairs_size = sizeof(KeyValuePair) * data.size();
    const size_t total_size = header_size + pairs_size + strings_size;

#ifdef _WIN32
    file_handle_ = CreateFileA(filename.c_str(), GENERIC_READ | GENERIC_WRITE, 0, NULL,
                               CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    if (file_handle_ == INVALID_HANDLE_VALUE) {
      throw std::runtime_error("Failed to create file");
    }

    LARGE_INTEGER size;
    size.QuadPart = total_size;
    if (!SetFilePointerEx(file_handle_, size, NULL, FILE_BEGIN) || !SetEndOfFile(file_handle_)) {
      CloseHandle(file_handle_);
      throw std::runtime_error("Failed to set file size");
    }

    mapping_handle_ =
        CreateFileMappingA(file_handle_, NULL, PAGE_READWRITE, size.HighPart, size.LowPart, NULL);
    if (!mapping_handle_) {
      CloseHandle(file_handle_);
      throw std::runtime_error("Failed to create file mapping");
    }

    mapped_data_ = MapViewOfFile(mapping_handle_, FILE_MAP_WRITE, 0, 0, total_size);
#else
    fd_ = open(filename.c_str(), O_RDWR | O_CREAT | O_TRUNC, 0644);
    if (fd_ == -1) {
      throw std::runtime_error("Failed to create file");
    }

    if (ftruncate(fd_, total_size) == -1) {
      ::close(fd_);
      throw std::runtime_error("Failed to set file size");
    }

    mapped_data_ = mmap(nullptr, total_size, PROT_READ | PROT_WRITE, MAP_SHARED, fd_, 0);
    if (mapped_data_ == MAP_FAILED) {
      ::close(fd_);
      throw std::runtime_error("Failed to map file");
    }
#endif
    mapped_size_ = total_size;

    // Write header
    auto* header = static_cast<FileHeader*>(mapped_data_);
    header->magic = MAGIC;
    header->count = data.size();
    header->strings_offset = header_size + pairs_size;
    header->strings_size = strings_size;

    // Write key-value pairs and strings
    auto* pairs =
        reinterpret_cast<KeyValuePair*>(static_cast<char*>(mapped_data_) + sizeof(FileHeader));
    char* strings = static_cast<char*>(mapped_data_) + header->strings_offset;
    uint64_t current_offset = 0;

    size_t i = 0;
    for (const auto& [key, value] : data) {
      // Write key
      pairs[i].key.offset = current_offset;
      pairs[i].key.length = key.size();
      memcpy(strings + current_offset, key.data(), key.size());
      current_offset += key.size();

      // Write value
      pairs[i].value.offset = current_offset;
      pairs[i].value.length = value.size();
      memcpy(strings + current_offset, value.data(), value.size());
      current_offset += value.size();

      ++i;
    }

    sync();
  }

  std::unordered_map<std::string, std::string> load(const std::string& filename) {
#ifdef _WIN32
    file_handle_ = CreateFileA(filename.c_str(), GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING,
                               FILE_ATTRIBUTE_NORMAL, NULL);
    if (file_handle_ == INVALID_HANDLE_VALUE) {
      throw std::runtime_error("Failed to open file");
    }

    LARGE_INTEGER file_size;
    if (!GetFileSizeEx(file_handle_, &file_size)) {
      CloseHandle(file_handle_);
      throw std::runtime_error("Failed to get file size");
    }

    mapping_handle_ = CreateFileMappingA(file_handle_, NULL, PAGE_READONLY, 0, 0, NULL);
    if (!mapping_handle_) {
      CloseHandle(file_handle_);
      throw std::runtime_error("Failed to create file mapping");
    }

    mapped_data_ = MapViewOfFile(mapping_handle_, FILE_MAP_READ, 0, 0, file_size.QuadPart);
    mapped_size_ = file_size.QuadPart;
#else
    fd_ = open(filename.c_str(), O_RDONLY);
    if (fd_ == -1) {
      throw std::runtime_error("Failed to open file");
    }

    struct stat sb;
    if (fstat(fd_, &sb) == -1) {
      ::close(fd_);
      throw std::runtime_error("Failed to get file size");
    }

    mapped_data_ = mmap(nullptr, sb.st_size, PROT_READ, MAP_SHARED, fd_, 0);
    if (mapped_data_ == MAP_FAILED) {
      ::close(fd_);
      throw std::runtime_error("Failed to map file");
    }
    mapped_size_ = sb.st_size;
#endif

    auto* header = static_cast<const FileHeader*>(mapped_data_);
    if (header->magic != MAGIC) {
      close();
      throw std::runtime_error("Invalid file format");
    }

    std::unordered_map<std::string, std::string> result;
    result.reserve(header->count);

    auto* pairs = reinterpret_cast<const KeyValuePair*>(static_cast<const char*>(mapped_data_) +
                                                        sizeof(FileHeader));
    const char* strings = static_cast<const char*>(mapped_data_) + header->strings_offset;

    for (size_t i = 0; i < header->count; ++i) {
      std::string key(strings + pairs[i].key.offset, pairs[i].key.length);
      std::string value(strings + pairs[i].value.offset, pairs[i].value.length);
      result.emplace(std::move(key), std::move(value));
    }

    return result;
  }

  void sync() {
    if (!mapped_data_)
      return;

#ifdef _WIN32
    if (!FlushViewOfFile(mapped_data_, mapped_size_)) {
      throw std::runtime_error("Failed to flush view");
    }
    if (!FlushFileBuffers(file_handle_)) {
      throw std::runtime_error("Failed to flush file");
    }
#else
    if (msync(mapped_data_, mapped_size_, MS_SYNC) != 0) {
      throw std::runtime_error("Failed to sync mapped memory");
    }
#endif
  }

  void close() {
    if (mapped_data_) {
#ifdef _WIN32
      UnmapViewOfFile(mapped_data_);
      if (mapping_handle_) {
        CloseHandle(mapping_handle_);
      }
      if (file_handle_ != INVALID_HANDLE_VALUE) {
        CloseHandle(file_handle_);
      }
      mapping_handle_ = NULL;
      file_handle_ = INVALID_HANDLE_VALUE;
#else
      munmap(mapped_data_, mapped_size_);
      if (fd_ != -1) {
        ::close(fd_);
      }
      fd_ = -1;
#endif
      mapped_data_ = nullptr;
      mapped_size_ = 0;
    }
  }
};
