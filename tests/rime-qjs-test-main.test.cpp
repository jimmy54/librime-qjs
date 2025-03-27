#include <gtest/gtest.h>
#include <rime/registry.h>
#include <rime/service.h>
#include <rime/setup.h>
#include <rime_api.h>

#include <filesystem>
#include <string>

#include "node_module_loader.h"

#ifdef _WIN32
#include <windows.h>
// C:\Program Files (x86)\Windows Kits\10\Include\10.0.22621.0\um\winsvc.h(1653,23): note: expanded from macro 'StartService'
//  1653 | #define StartService  StartServiceA
#undef StartService
#else
// this include breaks building on Windows:
// src\rime_api_impl.h:25:22: error: dllimport cannot be applied to non-inline function definition
// 25 | RIME_DEPRECATED void RimeSetup(RimeTraits* traits)
#include <rime_api_impl.h>
#endif

#include "qjs_helper.h"
#include "qjs_types.h"

using rime::kDefaultModules;

class GlobalEnvironment : public testing::Environment {
private:
  std::string userDataDir_;

public:
  void SetUp() override {
    rime::SetupLogging("rime.test");

    std::filesystem::path path(__FILE__);
    path.remove_filename();
    userDataDir_ = path.generic_string();

    RimeTraits traits = {
        .data_size = sizeof(RimeTraits) - sizeof((traits).data_size),
        .shared_data_dir = ".",
        .user_data_dir = userDataDir_.c_str(),
        .distribution_name = nullptr,
        .distribution_code_name = nullptr,
        .distribution_version = nullptr,
        .app_name = nullptr,
        .modules = nullptr,
        .min_log_level = 0,
        .log_dir = nullptr,
        .prebuilt_data_dir = ".",
        .staging_dir = ".",
    };
    rime::SetupDeployer(&traits);
    rime::LoadModules(static_cast<const char**>(kDefaultModules));
    rime::Service::instance().StartService();
  }

#ifndef _WIN32
  // not working on Windows since the related header <rime_api_impl.h> is not included
  // just drop it off on Windows, and run only the memory leak detection on macOS CI.
  void TearDown() override {
    setQjsBaseFolder(nullptr);
    RimeFinalize();
  }
#endif
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

  std::filesystem::path path(__FILE__);
  path.remove_filename().append("js");
  setQjsBaseFolder(path.generic_string().c_str());

  // Register the Rime types to quickjs again, since the ones registered in
  // module.cc are not available in the tests. It seems two diffrent quickjs
  // engines/contexts are created. Probably in diffrent process?
  rime::initQjsTypes(QjsHelper::getInstance().getContext());

  // "librime-qjs-tests" start time: Feb 17 10:18 CST
  // Output:
  // ----------------------------------------------------------
  // WARNING: Logging before InitGoogleLogging() is written to STDERR
  // I20250217 10:18:17.206839 0x10d94d600 module.cc:13] [qjs] registering
  // components from module 'qjs'. I20250217 10:18:17.211220 0x10d94d600
  // qjs_helper.h:31] new QjsHelper instance ctx = 0x616000000c80 Stacktrace: 0#
  // QjsHelper::QjsHelper() in librime/build/lib/rime-plugins/librime-qjs.dylib
  // 1# QjsHelper::getInstance() in
  // librime/build/lib/rime-plugins/librime-qjs.dylib 2# rime_qjs_initialize()
  // in librime/build/lib/rime-plugins/librime-qjs.dylib 3#
  // rime::ModuleManager::LoadModule(rime_module_t*) in
  // librime/build/lib/librime.1.12.0.dylib 4#
  // rime::PluginManager::LoadPlugins(rime::path) in
  // librime/build/lib/librime.1.12.0.dylib 5# rime_plugins_initialize() in
  // librime/build/lib/librime.1.12.0.dylib 6#
  // rime::ModuleManager::LoadModule(rime_module_t*) in
  // librime/build/lib/librime.1.12.0.dylib 7# rime::LoadModules(char const**)
  // in librime/build/lib/librime.1.12.0.dylib 8# main in
  // librime/plugins/qjs/build/librime-qjs-tests I20250217 10:18:17.212838
  // 0x10d94d600 qjs_types.cc:38] registering rime types to the quickjs
  // engine... WARNING: Logging before InitGoogleLogging() is written to STDERR
  // I20250217 10:18:17.227454 0x10d94d600 modules.cc:99] registering components
  // from module 'lua'. I20250217 10:18:17.241547 0x10d94d600 modules.cc:90]
  // rime.lua info: rime.lua should be either in the rime user data directory or
  // in the rime shared data directory WARNING: Logging before
  // InitGoogleLogging() is written to STDERR I20250217 10:18:17.243660
  // 0x10d94d600 qjs_helper.h:31] new QjsHelper instance ctx = 0x616000001580
  // Stacktrace:
  // 0# QjsHelper::QjsHelper() in librime/plugins/qjs/build/librime-qjs-tests
  // 1# QjsHelper::getInstance() in librime/plugins/qjs/build/librime-qjs-tests
  // 2# main in librime/plugins/qjs/build/librime-qjs-tests
  // I20250217 10:18:17.243822 0x10d94d600 rime-qjs-test-main.test.cpp:27]
  // QuickJSFilterTest ctx = 0x616000001580 I20250217 10:18:17.243833
  // 0x10d94d600 qjs_types.cc:38] registering rime types to the quickjs
  // engine...

  return RUN_ALL_TESTS();
}
