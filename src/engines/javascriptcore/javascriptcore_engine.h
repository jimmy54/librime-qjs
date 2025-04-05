#pragma once

#include <JavaScriptCore/JavaScript.h>
#include <JavaScriptCore/JavaScriptCore.h>
#include <glog/logging.h>
#include <memory>
#include <string>
#include <tuple>
#include <unordered_map>
#include <utility>

#include "engines/javascriptcore/jsc_code_loader.h"
#include "engines/javascriptcore/jsc_string_raii.hpp"
#include "engines/js_engine.h"
#include "types/js_wrapper.h"

template <>
class JsEngine<JSValueRef> {
  JSGlobalContextRef ctx_{nullptr};
  std::string engineId_;

  std::string baseFolderPath_;

  // the types should be registered to each jsc context
  inline static std::unordered_map<
      std::string,
      std::tuple<std::string, JSClassRef, JSClassDefinition, std::vector<JSStaticValue>>>
      clazzes{};

  inline static std::unordered_map<JSContextRef, JsEngine<JSValueRef>*> engines{};

public:
  JsEngine<JSValueRef>()
      : ctx_(JSGlobalContextCreate(nullptr)),
        engineId_(std::to_string(reinterpret_cast<size_t>(this))) {
    // Never `LOG(INFO) << engineId_;` here, as it crashes the program.
    // libc++abi: terminating with uncaught exception of type std::__1::system_error: mutex lock failed: Invalid argument

    engines[ctx_] = this;
    JscCodeLoader::exposeLogToJsConsole(ctx_);
  }

  ~JsEngine<JSValueRef>() {
    std::vector<std::string> keysToRemove;
    for (auto& clazz : clazzes) {
      if (clazz.first.find(engineId_) != std::string::npos) {
        auto& clazzDef = std::get<1>(clazz.second);
        JSClassRelease(clazzDef);
        keysToRemove.push_back(clazz.first);
      }
    }
    for (auto& key : keysToRemove) {
      clazzes.erase(key);
    }

    JSGlobalContextRelease(ctx_);
  }

  JsEngine(const JsEngine&) = delete;
  JsEngine(JsEngine&&) = delete;
  JsEngine& operator=(const JsEngine& other) {
    if (this != &other) {
      ctx_ = other.ctx_;
      engines[ctx_] = this;
    }
    return *this;
  };
  JsEngine& operator=(JsEngine&&) = delete;

  static JsEngine<JSValueRef>& getEngineByContext(JSContextRef ctx) {
    auto it = engines.find(ctx);
    if (it == engines.end()) {
      throw std::runtime_error("JsEngine<JSValue> not found");
    }
    return *it->second;
  }

  int64_t getMemoryUsage() {
    // TODO: find an API to get the memory usage of JavaScriptCore
    return -1;
  }

  template <typename T>
  static void* getOpaque(JSObjectRef value) {
    return JSObjectGetPrivate(value);
  }

  static void setOpaque(JSObjectRef value, void* opaque) { JSObjectSetPrivate(value, opaque); }

  [[nodiscard]] JSValueRef null() const { return JSValueMakeNull(ctx_); }
  [[nodiscard]] JSValueRef undefined() const { return JSValueMakeUndefined(ctx_); }

  [[nodiscard]] JSValueRef jsTrue() const { return JSValueMakeBoolean(ctx_, true); }
  [[nodiscard]] JSValueRef jsFalse() const { return JSValueMakeBoolean(ctx_, false); }

  [[nodiscard]] JSValueRef newArray() const { return JSObjectMakeArray(ctx_, 0, nullptr, nullptr); }

  [[nodiscard]] size_t getArrayLength(const JSValueRef& array) const {
    JSObjectRef arrayObj = JSValueToObject(ctx_, array, nullptr);
    JSValueRef lengthValue = JSObjectGetProperty(ctx_, arrayObj, JscStringRAII("length"), nullptr);
    return static_cast<size_t>(JSValueToNumber(ctx_, lengthValue, nullptr));
  }

  int insertItemToArray(JSValueRef array, size_t index, const JSValueRef& value) const {
    JSObjectRef arrayObj = JSValueToObject(ctx_, array, nullptr);
    JSObjectSetPropertyAtIndex(ctx_, arrayObj, index, value, nullptr);
    return 0;
  }

