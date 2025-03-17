#pragma once

#include <string>

#include "qjs_helper.h"

inline std::string getFolderPath(const std::string& sourcePath) {
  size_t found = sourcePath.find_last_of("/\\");
  return sourcePath.substr(0, found);
}

inline void setJsBasePath(const std::string& sourcePath, const std::string& relativePath) {
  std::string folderPath = getFolderPath(sourcePath);
  QjsHelper::baseFolder = folderPath + relativePath;
}
