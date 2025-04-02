#pragma once

#include <rime/engine.h>
#include <rime/schema.h>
#include <rime_api.h>

#include <cstddef>
#include <cstdio>
#include <memory>
#include <string>
#include <utility>
#include "qjs_os_info.h"

class Environment {
public:
  explicit Environment(rime::Engine* engine, std::string nameSpace)
      : engine_(engine), nameSpace_(std::move(nameSpace)) {}

  [[nodiscard]] rime::Engine* getEngine() const { return engine_; }
  [[nodiscard]] const std::string& getNameSpace() const { return nameSpace_; }
  [[nodiscard]] SystemInfo* getSystemInfo() { return &systemInfo_; }
  static std::string getUserDataDir() { return rime_get_api()->get_user_data_dir(); }
  static std::string getSharedDataDir() { return rime_get_api()->get_shared_data_dir(); }

  static std::string loadFile(const std::string& path);
  static bool fileExists(const std::string& path);
  static std::string getRimeInfo();
  static std::string popen(const std::string& command);

  static std::string formatMemoryUsage(size_t usage);

private:
  rime::Engine* engine_;
  std::string nameSpace_;
  SystemInfo systemInfo_{};
};
