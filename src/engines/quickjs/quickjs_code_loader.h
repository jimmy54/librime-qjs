#pragma once

#include <quickjs.h>
#include <string>
#include <vector>

class QuickJSCodeLoader {
public:
  static JSValue loadJsModuleToNamespace(JSContext* ctx, const char* moduleName);
  static JSValue loadJsModuleToGlobalThis(JSContext* ctx, const char* moduleName);
  static JSValue createInstanceOfEsmBundledModule(JSContext* ctx,
                                                  const std::string& moduleName,
                                                  std::vector<JSValue>& args,
                                                  const std::string& mainFuncName);
  static JSValue createInstanceOfIifeBundledModule(JSContext* ctx,
                                                   const std::string& baseFolderPath,
                                                   const std::string& moduleName,
                                                   const std::vector<JSValue>& args);
  static JSValue getExportedClassHavingMethodNameInModule(JSContext* ctx,
                                                          JSValue moduleObj,
                                                          const char* methodName);
  static JSValue getExportedClassByNameInModule(JSContext* ctx,
                                                JSValue moduleObj,
                                                const char* className);
  static JSValue getMethodByNameInClass(JSContext* ctx, JSValue classObj, const char* methodName);
  static void logJsError(JSContext* ctx,
                         const char* prefix = ": ",
                         const char* file = __FILE__,
                         int line = __LINE__);
};
