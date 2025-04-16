#include "jsc_code_loader.h"
#include <glog/logging.h>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

#include "engines/javascriptcore/jsc_string_raii.hpp"

static void removeExportStatementsInPlace(std::string& source) {
  size_t pos = 0;
  while ((pos = source.find("export", pos)) != std::string::npos) {
    size_t rightBracket = source.find('}', pos);
    if (rightBracket == std::string::npos) {
      rightBracket = source.length();
    } else {
      rightBracket++;
    }
    source.replace(pos, rightBracket - pos, "");
    pos = rightBracket;
  }
}

JSValueRef JscCodeLoader::loadJsModuleToGlobalThis(JSContextRef ctx,
                                                   const std::string& baseFolderPath,
                                                   const std::string& moduleName,
                                                   JSValueRef* exception) {
  // file lookup order: ./dist/module.dist.js > ./dist/module.js > ./module.dist.js > ./module.js
  std::vector<std::string> fileNames = {"dist/" + moduleName + ".dist.js",
                                        "dist/" + moduleName + ".js", moduleName + ".dist.js",
                                        moduleName + ".js"};

  for (const auto& fileName : fileNames) {
    std::filesystem::path filePath = std::filesystem::path(baseFolderPath) / fileName;
    if (!std::filesystem::exists(filePath)) {
      continue;
    }

    std::ifstream file(filePath);
    std::stringstream buffer;
    buffer << file.rdbuf();
    auto source = buffer.str();
    removeExportStatementsInPlace(source);

    auto sourceUrl = JscStringRAII(filePath.generic_string().c_str());
    JSEvaluateScript(ctx, JscStringRAII(source.c_str()), nullptr, sourceUrl, 0, exception);

    return JSContextGetGlobalObject(ctx);
  }

  std::string message = "Failed to open file for module: " + moduleName;
  LOG(ERROR) << "[jsc] " << message;
  *exception = JSValueMakeString(ctx, JscStringRAII(message.c_str()));
  return nullptr;
}

JSValueRef JscCodeLoader::getExportedClassHavingMethodNameInModule(JSContextRef ctx,
                                                                   JSValueRef moduleObj,
                                                                   const char* methodName) {
  if (!JSValueIsObject(ctx, moduleObj)) {
    return JSValueMakeNull(ctx);
  }

  JSObjectRef moduleObjRef = JSValueToObject(ctx, moduleObj, nullptr);

  // Get all properties of the module object
  JSPropertyNameArrayRef propertyNames = JSObjectCopyPropertyNames(ctx, moduleObjRef);
  size_t count = JSPropertyNameArrayGetCount(propertyNames);

  for (size_t i = 0; i < count; i++) {
    JSStringRef propertyName = JSPropertyNameArrayGetNameAtIndex(propertyNames, i);
    JSValueRef property = JSObjectGetProperty(ctx, moduleObjRef, propertyName, nullptr);

    if (JSValueIsObject(ctx, property)) {
      JSObjectRef propertyObj = JSValueToObject(ctx, property, nullptr);
      JSValueRef method =
          JSObjectGetProperty(ctx, propertyObj, JSStringCreateWithUTF8CString(methodName), nullptr);

      if (JSValueIsObject(ctx, method) &&
          JSObjectIsFunction(ctx, JSValueToObject(ctx, method, nullptr))) {
        JSPropertyNameArrayRelease(propertyNames);
        return property;
      }
    }
  }

  JSPropertyNameArrayRelease(propertyNames);
  return JSValueMakeNull(ctx);
}

JSValueRef JscCodeLoader::getExportedClassByNameInModule(JSContextRef ctx,
                                                         JSValueRef moduleObj,
                                                         const char* className) {
  if (!JSValueIsObject(ctx, moduleObj)) {
    return JSValueMakeNull(ctx);
  }

  JSObjectRef moduleObjRef = JSValueToObject(ctx, moduleObj, nullptr);
  JSValueRef classObj = JSObjectGetProperty(ctx, moduleObjRef, JscStringRAII(className), nullptr);
  return JSValueIsObject(ctx, classObj) ? classObj : JSValueMakeNull(ctx);
}

JSValueRef JscCodeLoader::getMethodByNameInClass(JSContextRef ctx,
                                                 JSValueRef classObj,
                                                 const char* methodName) {
  if (!JSValueIsObject(ctx, classObj)) {
    return JSValueMakeNull(ctx);
  }

  JSObjectRef classObjRef = JSValueToObject(ctx, classObj, nullptr);
  JSValueRef method = JSObjectGetProperty(ctx, classObjRef, JscStringRAII(methodName), nullptr);

  if (JSValueIsObject(ctx, method) &&
      JSObjectIsFunction(ctx, JSValueToObject(ctx, method, nullptr))) {
    return method;
  }

  return JSValueMakeNull(ctx);
}