  [[nodiscard]] JSValueRef getArrayItem(const JSValueRef& array, size_t index) const {
    JSObjectRef arrayObj = JSValueToObject(ctx_, array, nullptr);
    return JSObjectGetPropertyAtIndex(ctx_, arrayObj, index, nullptr);
  }

  [[nodiscard]] JSObjectRef newObject() const { return JSObjectMake(ctx_, nullptr, nullptr); }

  [[nodiscard]] JSValueRef getObjectProperty(const JSObjectRef& obj,
                                             const char* propertyName) const {
    return JSObjectGetProperty(ctx_, obj, JscStringRAII(propertyName), nullptr);
  }

  int setObjectProperty(const JSObjectRef& obj, const char* propertyName, const JSValueRef& value) {
    JSObjectSetProperty(ctx_, obj, JscStringRAII(propertyName), value, kJSPropertyAttributeNone,
                        nullptr);
    return 0;
  }

  int setObjectFunction(JSObjectRef obj,
                        const char* functionName,
                        JSObjectCallAsFunctionCallback cppFunction,
                        int expectingArgc) {
    JscStringRAII funcNameStr = functionName;
    JSObjectRef func = JSObjectMakeFunctionWithCallback(ctx_, funcNameStr, cppFunction);
    JSObjectSetProperty(ctx_, obj, funcNameStr, func, kJSPropertyAttributeNone, nullptr);
    return 0;
  }

  [[nodiscard]] JSObjectRef toObject(const JSValueRef& value) const {
    return JSValueToObject(ctx_, value, nullptr);
  }

  [[nodiscard]] JSValueRef toJsString(const char* str) const {
    return JSValueMakeString(ctx_, JscStringRAII(str));
  }

  [[nodiscard]] JSValueRef toJsString(const std::string& str) const {
    return toJsString(str.c_str());
  }

  [[nodiscard]] std::string toStdString(const JSValueRef& value) const {
    JscStringRAII jsStr = JSValueToStringCopy(ctx_, value, nullptr);
    size_t bufferSize = JSStringGetMaximumUTF8CStringSize(jsStr);
    std::vector<char> buffer(bufferSize);
    JSStringGetUTF8CString(jsStr, buffer.data(), bufferSize);
    return buffer.data();
  }

  [[nodiscard]] JSValueRef toJsBool(bool value) const { return JSValueMakeBoolean(ctx_, value); }

  [[nodiscard]] bool toBool(const JSValueRef& value) const { return JSValueToBoolean(ctx_, value); }

  [[nodiscard]] JSValueRef toJsInt(size_t value) const {
    return JSValueMakeNumber(ctx_, static_cast<double>(value));
  }

  [[nodiscard]] size_t toInt(const JSValueRef& value) const {
    return static_cast<size_t>(JSValueToNumber(ctx_, value, nullptr));
  }

  [[nodiscard]] JSValueRef toJsDouble(double value) const { return JSValueMakeNumber(ctx_, value); }

  [[nodiscard]] double toDouble(const JSValueRef& value) const {
    return JSValueToNumber(ctx_, value, nullptr);
  }

  JSValueRef callFunction(JSObjectRef func, JSObjectRef thisArg, int argc, JSValueRef* argv) const {
    JSValueRef exception = nullptr;
    auto* thisVal = isUndefined(thisArg) ? nullptr : thisArg;
    JSValueRef result = JSObjectCallAsFunction(ctx_, func, thisVal, argc, argv, &exception);
    logErrorStackTrace(exception, __FILE_NAME__, __LINE__);
    return result;
  }

  JSObjectRef newClassInstance(const JSObjectRef& clazz, int argc, JSValueRef* argv) {
    JSValueRef exception = nullptr;
    JSObjectRef result = JSObjectCallAsConstructor(ctx_, clazz, argc, argv, &exception);
    if (exception != nullptr) {
      logErrorStackTrace(exception, __FILE_NAME__, __LINE__);
      return nullptr;
    }
    return result;
  }

