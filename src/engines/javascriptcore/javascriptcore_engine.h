#pragma once

#include <JavaScriptCore/JavaScript.h>
#include <JavaScriptCore/JavaScriptCore.h>
#include <glog/logging.h>
#include <memory>
#include <string>
#include <tuple>
#include <unordered_map>

#include "engines/javascriptcore/jsc_code_loader.h"
#include "engines/js_engine.h"
#include "types/js_wrapper.h"

template <>
class JsEngine<JSValueRef> {
public:
  JsEngine(const JsEngine&) = delete;
  JsEngine(JsEngine&&) = delete;
  JsEngine& operator=(const JsEngine&) = delete;
  JsEngine& operator=(JsEngine&&) = delete;

  static JsEngine<JSValueRef>& getInstance() {
    static JsEngine<JSValueRef> instance;
    return instance;
  }

  int64_t getMemoryUsage() {
    // TODO: find an API to get the memory usage of JavaScriptCore
    return -1;
  }

  typename TypeMap<JSValueRef>::ContextType& getContext() { return ctx_; }

  JSObjectRef jsValueToObject(JSValueRef value) {
    if ((value == nullptr) || JSValueIsNull(ctx_, value) || JSValueIsUndefined(ctx_, value)) {
      return nullptr;
    }
    JSValueRef exception = nullptr;
    JSObjectRef obj = JSValueToObject(ctx_, value, &exception);
    if (exception != nullptr) {
      logErrorStackTrace(exception);
      return nullptr;
    }
    return obj;
  }

  template <typename T>
  void* getOpaque(JSObjectRef value) {
    return JSObjectGetPrivate(value);
  }

  void setOpaque(JSObjectRef value, void* opaque) { JSObjectSetPrivate(value, opaque); }

  JSValueRef null() { return JSValueMakeNull(ctx_); }
  JSValueRef undefined() { return JSValueMakeUndefined(ctx_); }

  JSValueRef jsTrue() { return JSValueMakeBoolean(ctx_, true); }
  JSValueRef jsFalse() { return JSValueMakeBoolean(ctx_, false); }

  JSValueRef newArray() { return JSObjectMakeArray(ctx_, 0, nullptr, nullptr); }

  size_t getArrayLength(const JSValueRef& array) {
    JSObjectRef arrayObj = JSValueToObject(ctx_, array, nullptr);
    JSValueRef lengthValue =
        JSObjectGetProperty(ctx_, arrayObj, JSStringCreateWithUTF8CString("length"), nullptr);
    return static_cast<size_t>(JSValueToNumber(ctx_, lengthValue, nullptr));
  }

  int insertItemToArray(JSValueRef array, size_t index, const JSValueRef& value) {
    JSObjectRef arrayObj = JSValueToObject(ctx_, array, nullptr);
    JSObjectSetPropertyAtIndex(ctx_, arrayObj, index, value, nullptr);
    return 0;
  }

  JSValueRef getArrayItem(const JSValueRef& array, size_t index) {
    JSObjectRef arrayObj = JSValueToObject(ctx_, array, nullptr);
    return JSObjectGetPropertyAtIndex(ctx_, arrayObj, index, nullptr);
  }

  JSObjectRef newObject() { return JSObjectMake(ctx_, nullptr, nullptr); }

  JSValueRef getObjectProperty(const JSObjectRef& obj, const char* propertyName) {
    JSStringRef propertyStr = JSStringCreateWithUTF8CString(propertyName);
    JSValueRef value = JSObjectGetProperty(ctx_, obj, propertyStr, nullptr);
    JSStringRelease(propertyStr);
    return value;
  }

  int setObjectProperty(JSObjectRef obj, const char* propertyName, const JSValueRef& value) {
    JSStringRef propertyStr = JSStringCreateWithUTF8CString(propertyName);
    JSObjectSetProperty(ctx_, obj, propertyStr, value, kJSPropertyAttributeNone, nullptr);
    JSStringRelease(propertyStr);
    return 0;
  }

