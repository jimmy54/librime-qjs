#include <leveldb/db.h>
#include <leveldb/write_batch.h>
#include <optional>
#include <string>
#include <vector>

#include "dicts/dictionary.h"

using namespace std;

class LevelDb : public Dictionary {
  leveldb::DB* ptr_ = nullptr;
  leveldb::WriteBatch batch_;

  std::string txtPath_;
  ParseTextFileOptions textFileOptions_;

public:
  LevelDb() = default;
  LevelDb(const LevelDb&) = delete;
  LevelDb(LevelDb&&) = delete;
  LevelDb& operator=(const LevelDb&) = delete;
  LevelDb& operator=(LevelDb&&) = delete;

  ~LevelDb() override {
    if (ptr_ != nullptr) {
      delete ptr_;
      ptr_ = nullptr;
    }
  }

  void close() {
    if (ptr_ != nullptr) {
      delete ptr_;
      ptr_ = nullptr;
    }
  }

  void loadTextFile(const std::string& txtPath, const ParseTextFileOptions& options) override {
    txtPath_ = txtPath;
    textFileOptions_ = options;

    this->textFileOptions_ = options;
  }

  void loadBinaryFile(const std::string& filePath) override {
    leveldb::Options options;
    options.create_if_missing = false;
    leveldb::DB::Open(options, filePath, &ptr_);
  }

  void saveToBinaryFile(const std::string& filePath) override {
    if (txtPath_.empty()) {
      throw std::runtime_error("No text file loaded.");
    }
    if (ptr_ != nullptr) {
      throw std::runtime_error("LevelDb already loaded.");
    }

    parseTextFile(
        txtPath_, textFileOptions_,
        [this](const std::string& key, const std::string& value) { batch_.Put(key, value); });

    leveldb::Options options;
    options.create_if_missing = true;
    options.error_if_exists = false;
    leveldb::DB::Open(options, filePath, &ptr_);
    ptr_->Write(leveldb::WriteOptions(), &batch_);
    batch_.Clear();
  }

  [[nodiscard]] std::optional<std::string> find(const std::string& key) const override {
    if (ptr_ == nullptr) {
      throw std::runtime_error("LevelDb not loaded.");
    }
    std::string value;
    auto status = ptr_->Get(leveldb::ReadOptions(), key, &value);
    return status.ok() ? std::make_optional(value) : std::nullopt;
  }

  [[nodiscard]] std::vector<std::pair<std::string, std::string>> prefixSearch(
      const std::string& prefix) const override {
    if (ptr_ == nullptr) {
      throw std::runtime_error("LevelDb not loaded.");
    }
    std::vector<std::pair<std::string, std::string>> results;
    leveldb::Iterator* it = ptr_->NewIterator(leveldb::ReadOptions());
    for (it->Seek(prefix); it->Valid(); it->Next()) {
      std::string key = it->key().ToString();
      if (key.find(prefix) != 0) {
        break;
      }
      std::string value = it->value().ToString();
      results.emplace_back(key, value);
    }
    delete it;
    return results;
  }
};
