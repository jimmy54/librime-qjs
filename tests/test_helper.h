#pragma once

#include <string>

#include "node_module_loader.h"

inline std::string getFolderPath(const std::string& sourcePath) {
  size_t found = sourcePath.find_last_of("/\\");
  return sourcePath.substr(0, found);
}

inline void setJsBasePathForTest(const std::string& sourcePath, const std::string& relativePath) {
  std::string folderPath = getFolderPath(sourcePath);
  setQjsBaseFolder((folderPath + relativePath).c_str());
}