  int setObjectFunction(JSObjectRef obj,
                        const char* functionName,
                        JSObjectCallAsFunctionCallback cppFunction,
                        int expectingArgc) {
    JSStringRef funcNameStr = JSStringCreateWithUTF8CString(functionName);
    JSObjectRef func = JSObjectMakeFunctionWithCallback(ctx_, funcNameStr, cppFunction);
    JSObjectSetProperty(ctx_, obj, funcNameStr, func, kJSPropertyAttributeNone, nullptr);
    JSStringRelease(funcNameStr);
    return 0;
  }

  JSObjectRef toObject(const JSValueRef& value) { return JSValueToObject(ctx_, value, nullptr); }

  JSValueRef toJsString(const char* str) {
    JSStringRef jsStr = JSStringCreateWithUTF8CString(str);
    JSValueRef value = JSValueMakeString(ctx_, jsStr);
    JSStringRelease(jsStr);
    return value;
  }

  JSValueRef toJsString(const std::string& str) { return toJsString(str.c_str()); }

  std::string toStdString(const JSValueRef& value) {
    JSStringRef jsStr = JSValueToStringCopy(ctx_, value, nullptr);
    size_t bufferSize = JSStringGetMaximumUTF8CStringSize(jsStr);
    std::vector<char> buffer(bufferSize);
    JSStringGetUTF8CString(jsStr, buffer.data(), bufferSize);
    JSStringRelease(jsStr);
    return buffer.data();
  }

  JSValueRef toJsBool(bool value) { return JSValueMakeBoolean(ctx_, value); }

  bool toBool(const JSValueRef& value) { return JSValueToBoolean(ctx_, value); }

  JSValueRef toJsInt(size_t value) { return JSValueMakeNumber(ctx_, static_cast<double>(value)); }

  size_t toInt(const JSValueRef& value) {
    return static_cast<size_t>(JSValueToNumber(ctx_, value, nullptr));
  }

  JSValueRef toJsDouble(double value) { return JSValueMakeNumber(ctx_, value); }

  double toDouble(const JSValueRef& value) { return JSValueToNumber(ctx_, value, nullptr); }

  JSValueRef callFunction(JSObjectRef func, JSObjectRef thisArg, int argc, JSValueRef* argv) {
    JSValueRef exception = nullptr;
    JSValueRef result = JSObjectCallAsFunction(ctx_, func, thisArg, argc, argv, &exception);
    if (exception != nullptr) {
      logErrorStackTrace(exception);
      return nullptr;
    }
    return result;
  }

  JSObjectRef callConstructor(JSObjectRef func, int argc, JSValueRef* argv) {
    JSValueRef exception = nullptr;
    JSObjectRef result = JSObjectCallAsConstructor(ctx_, func, argc, argv, &exception);
    if (exception != nullptr) {
      logErrorStackTrace(exception);
      return nullptr;
    }
    return result;
  }

  JSValueRef getJsClassHavingMethod(const JSValueRef& module, const char* methodName) {
    JSObjectRef globalObj = JSContextGetGlobalObject(ctx_);
    JSPropertyNameArrayRef propertyNames = JSObjectCopyPropertyNames(ctx_, globalObj);
    size_t count = JSPropertyNameArrayGetCount(propertyNames);

    for (size_t i = 0; i < count; i++) {
      JSStringRef propertyName = JSPropertyNameArrayGetNameAtIndex(propertyNames, i);
      JSValueRef value = JSObjectGetProperty(ctx_, globalObj, propertyName, nullptr);

      if (JSValueIsObject(ctx_, value)) {
        JSObjectRef obj = JSValueToObject(ctx_, value, nullptr);
        JSValueRef prototype =
            JSObjectGetProperty(ctx_, obj, JSStringCreateWithUTF8CString("prototype"), nullptr);

        if (!JSValueIsUndefined(ctx_, prototype) && JSValueIsObject(ctx_, prototype)) {
          JSObjectRef prototypeObj = JSValueToObject(ctx_, prototype, nullptr);
          JSValueRef method = JSObjectGetProperty(
              ctx_, prototypeObj, JSStringCreateWithUTF8CString(methodName), nullptr);

          if (!JSValueIsUndefined(ctx_, method) && JSValueIsObject(ctx_, method)) {
            JSPropertyNameArrayRelease(propertyNames);
            return value;
          }
        }
      }
    }

    JSPropertyNameArrayRelease(propertyNames);
    return nullptr;
  }

