#pragma once

#include <string>

inline std::string getFolderPath(const std::string& sourcePath) {
  size_t found = sourcePath.find_last_of("/\\");
  return sourcePath.substr(0, found);
}
