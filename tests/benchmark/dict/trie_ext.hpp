#include <marisa.h>
#include <iostream>
#include <string>
#include <string_view>
#include <vector>
#include <cstdint>
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <filesystem>
// mmap headers
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>

#include <yas/binary_iarchive.hpp>
#include <yas/binary_oarchive.hpp>
#include <yas/std_types.hpp>
#include <yas/serialize.hpp>

#include "trie.h"

class TrieWithStringExt : public rime::Trie {

public:
    void save_to_files(std::string_view data_path) const {
        const auto trie_path = std::string(data_path) + ".trie";
        trie_.save(trie_path.c_str());

        // Save associated string data
        std::ofstream data_file(data_path, std::ios::binary);
        if (!data_file) {
            throw std::runtime_error("Failed to open data file for writing");
        }

        size_t size = data_.size();
        data_file.write(reinterpret_cast<const char*>(&size), sizeof(size));

        // Write each string with its length
        for (const auto& str : data_) {
            size_t str_len = str.length();
            data_file.write(reinterpret_cast<const char*>(&str_len), sizeof(str_len));
            data_file.write(str.data(), str_len);
        }
    }

    void load_from_files(const std::string& data_path) {
        // Load trie data
        std::string trie_path = data_path + ".trie";
        trie_.load(trie_path.c_str());

        // Load associated string data
        std::ifstream data_file(data_path, std::ios::binary);
        if (!data_file) {
            throw std::runtime_error("Failed to open data file for reading");
        }

        size_t size;
        data_file.read(reinterpret_cast<char*>(&size), sizeof(size));
        data_.resize(size);

        // Read each string
        for (size_t i = 0; i < size; ++i) {
            size_t str_len;
            data_file.read(reinterpret_cast<char*>(&str_len), sizeof(str_len));

            std::string str(str_len, '\0');
            data_file.read(&str[0], str_len);
            data_[i] = std::move(str);
        }

        if (data_file.fail()) {
            throw std::runtime_error("Failed to read data from file");
        }
    }

    void load_from_single_file(std::string_view file_path) {
        std::ifstream file(file_path, std::ios::binary);
        if (!file) throw std::runtime_error("Failed to open file for reading");

        IOUtil::read_vector_data(file, data_);

        // Read and load trie
        size_t trie_size;
        file.read(reinterpret_cast<char*>(&trie_size), sizeof(trie_size));

        ScopedTempFile temp_file{ file_path };
        {
            std::ofstream trie_file(temp_file.path(), std::ios::binary);
            std::vector<char> buffer(trie_size);
            file.read(buffer.data(), trie_size);
            trie_file.write(buffer.data(), trie_size);
        }

        trie_.load(temp_file.path().c_str());

        if (file.fail()) throw std::runtime_error("Failed to read data from file");
    }

    void loadBinaryFileYas(std::string_view file_path) {
        // Read the entire file into a buffer first
        std::ifstream ifs(std::string(file_path), std::ios::binary | std::ios::ate);
        if (!ifs) throw std::runtime_error("Failed to open file");

        const auto size = ifs.tellg();
        ifs.seekg(0);

        std::vector<char> buffer(size);
        if (!ifs.read(buffer.data(), size)) {
            throw std::runtime_error("Failed to read file");
        }

        // Use memory buffer for deserialization
        yas::mem_istream ms(buffer.data(), buffer.size());
        constexpr std::uint32_t yas_flags = yas::binary | yas::no_header;
        yas::binary_iarchive<yas::mem_istream, yas_flags> ia(ms);

        ia & data_;

        // Read and load trie
        std::vector<char> trie_data;
        ia & trie_data;

        ScopedTempFile temp_file{ file_path };
        {
            std::ofstream trie_out(temp_file.path(), std::ios::binary);
            trie_out.write(trie_data.data(), trie_data.size());
        }
        trie_.load(temp_file.path().c_str());
    }

    void saveToBinaryFileYas(std::string_view file_path) const {
        // Use memory buffer for serialization
        yas::mem_ostream ms;
        constexpr std::uint32_t yas_flags = yas::binary | yas::no_header;
        yas::binary_oarchive<yas::mem_ostream, yas_flags> oa(ms);

        oa & data_;

        // Handle trie data
        ScopedTempFile temp_file{ file_path };
        trie_.save(temp_file.path().c_str());

        std::vector<char> trie_data;
        {
            std::ifstream trie_file(temp_file.path(), std::ios::binary);
            trie_data.assign(
                std::istreambuf_iterator<char>(trie_file),
                std::istreambuf_iterator<char>()
            );
        }
        oa & trie_data;

        // Write the serialized data to file
        auto buf = ms.get_shared_buffer();
        std::ofstream ofs(std::string(file_path), std::ios::binary);
        if (!ofs) throw std::runtime_error("Failed to open file for writing");
        ofs.write(buf.data.get(), buf.size);
    }

};