  JSValueRef getMethodOfClass(JSObjectRef jsClass, const char* methodName) {
    JSStringRef methodStr = JSStringCreateWithUTF8CString(methodName);
    JSValueRef method = JSObjectGetProperty(ctx_, jsClass, methodStr, nullptr);
    JSStringRelease(methodStr);
    return method;
  }

  JSValueRef throwError(JsErrorType errorType, const char* format, ...) {
    va_list args;
    va_start(args, format);
    char buffer[1024];
    vsnprintf(buffer, sizeof(buffer), format, args);
    va_end(args);

    JSStringRef messageStr = JSStringCreateWithUTF8CString(buffer);
    JSValueRef exception = JSValueMakeString(ctx_, messageStr);
    JSStringRelease(messageStr);
    return exception;
  }

  bool isObject(const JSValueRef& value) { return JSValueIsObject(ctx_, value); }

  bool isNull(const JSValueRef& value) { return JSValueIsNull(ctx_, value); }

  bool isUndefined(const JSValueRef& value) { return JSValueIsUndefined(ctx_, value); }

  bool isException(const JSValueRef& value) { return value != nullptr; }

  void logErrorStackTrace(const JSValueRef& exception) {
    JSStringRef exceptionStr = JSValueToStringCopy(ctx_, exception, nullptr);
    size_t bufferSize = JSStringGetMaximumUTF8CStringSize(exceptionStr);
    std::vector<char> buffer(bufferSize);
    JSStringGetUTF8CString(exceptionStr, buffer.data(), bufferSize);
    LOG(ERROR) << "[jsc] JavaScript exception: " << buffer.data() << '\n';
    JSStringRelease(exceptionStr);
  }

  void freeValue(const JSValueRef& value) {
    // JavaScriptCore handles memory management automatically
  }

  template <typename T_RIME_TYPE>
  void registerType(JsWrapper<T_RIME_TYPE, JSValueRef>& wrapper) {
    const char* key = typeid(T_RIME_TYPE).name();
    const char* typeName = JsWrapper<T_RIME_TYPE, JSValueRef>::getTypeName();
    if (clazzes.find(key) != clazzes.end()) {
      LOG(INFO) << "type: " << typeName << " has already been registered.";
      return;
    }

    JSClassDefinition classDef = {
        0,  // version
        kJSClassAttributeNone,
        typeName,
        nullptr,  // parentClass
        nullptr,  // staticValues
        nullptr,  // staticFunctions
        nullptr,  // initialize
        wrapper.getFinalizerJsc(),
        nullptr,  // hasProperty
        nullptr,  // getProperty
        nullptr,  // setProperty
        nullptr,  // deleteProperty
        nullptr,  // getPropertyNames
        nullptr,  // callAsFunction
        nullptr,  // callAsConstructor
        nullptr,  // hasInstance
        nullptr,  // convertToType
    };

    JSClassRef jsClass = JSClassCreate(&classDef);
    clazzes[key] = std::make_tuple(typeName, jsClass, classDef);

    JSObjectRef proto = JSObjectMake(ctx_, nullptr, nullptr);
    JSObjectRef constructor = JSObjectMakeConstructor(ctx_, jsClass, wrapper.getConstructorJsc());

    // Set properties and functions
    auto properties = wrapper.getPropertiesJsc();
    for (size_t i = 0; i < wrapper.getPropertiesCount(); ++i) {
      JSStringRef nameStr = JSStringCreateWithUTF8CString(properties[i].name);
      JSValueRef value = properties[i].getProperty(ctx_, proto, nameStr, nullptr);
      JSObjectSetProperty(ctx_, proto, nameStr, value, kJSPropertyAttributeNone, nullptr);
      JSStringRelease(nameStr);
    }

    auto getters = wrapper.getGettersJsc();
    for (size_t i = 0; i < wrapper.getGettersCount(); ++i) {
      JSStringRef nameStr = JSStringCreateWithUTF8CString(getters[i].name);
      JSValueRef value = getters[i].getProperty(ctx_, proto, nameStr, nullptr);
      JSObjectSetProperty(ctx_, proto, nameStr, value, kJSPropertyAttributeNone, nullptr);
      JSStringRelease(nameStr);
    }

    auto functions = wrapper.getFunctionsJsc();
    for (size_t i = 0; i < wrapper.getFunctionsCount(); ++i) {
      JSStringRef nameStr = JSStringCreateWithUTF8CString(functions[i].name);
      JSObjectRef func =
          JSObjectMakeFunctionWithCallback(ctx_, nameStr, functions[i].callAsFunction);
      JSObjectSetProperty(ctx_, proto, nameStr, func, kJSPropertyAttributeNone, nullptr);
      JSStringRelease(nameStr);
    }

    // Expose to global object
    JSObjectRef globalObj = JSContextGetGlobalObject(ctx_);
    JSStringRef typeNameStr = JSStringCreateWithUTF8CString(typeName);
    JSObjectSetProperty(ctx_, globalObj, typeNameStr, constructor, kJSPropertyAttributeNone,
                        nullptr);
    JSStringRelease(typeNameStr);
  }

