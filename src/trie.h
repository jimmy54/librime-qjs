#ifndef MARISA_TRIES_WITH_STRING_H
#define MARISA_TRIES_WITH_STRING_H

#include <marisa.h>
#include <cstddef>
#include <iostream>
#include <string>
#include <string_view>
#include <vector>
#include <fstream>
#include <stdexcept>
#include <filesystem>
#include <optional>
// mmap headers
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>

namespace rime {

class Trie {
protected:
    marisa::Trie trie_;
    std::vector<std::string> data_;

    // RAII wrapper for mmap
    class MappedFile {
        void* data_ = nullptr;
        size_t size_ = 0;
        int fd_ = -1;
    public:
        MappedFile(const std::string& path) {
            fd_ = open(path.c_str(), O_RDONLY);
            if (fd_ == -1) throw std::runtime_error("Failed to open file");

            struct stat sb;
            if (fstat(fd_, &sb) == -1) {
                close(fd_);
                throw std::runtime_error("Failed to get file size");
            }

            size_ = sb.st_size;
            data_ = mmap(nullptr, size_, PROT_READ, MAP_PRIVATE, fd_, 0);
            if (data_ == MAP_FAILED) {
                close(fd_);
                throw std::runtime_error("Failed to mmap file");
            }
        }

        ~MappedFile() {
            if (data_) munmap(data_, size_);
            if (fd_ != -1) close(fd_);
        }

        const char* data() const {
            return static_cast<const char*>(data_);
        }

        size_t size() const { return size_; }
        int fd() const { return fd_; }

        MappedFile(const MappedFile&) = delete;
        MappedFile& operator=(const MappedFile&) = delete;
    };

    // Enhanced I/O utilities
    struct IOUtil {
        static void write_size_and_data(std::ostream& out, std::string_view data) {
            const size_t len = data.length();
            out.write(reinterpret_cast<const char*>(&len), sizeof(len));
            out.write(data.data(), len);
        }

        static std::string read_sized_string(std::istream& in) {
            size_t len;
            in.read(reinterpret_cast<char*>(&len), sizeof(len));
            std::string result(len, '\0');
            in.read(&result[0], len);
            return result;
        }

        static void write_vector_data(std::ostream& out, const std::vector<std::string>& data) {
            const size_t size = data.size();
            out.write(reinterpret_cast<const char*>(&size), sizeof(size));
            for (const auto& str : data) {
                write_size_and_data(out, str);
            }
        }

        static void read_vector_data(std::istream& in, std::vector<std::string>& data) {
            size_t size;
            in.read(reinterpret_cast<char*>(&size), sizeof(size));
            data.resize(size);
            for (auto& str : data) {
                str = read_sized_string(in);
            }
        }
    };

    class ScopedTempFile {
        std::filesystem::path path_;
    public:
        explicit ScopedTempFile(const std::filesystem::path& base)
            : path_(base.string() + ".temp") {
        }
        ~ScopedTempFile() { std::filesystem::remove(path_); }
        const std::filesystem::path& path() const { return path_; }
    };

public:
    void loadBinaryFileMmap(std::string_view file_path) {
        MappedFile mapped{ std::string(file_path) };
        const char* current = mapped.data();
        const char* end = current + mapped.size();

        // Read data vector size
        size_t data_size;
        std::memcpy(&data_size, current, sizeof(data_size));
        current += sizeof(data_size);

        data_.resize(data_size);

        // Read strings
        for (size_t i = 0; i < data_size && current < end; ++i) {
            size_t str_len;
            std::memcpy(&str_len, current, sizeof(str_len));
            current += sizeof(str_len);

            if (current + str_len > end) {
                throw std::runtime_error("Corrupted data file");
            }

            data_[i].assign(current, str_len);
            current += str_len;
        }

        // Read and load trie
        size_t trie_size;
        std::memcpy(&trie_size, current, sizeof(trie_size));
        current += sizeof(trie_size);

        off_t offset = current - mapped.data();
        lseek(mapped.fd(), offset, SEEK_SET);
        trie_.read(mapped.fd());
    }

    void loadTextFile(const std::string& txtPath, size_t entrySize) {
        std::vector<std::pair<std::string, std::string>> items(entrySize);
        std::ifstream infile(txtPath);
        std::string line;
        size_t idx = 0;
        while (std::getline(infile, line)) {
            if (!line.empty() && line[0] == '#') continue;

            size_t tab_pos = line.find('\t');
            if (tab_pos != std::string::npos) {
                std::string_view lineView = std::string_view(line);
                std::string_view key = lineView.substr(0, tab_pos);
                std::string_view value = lineView.substr(tab_pos + 1);

                if (!value.empty() && value.back() == '\r') value.remove_suffix(1);

                if (idx < entrySize) {
                    items[idx++] = std::make_pair(std::string(key), std::string(value));
                } else {
                    items.emplace_back(std::string(key), std::string(value));
                }
            }
        }
        build(items);
    }

    void saveToBinaryFile(std::string_view file_path) const {
        std::ofstream file(file_path, std::ios::binary);
        if (!file) throw std::runtime_error("Failed to open file for writing");

        IOUtil::write_vector_data(file, data_);

        // Handle trie data
        ScopedTempFile temp_file{ file_path };
        trie_.save(temp_file.path().c_str());

        std::ifstream trie_file(temp_file.path(), std::ios::binary | std::ios::ate);
        const size_t trie_size = trie_file.tellg();
        trie_file.seekg(0);

        file.write(reinterpret_cast<const char*>(&trie_size), sizeof(trie_size));
        file << trie_file.rdbuf();
    }

    void add(const std::string& key, const std::string& value) {
        marisa::Keyset keyset;
        keyset.push_back(key.c_str(), key.length());

        // Build the trie
        trie_.build(keyset, MARISA_BINARY_TAIL); // UTF-8 support

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

    void build(const std::vector<std::pair<std::string, std::string>>& items) {
        marisa::Keyset keyset;

        // First, add all keys to the keyset
        for (const auto& [key, _] : items) {
            keyset.push_back(key.c_str(), key.length());
        }

        // Build the trie
        trie_.build(keyset, MARISA_BINARY_TAIL); // UTF-8 support

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

    [[nodiscard]] std::optional<std::string> find(std::string_view key) const {
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

    [[nodiscard]] bool contains(std::string_view key) const {
        marisa::Agent agent;
        agent.set_query(key.data(), key.length());
        return trie_.lookup(agent);
    }

    [[nodiscard]] std::vector<std::pair<std::string, std::string>>
        prefix_search(std::string_view prefix) const {
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
};

} // namespace Rime
#endif // MARISA_TRIES_WITH_STRING_H
