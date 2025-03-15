#include <gtest/gtest.h>
#include <rime/registry.h>
#include <rime/service.h>
#include <rime/setup.h>
#include <rime_api.h>

#include "qjs_helper.h"
#include "qjs_types.h"

#include "test_helper.h"

using rime::kDefaultModules;

using namespace rime;

// copied from ../../../test/rime_test_main.cc
// to make `Engine::Create()` not crashing the tests
int main(int argc, char** argv) {
  testing::InitGoogleTest(&argc, argv);

  rime::SetupLogging("rime.test");
  rime::LoadModules(static_cast<const char**>(kDefaultModules));
  // Do not StartService, otherwise it would leak memory.
  // rime::Service::instance().StartService();

  setJsBasePath(__FILE__, "/js");

  // Register the Rime types to quickjs again, since the ones registered in
  // module.cc are not available in the tests. It seems two diffrent quickjs
  // engines/contexts are created. Probably in diffrent process?
  auto* ctx = QjsHelper::getInstance().getContext();
  initQjsTypes(ctx);

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
