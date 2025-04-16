#pragma once

#include <JavaScriptCore/JavaScript.h>
#include <JavaScriptCore/JavaScriptCore.h>
#include <string>

class JscCodeLoader {
public:
  static JSValueRef loadJsModuleToGlobalThis(JSContextRef ctx,
                                             const std::string& baseFolderPath,
                                             const std::string& moduleName,
                                             JSValueRef* exception);

  static JSValueRef getExportedClassHavingMethodNameInModule(JSContextRef ctx,
                                                             JSValueRef moduleObj,
                                                             const char* methodName);
  static JSValueRef getExportedClassByNameInModule(JSContextRef ctx,
                                                   JSValueRef moduleObj,
                                                   const char* className);
  static JSValueRef getMethodByNameInClass(JSContextRef ctx,
                                           JSValueRef classObj,
                                           const char* methodName);
};
