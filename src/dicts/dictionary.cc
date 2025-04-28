#include "dictionary.h"

#include <algorithm>
#include <fstream>

std::unordered_map<std::string, std::string> Dictionary::parseTextFile(
    const std::string& path,
    const ParseTextFileOptions& options) {
  std::unordered_map<std::string, std::string> ret(options.lines);
  std::ifstream infile(path);
  std::string line;
  while (std::getline(infile, line)) {
    if (!line.empty() && line.find(options.comment) == 0) {
      continue;
    }

    if (!options.charsToRemove.empty()) {
      removeChars(line, options.charsToRemove);
    }

    size_t tabPos = line.find(options.delimiter);
    if (tabPos == std::string::npos) {
      continue;
    }

    std::string key = line.substr(0, tabPos);
    std::string value = line.substr(tabPos + 1);

    if (options.isReversed) {
      std::swap(key, value);
    }

    if (options.onDuplicatedKey != OnDuplicatedKey::Overwrite) {
      auto it = ret.find(key);
      if (it != ret.end()) {
        if (options.onDuplicatedKey == OnDuplicatedKey::Skip) {
          continue;
        }
        if (options.onDuplicatedKey == OnDuplicatedKey::Concat) {
          value = ret[key].append(options.concatSeparator).append(value);
        }
      }
    }

    ret[key] = value;
  }
  return ret;
}

std::vector<std::string> Dictionary::split(const std::string& str, const std::string& delimiters) {
  std::vector<std::string> tokens;
  size_t start = 0;
  size_t end = 0;

  while ((end = str.find_first_of(delimiters, start)) != std::string::npos) {
    if (end != start) {  // Avoid empty tokens
      tokens.push_back(str.substr(start, end - start));
    }
    start = end + 1;
  }

  if (start < str.length()) {
    tokens.push_back(str.substr(start));
  }

  return tokens;
}

void Dictionary::removeChars(std::string& str, const std::string& charsToRemove) {
  str.erase(std::remove_if(
                str.begin(), str.end(),
                [&charsToRemove](char c) { return charsToRemove.find(c) != std::string::npos; }),
            str.end());
}
