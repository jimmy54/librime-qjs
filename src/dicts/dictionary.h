#pragma once

#include <algorithm>
#include <fstream>
#include <functional>
#include <optional>
#include <string>

struct ParseTextFileOptions {
  std::string delimiter = "\t";
  std::string comment = "#";
  size_t lines = 0;
  bool isReversed = false;
  std::string charsToRemove = "\r";
};

class Dictionary {
public:
  virtual ~Dictionary() = default;

  virtual void loadTextFile(const std::string& txtPath, const ParseTextFileOptions& options) = 0;
  virtual void loadBinaryFile(const std::string& filePath) = 0;
  virtual void saveToBinaryFile(const std::string& filePath) = 0;
  [[nodiscard]] virtual std::optional<std::string> find(const std::string& key) const = 0;
  [[nodiscard]] virtual std::vector<std::pair<std::string, std::string>> prefixSearch(
      const std::string& prefix) const = 0;

protected:
  using FnHandleEntry = std::function<void(const std::string&, const std::string&)>;
  static void parseTextFile(const std::string& path,
                            const ParseTextFileOptions& options,
                            const FnHandleEntry& fnHandleEntry) {
    std::ifstream infile(path);
    std::string line;
    while (std::getline(infile, line)) {
      if (!line.empty() && line.find(options.comment) == 0) {
        continue;
      }

      if (!options.charsToRemove.empty()) {
        line.erase(std::remove_if(line.begin(), line.end(),
                                  [&options](char c) {
                                    return options.charsToRemove.find(c) != std::string::npos;
                                  }),
                   line.end());
      }

      size_t tabPos = line.find(options.delimiter);
      if (tabPos != std::string::npos) {
        std::string key = line.substr(0, tabPos);
        std::string value = line.substr(tabPos + 1);

        if (options.isReversed) {
          std::swap(key, value);
        }
        fnHandleEntry(key, value);
      }
    }
  }
};
