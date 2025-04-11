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

  std::string baseFolderPath_;

  JSValueRef lastException_{nullptr};

  // the types should be registered to each jsc context
  using ClassDefTuple = std::tuple<JSClassRef, JSClassDefinition, std::vector<JSStaticValue>>;
  std::unordered_map<std::string, ClassDefTuple> clazzes_;

  inline static std::unordered_map<JSContextRef, JsEngine<JSValueRef>*> engines{};

public:
  JsEngine<JSValueRef>() : ctx_(JSGlobalContextCreate(nullptr)) {
    engines[ctx_] = this;
    JscCodeLoader::exposeLogToJsConsole(ctx_);
  }

  ~JsEngine<JSValueRef>() {
    for (auto& clazz : clazzes_) {
      auto& clazzDef = std::get<0>(clazz.second);
      JSClassRelease(clazzDef);
    }

    engines.erase(ctx_);
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
      throw std::runtime_error("JsEngine<JSValueRef> not found");
    }
    return *it->second;
  }

  // NOLINTBEGIN(readability-convert-member-functions-to-static)
  int64_t getMemoryUsage() {
    // TODO: find an API to get the memory usage of JavaScriptCore
    return -1;
  }

  [[nodiscard]] bool isException(const JSValueRef& value) const {
    // the thrown exception in `callFunction` and `newClassInstance` was set to `lastException_`
    return lastException_ != nullptr;
  }

  [[nodiscard]] JSValueRef duplicateValue(const JSValueRef& value) const {
    // JavaScriptCore handles memory management automatically
    return value;
  }

  template <typename... Args>
  void freeValue(const Args&... args) const {
    // JavaScriptCore handles memory management automatically
  }
  // NOLINTEND(readability-convert-member-functions-to-static)

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

  void insertItemToArray(JSValueRef array, size_t index, const JSValueRef& value) const {
    JSObjectRef arrayObj = JSValueToObject(ctx_, array, nullptr);
    JSObjectSetPropertyAtIndex(ctx_, arrayObj, index, value, nullptr);
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

  JSValueRef callFunction(JSObjectRef func, JSObjectRef thisArg, int argc, JSValueRef* argv) {
    lastException_ = nullptr;
    auto* thisVal = isUndefined(thisArg) ? nullptr : thisArg;
    JSValueRef result = JSObjectCallAsFunction(ctx_, func, thisVal, argc, argv, &lastException_);
    logErrorStackTrace(lastException_, __FILE_NAME__, __LINE__);
    return result;
  }

  JSObjectRef newClassInstance(const JSObjectRef& clazz, int argc, JSValueRef* argv) {
    lastException_ = nullptr;
    JSObjectRef result = JSObjectCallAsConstructor(ctx_, clazz, argc, argv, &lastException_);
    if (lastException_ != nullptr) {
      logErrorStackTrace(lastException_, __FILE_NAME__, __LINE__);
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
                                         const char* methodName) {
    lastException_ = nullptr;
    JSValueRef method =
        JSObjectGetProperty(ctx_, instance, JscStringRAII(methodName), &lastException_);
    logErrorStackTrace(lastException_, __FILE_NAME__, __LINE__);
    return toObject(method);
  }

  [[nodiscard]] JSValueRef throwError(JsErrorType errorType, const std::string& message) const {
    return JSValueMakeString(ctx_, JscStringRAII(message.c_str()));
  }

  [[nodiscard]] bool isFunction(const JSValueRef& value) const {
    return isObject(value) && JSObjectIsFunction(ctx_, toObject(value));
  }

  [[nodiscard]] bool isObject(const JSValueRef& value) const {
    return JSValueIsObject(ctx_, value);
  }

  [[nodiscard]] bool isNull(const JSValueRef& value) const { return JSValueIsNull(ctx_, value); }

  [[nodiscard]] bool isUndefined(const JSValueRef& value) const {
    return JSValueIsUndefined(ctx_, value);
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

  template <typename T_RIME_TYPE>
  void registerType() {
    using WRAPPER = JsWrapper<T_RIME_TYPE, JSValueRef>;
    const char* typeName = WRAPPER::TYPENAME;
    if (clazzes_.find(typeName) != clazzes_.end()) {
      LOG(INFO) << "[jsc] type: " << typeName << " has already been registered.";
      return;
    }
    DLOG(INFO) << "[jsc] registering type: " << typeName;

    std::vector<JSStaticValue> staticValues;
    auto& properties = WRAPPER::propertiesJsc;
    for (int i = 0; i < WRAPPER::PROPERTIES_SIZE; i++) {
      staticValues.push_back(properties[i]);
    }
    auto& getters = WRAPPER::gettersJsc;
    for (int i = 0; i < WRAPPER::GETTERS_SIZE; i++) {
      staticValues.push_back(getters[i]);
    }
    staticValues.push_back({nullptr, nullptr, nullptr, 0});

    JSClassDefinition classDef = {.version = 0,
                                  .attributes = kJSClassAttributeNone,
                                  .className = typeName,
                                  .parentClass = nullptr,
                                  .staticValues = staticValues.data(),
                                  .staticFunctions = WRAPPER::functionsJsc,
                                  .initialize = nullptr,
                                  .finalize = WRAPPER::finalizerJsc,
                                  .hasProperty = nullptr,
                                  .getProperty = nullptr,
                                  .setProperty = nullptr,
                                  .deleteProperty = nullptr,
                                  .getPropertyNames = nullptr,
                                  .callAsFunction = nullptr,
                                  .callAsConstructor = WRAPPER::constructorJsc,
                                  .convertToType = nullptr};

    DLOG(INFO) << "[jsc] registering type: " << typeName << " with " << (staticValues.size() - 1)
               << " properties and " << countof(WRAPPER::functionsJsc) << " functions";

    JSClassRef jsClass = JSClassCreate(&classDef);
    clazzes_[typeName] = std::make_tuple(jsClass, classDef, staticValues);

    // Add the constructor to the global object
    JSObjectRef globalObj = JSContextGetGlobalObject(ctx_);
    JSObjectRef constructorObj = JSObjectMake(ctx_, jsClass, nullptr);
    JSObjectSetProperty(ctx_, globalObj, JscStringRAII(typeName), constructorObj,
                        kJSPropertyAttributeNone, nullptr);
  }

  template <typename T>
  [[nodiscard]] bool isTypeRegistered() const {
    const char* typeName = JsWrapper<T, JSValueRef>::TYPENAME;
    if (clazzes_.find(typeName) == clazzes_.end()) {
      LOG(ERROR) << "type: " << typeName << " has not been registered.";
      return false;
    }
    return true;
  }

  template <typename T>
  T* unwrap(const JSValueRef& value) {
    if (!value || !isTypeRegistered<T>() || !isObject(value)) {
      return nullptr;
    }
    if (auto* ptr = JSObjectGetPrivate(toObject(value))) {
      return static_cast<T*>(ptr);
    }
    return nullptr;
  }

  template <typename T>
  std::shared_ptr<T> unwrapShared(const JSValueRef& value) const {
    if (!value || !isTypeRegistered<T>() || !isObject(value)) {
      return nullptr;
    }
    if (void* ptr = isTypeRegistered<T>() ? JSObjectGetPrivate(toObject(value)) : nullptr) {
      if (auto sharedPtr = static_cast<std::shared_ptr<T>*>(ptr)) {
        return *sharedPtr;
      }
    }
    return nullptr;
  }

  template <typename T>
  JSObjectRef wrap(T* ptrValue) const {
    if (!ptrValue || !isTypeRegistered<T>()) {
      return nullptr;
    }

    JSClassRef jsClass = std::get<0>(clazzes_.at(JsWrapper<T, JSValueRef>::TYPENAME));
    return JSObjectMake(ctx_, jsClass, ptrValue);
  }

  template <typename T>
  JSObjectRef wrapShared(const std::shared_ptr<T>& value) const {
    if (!value || !isTypeRegistered<T>()) {
      return nullptr;
    }

    JSClassRef jsClass = std::get<0>(clazzes_.at(JsWrapper<T, JSValueRef>::TYPENAME));
    auto ptr = std::make_unique<std::shared_ptr<T>>(value);
    return JSObjectMake(ctx_, jsClass, ptr.release());
  }

  void setBaseFolderPath(const char* absolutePath) { baseFolderPath_ = absolutePath; }

  JSValueRef loadJsFile(const char* fileName) {
    lastException_ = nullptr;
    const auto* globalThis =
        JscCodeLoader::loadJsModuleToGlobalThis(ctx_, baseFolderPath_, fileName, &lastException_);
    if (lastException_ != nullptr) {
      logErrorStackTrace(lastException_, __FILE_NAME__, __LINE__);
      return undefined();
    }
    return globalThis;
  }

  JSValueRef eval(const char* code, const char* filename = "<eval>") {
    JscStringRAII jsCode = code;
    JscStringRAII filenameStr = filename;
    lastException_ = nullptr;
    JSValueRef result = JSEvaluateScript(ctx_, jsCode, nullptr, filenameStr, 0, &lastException_);
    logErrorStackTrace(lastException_, __FILE_NAME__, __LINE__);
    return result;
  }

  JSValueRef getGlobalObject() { return JSContextGetGlobalObject(ctx_); }
};
