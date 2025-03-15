#pragma once

#include <string>

#include "qjs_helper.h"

inline std::string getFolderPath(std::string sourcePath) {
  size_t found = sourcePath.find_last_of("/\\");
  return sourcePath.substr(0, found);
}

inline void setJsBasePath(std::string sourcePath, std::string relativePath) {
  std::string folderPath = getFolderPath(sourcePath);
  QjsHelper::basePath = folderPath + relativePath;
}
