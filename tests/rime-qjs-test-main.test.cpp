#include <gtest/gtest.h>
#include <rime/registry.h>
#include <rime/setup.h>
#include <rime_api.h>

#include <filesystem>
#include <string>
#include "engines/engine_manager.h"

#include <quickjs.h>

#ifdef _WIN32
#include <windows.h>
#endif

using rime::kDefaultModules;

class GlobalEnvironment : public testing::Environment {
private:
  std::string userDataDir_;

public:
  void SetUp() override {
    std::filesystem::path path(__FILE__);
    path.remove_filename();
    userDataDir_ = path.generic_string();
    LOG(INFO) << "setting up user data dir: " << userDataDir_;

    RimeTraits traits = {
        .data_size = sizeof(RimeTraits) - sizeof((traits).data_size),
        .shared_data_dir = ".",
        .user_data_dir = userDataDir_.c_str(),
        .distribution_name = nullptr,
        .distribution_code_name = nullptr,
        .distribution_version = nullptr,
        .app_name = "rime.test",
        .modules = nullptr,
        .min_log_level = 0,
        .log_dir = nullptr,
        .prebuilt_data_dir = ".",
        .staging_dir = ".",
    };
    rime_get_api()->setup(&traits);
    rime_get_api()->initialize(&traits);
  }

  void TearDown() override { rime_get_api()->finalize(); }
};

int main(int argc, char** argv) noexcept {
  testing::InitGoogleTest(&argc, argv);
  try {
    testing::AddGlobalTestEnvironment(new GlobalEnvironment);
  } catch (const std::bad_alloc& e) {
    LOG(ERROR) << "Failed to allocate GlobalEnvironment: " << e.what();
    std::exit(1);
  }

#ifdef _WIN32
  // Enables UTF-8 output in the Windows console
  // Only works when printing logs directly to console using `.\librime-qjs-tests.exe`
  // Does not work when redirecting output to a file using `.\librime-qjs-tests.exe > tests.log 2>&1`
  SetConsoleOutputCP(CP_UTF8);
#endif

  return RUN_ALL_TESTS();
}
