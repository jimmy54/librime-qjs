#ifndef MARISA_TRIES_WITH_STRING_H
#define MARISA_TRIES_WITH_STRING_H

#include <marisa.h>

#include <cstddef>
#include <filesystem>
#include <iostream>
#include <optional>
#include <string>
#include <string_view>
#include <vector>
// mmap headers
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>

namespace rime {

class Trie {
private:
  marisa::Trie trie_;
  std::vector<std::string> data_;

protected:
  marisa::Trie& getTrie() { return trie_; }
  std::vector<std::string>& getData() { return data_; }

  // Enhanced I/O utilities
  struct IOUtil {
    static void writeSizeAndData(std::ostream& out, std::string_view data) {
      const size_t len = data.length();
      out.write(reinterpret_cast<const char*>(&len), sizeof(len));
      out.write(data.data(), static_cast<std::streamsize>(len));
    }

    static std::string readSizedString(std::istream& in) {
      size_t len = 0;
      in.read(reinterpret_cast<char*>(&len), sizeof(len));
      std::string result(len, '\0');
      in.read(result.data(), static_cast<std::streamsize>(len));
      return result;
    }

    static void writeVectorData(std::ostream& out, const std::vector<std::string>& data) {
      const size_t size = data.size();
      out.write(reinterpret_cast<const char*>(&size), sizeof(size));
      for (const auto& str : data) {
        writeSizeAndData(out, str);
      }
    }

    static void readVectorData(std::istream& in, std::vector<std::string>& data) {
      size_t size = 0;
      in.read(reinterpret_cast<char*>(&size), sizeof(size));
      data.resize(size);
      for (auto& str : data) {
        str = readSizedString(in);
      }
    }
  };

  class ScopedTempFile {
    std::filesystem::path path_;

  public:
    explicit ScopedTempFile(const std::filesystem::path& base) : path_(base.string() + ".temp") {}
    ~ScopedTempFile() { std::filesystem::remove(path_); }

    // Delete copy constructor and assignment operator
    ScopedTempFile(const ScopedTempFile&) = delete;
    ScopedTempFile& operator=(const ScopedTempFile&) = delete;

    // Delete move constructor and assignment operator since we manage a
    // filesystem resource
    ScopedTempFile(ScopedTempFile&&) = delete;
    ScopedTempFile& operator=(ScopedTempFile&&) = delete;

    [[nodiscard]] const std::filesystem::path& path() const { return path_; }
  };

public:
  void loadBinaryFileMmap(std::string_view filePath);
  void loadTextFile(const std::string& txtPath, size_t entrySize);
  void saveToBinaryFile(const std::string& filePath) const;
  void add(const std::string& key, const std::string& value);
  void build(const std::vector<std::pair<std::string, std::string>>& items);

  [[nodiscard]] std::optional<std::string> find(std::string_view key) const;
  [[nodiscard]] bool contains(std::string_view key) const;
  [[nodiscard]] std::vector<std::pair<std::string, std::string>> prefixSearch(
      std::string_view prefix) const;
};

}  // namespace rime
#endif  // MARISA_TRIES_WITH_STRING_H