  typename TypeMap<JSValueRef>::ExposeFunctionType
  defineFunction(const char* name, int argc, JSObjectCallAsFunctionCallback func) {
    return {name, func, static_cast<JSPropertyAttributes>(argc)};
  }

  typename TypeMap<JSValueRef>::ExposePropertyType defineProperty(
      const char* name,
      typename TypeMap<JSValueRef>::GetterFunctionType getter,
      typename TypeMap<JSValueRef>::SetterFunctionType setter) {
    return {name, getter, setter, kJSPropertyAttributeNone};
  }

  template <typename T>
  T* unwrap(const JSValueRef& value) {
    const char* key = typeid(T).name();
    if (clazzes.find(key) == clazzes.end()) {
      LOG(ERROR) << "type: " << key << " has not been registered.";
      return nullptr;
    }

    if (void* ptr = JSObjectGetPrivate(toObject(value))) {
      return static_cast<T*>(ptr);
    }
    return nullptr;
  }

  template <typename T>
  std::shared_ptr<T> unwrapShared(const JSValueRef& value) {
    if (void* ptr = JSObjectGetPrivate(toObject(value))) {
      if (auto sharedPtr = static_cast<std::shared_ptr<T>*>(ptr)) {
        return *sharedPtr;
      }
    }
    return nullptr;
  }

  template <typename T>
  JSObjectRef wrap(T* ptrValue) {
    if (!ptrValue) {
      return nullptr;
    }

    const char* key = typeid(T).name();
    if (clazzes.find(key) == clazzes.end()) {
      LOG(ERROR) << "type: " << key << " has not been registered.";
      return nullptr;
    }

    JSClassRef jsClass = std::get<1>(clazzes[key]);
    return JSObjectMake(ctx_, jsClass, ptrValue);
  }

  template <typename T>
  JSObjectRef wrapShared(const std::shared_ptr<T>& value) {
    if (!value) {
      return nullptr;
    }

    const char* key = typeid(T).name();
    if (clazzes.find(key) == clazzes.end()) {
      LOG(ERROR) << "type: " << key << " has not been registered.";
      return nullptr;
    }

    JSClassRef jsClass = std::get<1>(clazzes[key]);
    auto ptr = std::make_unique<std::shared_ptr<T>>(value);
    return JSObjectMake(ctx_, jsClass, ptr.release());
  }

  void setBaseFolderPath(const char* absolutePath) { baseFolderPath_ = absolutePath; }

  JSValueRef loadJsFile(const char* fileName) {
    JSValueRef exception = nullptr;
    return JscCodeLoader::loadJsModuleToGlobalThis(ctx_, baseFolderPath_, fileName, &exception);
  }

private:
  JsEngine<JSValueRef>() : ctx_(JSGlobalContextCreate(nullptr)) {}

  ~JsEngine<JSValueRef>() { JSGlobalContextRelease((JSGlobalContextRef)ctx_); }

  JSContextRef ctx_{nullptr};

  std::string baseFolderPath_;

  inline static std::unordered_map<std::string,
                                   std::tuple<std::string, JSClassRef, JSClassDefinition>>
      clazzes{};
};
