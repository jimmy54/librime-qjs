#include "jsc_code_loader.h"
#include <filesystem>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>

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
    throw std::runtime_error("Failed to open file: " + filePath.string());
  }

  std::stringstream buffer;
  buffer << file.rdbuf();
  auto source = buffer.str();
  removeExportStatementsInPlace(source);

  JSStringRef scriptJS = JSStringCreateWithUTF8CString(source.c_str());
  JSEvaluateScript(ctx, scriptJS, nullptr, nullptr, 0, exception);
  JSStringRelease(scriptJS);

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
  JSStringRef classNameStr = JSStringCreateWithUTF8CString(className);

  JSValueRef classObj = JSObjectGetProperty(ctx, moduleObjRef, classNameStr, nullptr);
  JSStringRelease(classNameStr);

  return JSValueIsObject(ctx, classObj) ? classObj : JSValueMakeNull(ctx);
}

JSValueRef JscCodeLoader::getMethodByNameInClass(JSContextRef ctx,
                                                 JSValueRef classObj,
                                                 const char* methodName) {
  if (!JSValueIsObject(ctx, classObj)) {
    return JSValueMakeNull(ctx);
  }

  JSObjectRef classObjRef = JSValueToObject(ctx, classObj, nullptr);
  JSStringRef methodNameStr = JSStringCreateWithUTF8CString(methodName);

  JSValueRef method = JSObjectGetProperty(ctx, classObjRef, methodNameStr, nullptr);
  JSStringRelease(methodNameStr);

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
  JSObjectRef logFunc = JSObjectMakeFunctionWithCallback(ctx, JSStringCreateWithUTF8CString("log"),
                                                         JscCodeLoader::jsLog);

  // Create error function
  JSObjectRef errorFunc = JSObjectMakeFunctionWithCallback(
      ctx, JSStringCreateWithUTF8CString("error"), JscCodeLoader::jsError);

  // Add functions to console object
  JSStringRef logStr = JSStringCreateWithUTF8CString("log");
  JSStringRef errorStr = JSStringCreateWithUTF8CString("error");
  JSObjectSetProperty(ctx, consoleObj, logStr, logFunc, kJSPropertyAttributeNone, nullptr);
  JSObjectSetProperty(ctx, consoleObj, errorStr, errorFunc, kJSPropertyAttributeNone, nullptr);
  JSStringRelease(logStr);
  JSStringRelease(errorStr);

  // Add console object to global scope
  JSStringRef consoleStr = JSStringCreateWithUTF8CString("console");
  JSObjectSetProperty(ctx, globalObject, consoleStr, consoleObj, kJSPropertyAttributeNone, nullptr);
  JSStringRelease(consoleStr);
}

JSValueRef JscCodeLoader::jsLog(JSContextRef ctx,
                                JSObjectRef function,
                                JSObjectRef thisObject,
                                size_t argumentCount,
                                const JSValueRef arguments[],
                                JSValueRef* exception) {
  for (size_t i = 0; i < argumentCount; i++) {
    JSStringRef strRef = JSValueToStringCopy(ctx, arguments[i], exception);
    size_t bufferSize = JSStringGetMaximumUTF8CStringSize(strRef);
    char* buffer = new char[bufferSize];
    JSStringGetUTF8CString(strRef, buffer, bufferSize);
    std::cout << buffer;
    if (i < argumentCount - 1)
      std::cout << " ";
    delete[] buffer;
    JSStringRelease(strRef);
  }
  std::cout << std::endl;

  return JSValueMakeUndefined(ctx);
}

JSValueRef JscCodeLoader::jsError(JSContextRef ctx,
                                  JSObjectRef function,
                                  JSObjectRef thisObject,
                                  size_t argumentCount,
                                  const JSValueRef arguments[],
                                  JSValueRef* exception) {
  for (size_t i = 0; i < argumentCount; i++) {
    JSStringRef strRef = JSValueToStringCopy(ctx, arguments[i], exception);
    size_t bufferSize = JSStringGetMaximumUTF8CStringSize(strRef);
    char* buffer = new char[bufferSize];
    JSStringGetUTF8CString(strRef, buffer, bufferSize);
    std::cerr << buffer;
    if (i < argumentCount - 1)
      std::cerr << " ";
    delete[] buffer;
    JSStringRelease(strRef);
  }
  std::cerr << std::endl;

  return JSValueMakeUndefined(ctx);
}