  JSValueRef getJsClassHavingMethod(const JSValueRef& module, const char* methodName) const {
    JSObjectRef globalObj = JSContextGetGlobalObject(ctx_);
    JSPropertyNameArrayRef propertyNames = JSObjectCopyPropertyNames(ctx_, globalObj);
    size_t count = JSPropertyNameArrayGetCount(propertyNames);

    for (size_t i = count - 1; i >= 0; i--) {
      JSStringRef propertyName = JSPropertyNameArrayGetNameAtIndex(propertyNames, i);
      JSValueRef value = JSObjectGetProperty(ctx_, globalObj, propertyName, nullptr);

      if (JSValueIsObject(ctx_, value)) {
        JSObjectRef obj = JSValueToObject(ctx_, value, nullptr);
        JSValueRef prototype = JSObjectGetProperty(ctx_, obj, JscStringRAII("prototype"), nullptr);

        if (!JSValueIsUndefined(ctx_, prototype) && JSValueIsObject(ctx_, prototype)) {
          JSObjectRef prototypeObj = JSValueToObject(ctx_, prototype, nullptr);
          JSValueRef method =
              JSObjectGetProperty(ctx_, prototypeObj, JscStringRAII(methodName), nullptr);

          if (!JSValueIsUndefined(ctx_, method) && JSValueIsObject(ctx_, method)) {
            JSPropertyNameArrayRelease(propertyNames);
            return value;
          }
        }
      }
    }

    JSPropertyNameArrayRelease(propertyNames);
    return undefined();
  }

  JSObjectRef getMethodOfClassOrInstance(JSObjectRef jsClass,
                                         JSObjectRef instance,
                                         const char* methodName) const {
    JSValueRef exception = nullptr;
    JSValueRef method = JSObjectGetProperty(ctx_, instance, JscStringRAII(methodName), &exception);
    logErrorStackTrace(exception, __FILE_NAME__, __LINE__);
    return toObject(method);
  }

  JSValueRef throwError(JsErrorType errorType, const char* format, ...) const {
    va_list args;
    va_start(args, format);
    char buffer[1024];
    vsnprintf(buffer, sizeof(buffer), format, args);
    va_end(args);

    return JSValueMakeString(ctx_, JscStringRAII(static_cast<char*>(buffer)));
  }

  [[nodiscard]] bool isObject(const JSValueRef& value) const {
    return JSValueIsObject(ctx_, value);
  }

  [[nodiscard]] bool isNull(const JSValueRef& value) const { return JSValueIsNull(ctx_, value); }

  [[nodiscard]] bool isUndefined(const JSValueRef& value) const {
    return JSValueIsUndefined(ctx_, value);
  }

  [[nodiscard]] bool isException(const JSValueRef& value) const {
    // nullptr is returned if an exception is thrown in `callFunction` and `newClassInstance`
    return value == nullptr;
  }

  void logErrorStackTrace(const JSValueRef& exception,
                          const char* file = __FILE_NAME__,
                          int line = __LINE__) const {
    if (exception == nullptr) {
      return;
    }
    JscStringRAII exceptionStr = JSValueToStringCopy(ctx_, exception, nullptr);
    size_t bufferSize = JSStringGetMaximumUTF8CStringSize(exceptionStr);
    std::vector<char> buffer(bufferSize);
    JSStringGetUTF8CString(exceptionStr, buffer.data(), bufferSize);
    LOG(ERROR) << "[qjs] JS exception at " << file << ':' << line << " => " << buffer.data();
  }

  void freeValue(const JSValueRef& value) const {
    // JavaScriptCore handles memory management automatically
  }

  template <typename... Args>
  void freeValue(const JSValueRef& first, const Args&... rest) const {
    freeValue(first);
    freeValue(rest...);
  }

  void freeValue() const {}

