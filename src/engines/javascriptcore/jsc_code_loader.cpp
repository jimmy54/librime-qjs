#include "jsc_code_loader.h"
#include <glog/logging.h>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>

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
                                                   const char* moduleName,
                                                   JSValueRef* exception) {
  // Get the global object
  JSObjectRef globalObject = JSContextGetGlobalObject(ctx);

  // todo: find the js file in these orders: ./dist/module.js > ./module.js
  std::filesystem::path filePath = std::filesystem::path(baseFolderPath + "/dist/" + moduleName);
  std::ifstream file(filePath);
  if (!file.is_open()) {
    std::string message = "Failed to open file: " + filePath.string();
    LOG(ERROR) << "[jsc] " << message;

    return JSValueMakeString(ctx, JscStringRAII(message.c_str()));
  }

  std::stringstream buffer;
  buffer << file.rdbuf();
  auto source = buffer.str();
  removeExportStatementsInPlace(source);

  JSEvaluateScript(ctx, JscStringRAII(source.c_str()), nullptr, nullptr, 0, exception);

  return globalObject;
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

void JscCodeLoader::exposeLogToJsConsole(JSContextRef ctx) {
  JSObjectRef globalObject = JSContextGetGlobalObject(ctx);

  // Create console object
  JSObjectRef consoleObj = JSObjectMake(ctx, nullptr, nullptr);

  // Create log function
  JscStringRAII logStr = "log";
  JSObjectRef logFunc = JSObjectMakeFunctionWithCallback(ctx, logStr, JscCodeLoader::jsLog);

  // Create error function
  JscStringRAII errorStr = "error";
  JSObjectRef errorFunc = JSObjectMakeFunctionWithCallback(ctx, errorStr, JscCodeLoader::jsError);

  // Add functions to console object
  JSObjectSetProperty(ctx, consoleObj, logStr, logFunc, kJSPropertyAttributeNone, nullptr);
  JSObjectSetProperty(ctx, consoleObj, errorStr, errorFunc, kJSPropertyAttributeNone, nullptr);

  // Add console object to global scope
  JSObjectSetProperty(ctx, globalObject, JscStringRAII("console"), consoleObj,
                      kJSPropertyAttributeNone, nullptr);
}

static std::string processJsArguments(JSContextRef ctx,
                                      size_t argumentCount,
                                      const JSValueRef arguments[],
                                      JSValueRef* exception) {
  std::stringstream ss;
  for (size_t i = 0; i < argumentCount; i++) {
    JscStringRAII strRef = JSValueToStringCopy(ctx, arguments[i], exception);
    size_t bufferSize = JSStringGetMaximumUTF8CStringSize(strRef);
    char* buffer = new char[bufferSize];
    JSStringGetUTF8CString(strRef, buffer, bufferSize);
    ss << buffer;
    if (i < argumentCount - 1) {
      ss << " ";
    }
    delete[] buffer;
  }
  return ss.str();
}

JSValueRef JscCodeLoader::jsLog(JSContextRef ctx,
                                JSObjectRef function,
                                JSObjectRef thisObject,
                                size_t argumentCount,
                                const JSValueRef arguments[],
                                JSValueRef* exception) {
  std::string message = processJsArguments(ctx, argumentCount, arguments, exception);
  LOG(INFO) << "$jsc$ " << message;
  return JSValueMakeUndefined(ctx);
}

JSValueRef JscCodeLoader::jsError(JSContextRef ctx,
                                  JSObjectRef function,
                                  JSObjectRef thisObject,
                                  size_t argumentCount,
                                  const JSValueRef arguments[],
                                  JSValueRef* exception) {
  std::string message = processJsArguments(ctx, argumentCount, arguments, exception);
  LOG(ERROR) << "$jsc$ " << message;
  return JSValueMakeUndefined(ctx);
}
