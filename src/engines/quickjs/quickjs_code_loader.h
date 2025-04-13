#pragma once

#include <quickjs.h>

class QuickJSCodeLoader {
public:
  static JSValue loadJsModuleToNamespace(JSContext* ctx, const char* moduleName);
  static JSValue loadJsModuleToGlobalThis(JSContext* ctx, const char* moduleName);
  static JSValue getExportedClassHavingMethodNameInModule(JSContext* ctx,
                                                          JSValue moduleObj,
                                                          const char* methodName);
  static JSValue getExportedClassByNameInModule(JSContext* ctx,
                                                JSValue moduleObj,
                                                const char* className);
  static JSValue getMethodByNameInClass(JSContext* ctx, JSValue classObj, const char* methodName);
};