  template <typename T_RIME_TYPE>
  void registerType(JsWrapper<T_RIME_TYPE, JSValueRef>& wrapper) {
    auto key = engineId_ + typeid(T_RIME_TYPE).name();
    const char* typeName = JsWrapper<T_RIME_TYPE, JSValueRef>::getTypeName();
    if (clazzes.find(key) != clazzes.end()) {
      LOG(INFO) << "[jsc] type: " << typeName << " has already been registered.";
      return;
    }
    DLOG(INFO) << "[jsc] registering type: " << typeName;

    // the counts would be available after getting the properties and getters
    auto properties = wrapper.getPropertiesJsc(ctx_);
    auto getters = wrapper.getGettersJsc(ctx_);
    std::vector<JSStaticValue> staticValues(wrapper.getPropertiesCount() +
                                            wrapper.getGettersCount() + 1);
    for (int i = 0; i < wrapper.getPropertiesCount(); i++) {
      staticValues[i] = properties[i];
    }
    for (int i = 0; i < wrapper.getGettersCount(); i++) {
      staticValues[wrapper.getPropertiesCount() + i] = getters[i];
    }
    staticValues.back() = {nullptr, nullptr, nullptr, 0};

    JSClassDefinition classDef = {.version = 0,
                                  .attributes = kJSClassAttributeNone,
                                  .className = typeName,
                                  .parentClass = nullptr,
                                  .staticValues = staticValues.data(),
                                  .staticFunctions = wrapper.getFunctionsJsc(ctx_),
                                  .initialize = nullptr,
                                  .finalize = wrapper.getFinalizerJsc(),
                                  .hasProperty = nullptr,
                                  .getProperty = nullptr,
                                  .setProperty = nullptr,
                                  .deleteProperty = nullptr,
                                  .getPropertyNames = nullptr,
                                  .callAsFunction = nullptr,
                                  .callAsConstructor = wrapper.getConstructorJsc(),
                                  .hasInstance = nullptr,
                                  .convertToType = nullptr};

    DLOG(INFO) << "[jsc] registering type: " << typeName << " with " << (staticValues.size() - 1)
               << " properties and " << wrapper.getFunctionsCount() << " functions";

    JSClassRef jsClass = JSClassCreate(&classDef);
    clazzes[key] = std::make_tuple(typeName, jsClass, classDef, staticValues);

    // Add the constructor to the global object
    JSObjectRef globalObj = JSContextGetGlobalObject(ctx_);
    JSObjectRef constructorObj = JSObjectMake(ctx_, jsClass, nullptr);
    JSObjectSetProperty(ctx_, globalObj, JscStringRAII(typeName), constructorObj,
                        kJSPropertyAttributeNone, nullptr);
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
    const auto* type = typeid(T).name();
    auto key = engineId_ + type;
    if (clazzes.find(key) == clazzes.end()) {
      LOG(ERROR) << "type: " << type << " has not been registered.";
      return nullptr;
    }

    if (void* ptr = JSObjectGetPrivate(toObject(value))) {
      return static_cast<T*>(ptr);
    }
    return nullptr;
  }

  template <typename T>
  std::shared_ptr<T> unwrapShared(const JSValueRef& value) const {
    if (void* ptr = JSObjectGetPrivate(toObject(value))) {
      if (auto sharedPtr = static_cast<std::shared_ptr<T>*>(ptr)) {
        return *sharedPtr;
      }
    }
    return nullptr;
  }

  template <typename T>
  JSObjectRef wrap(T* ptrValue) const {
    if (!ptrValue) {
      return nullptr;
    }

    const auto* type = typeid(T).name();
    auto key = engineId_ + type;
    if (clazzes.find(key) == clazzes.end()) {
      LOG(ERROR) << "type: " << type << " has not been registered.";
      return nullptr;
    }

    JSClassRef jsClass = std::get<1>(clazzes[key]);
    return JSObjectMake(ctx_, jsClass, ptrValue);
  }

  template <typename T>
  JSObjectRef wrapShared(const std::shared_ptr<T>& value) const {
    if (!value) {
      return nullptr;
    }

    const auto* type = typeid(T).name();
    auto key = engineId_ + type;
    if (clazzes.find(key) == clazzes.end()) {
      LOG(ERROR) << "type: " << type << " has not been registered.";
      return nullptr;
    }

    JSClassRef jsClass = std::get<1>(clazzes[key]);
    auto ptr = std::make_unique<std::shared_ptr<T>>(value);
    return JSObjectMake(ctx_, jsClass, ptr.release());
  }

  void setBaseFolderPath(const char* absolutePath) { baseFolderPath_ = absolutePath; }

  JSValueRef loadJsFile(const char* fileName) const {
    JSValueRef exception = nullptr;
    const auto* globalThis =
        JscCodeLoader::loadJsModuleToGlobalThis(ctx_, baseFolderPath_, fileName, &exception);
    if (exception != nullptr) {
      logErrorStackTrace(exception, __FILE_NAME__, __LINE__);
      return undefined();
    }
    return globalThis;
  }

  JSValueRef eval(const char* code, const char* filename = "<eval>") {
    JscStringRAII jsCode = code;
    JscStringRAII filenameStr = filename;
    JSValueRef exception = nullptr;
    JSValueRef result = JSEvaluateScript(ctx_, jsCode, nullptr, filenameStr, 0, &exception);
    logErrorStackTrace(exception, __FILE_NAME__, __LINE__);
    return result;
  }

  JSValueRef getGlobalObject() { return JSContextGetGlobalObject(ctx_); }
};
