#pragma once

#include <JavaScriptCore/JavaScript.h>
#include <memory>

#include "engines/javascriptcore/jsc_engine_impl.h"
#include "engines/javascriptcore/jsc_string_raii.hpp"
#include "engines/js_engine.h"

template <>
class JsEngine<JSValueRef> {
  std::unique_ptr<JscEngineImpl> impl_{std::make_unique<JscEngineImpl>()};
  inline static std::unordered_map<JSContextRef, JsEngine<JSValueRef>*> engines{};

public:
  JsEngine<JSValueRef>() { engines[impl_->getContext()] = this; }

  ~JsEngine<JSValueRef>() { engines.erase(impl_->getContext()); }

  JsEngine(const JsEngine&) = delete;
  JsEngine(JsEngine&&) = delete;
  JsEngine& operator=(const JsEngine& other) {
    if (this != &other) {
      impl_ = std::make_unique<JscEngineImpl>(*other.impl_);
      engines[impl_->getContext()] = this;
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
    return impl_->getLastException() != nullptr;
  }

  [[nodiscard]] JSValueRef duplicateValue(const JSValueRef& value) const {
    // JavaScriptCore handles memory management automatically
    return value;
  }

  template <typename... Args>
  void freeValue(const Args&... args) const {
    // JavaScriptCore handles memory management automatically
  }

  template <typename... Args>
  void protectFromGC(const Args&... args) const {
    (((args != nullptr && !isUndefined(args)) ? JSValueProtect(impl_->getContext(), args) : void()),
     ...);
  }

  template <typename... Args>
  void unprotectFromGC(const Args&... args) const {
    (((args != nullptr && !isUndefined(args)) ? JSValueUnprotect(impl_->getContext(), args)
                                              : void()),
     ...);
  }

  // NOLINTEND(readability-convert-member-functions-to-static)
  [[nodiscard]] JSValueRef null() const { return JSValueMakeNull(impl_->getContext()); }
  [[nodiscard]] JSValueRef undefined() const { return JSValueMakeUndefined(impl_->getContext()); }

  [[nodiscard]] JSValueRef jsTrue() const { return JSValueMakeBoolean(impl_->getContext(), true); }
  [[nodiscard]] JSValueRef jsFalse() const {
    return JSValueMakeBoolean(impl_->getContext(), false);
  }

  [[nodiscard]] JSValueRef newArray() const {
    return JSObjectMakeArray(impl_->getContext(), 0, nullptr, nullptr);
  }

  [[nodiscard]] size_t getArrayLength(const JSValueRef& array) const {
    return impl_->getArrayLength(array);
  }

  void insertItemToArray(JSValueRef array, size_t index, const JSValueRef& value) const {
    impl_->insertItemToArray(array, index, value);
  }

  [[nodiscard]] JSValueRef getArrayItem(const JSValueRef& array, size_t index) const {
    return impl_->getArrayItem(array, index);
  }

  [[nodiscard]] JSObjectRef newObject() const {
    return JSObjectMake(impl_->getContext(), nullptr, nullptr);
  }

  [[nodiscard]] JSValueRef getObjectProperty(const JSObjectRef& obj,
                                             const char* propertyName) const {
    return impl_->getObjectProperty(obj, propertyName);
  }

  int setObjectProperty(const JSObjectRef& obj, const char* propertyName, const JSValueRef& value) {
    return impl_->setObjectProperty(obj, propertyName, value);
  }

  int setObjectFunction(JSObjectRef obj,
                        const char* functionName,
                        JSObjectCallAsFunctionCallback cppFunction,
                        int expectingArgc) {
    return impl_->setObjectFunction(obj, functionName, cppFunction, expectingArgc);
  }

  [[nodiscard]] JSObjectRef toObject(const JSValueRef& value) const {
    return JSValueToObject(impl_->getContext(), value, nullptr);
  }

  [[nodiscard]] JSValueRef toJsString(const char* str) const {
    return JSValueMakeString(impl_->getContext(), JscStringRAII(str));
  }

  [[nodiscard]] JSValueRef toJsString(const std::string& str) const {
    return toJsString(str.c_str());
  }

  [[nodiscard]] std::string toStdString(const JSValueRef& value) const {
    return impl_->toStdString(value);
  }

  [[nodiscard]] JSValueRef toJsBool(bool value) const {
    return JSValueMakeBoolean(impl_->getContext(), value);
  }

  [[nodiscard]] bool toBool(const JSValueRef& value) const {
    return JSValueToBoolean(impl_->getContext(), value);
  }

  [[nodiscard]] JSValueRef toJsInt(size_t value) const {
    return JSValueMakeNumber(impl_->getContext(), static_cast<double>(value));
  }

  [[nodiscard]] size_t toInt(const JSValueRef& value) const {
    return static_cast<size_t>(JSValueToNumber(impl_->getContext(), value, nullptr));
  }

  [[nodiscard]] JSValueRef toJsDouble(double value) const {
    return JSValueMakeNumber(impl_->getContext(), value);
  }

  [[nodiscard]] double toDouble(const JSValueRef& value) const {
    return JSValueToNumber(impl_->getContext(), value, nullptr);
  }

  JSValueRef callFunction(JSObjectRef func, JSObjectRef thisArg, int argc, JSValueRef* argv) const {
    return impl_->callFunction(func, thisArg, argc, argv);
  }

  JSObjectRef newClassInstance(const JSObjectRef& clazz, int argc, JSValueRef* argv) {
    return impl_->newClassInstance(clazz, argc, argv);
  }

  JSValueRef getJsClassHavingMethod(const JSValueRef& module, const char* methodName) const {
    return impl_->getJsClassHavingMethod(module, methodName);
  }

  JSObjectRef getMethodOfClassOrInstance(JSObjectRef jsClass,
                                         JSObjectRef instance,
                                         const char* methodName) {
    return impl_->getMethodOfClassOrInstance(jsClass, instance, methodName);
  }

  [[nodiscard]] JSValueRef throwError(JsErrorType errorType, const std::string& message) const {
    return JSValueMakeString(impl_->getContext(), JscStringRAII(message.c_str()));
  }

  [[nodiscard]] bool isFunction(const JSValueRef& value) const {
    return isObject(value) && JSObjectIsFunction(impl_->getContext(), toObject(value));
  }

  [[nodiscard]] bool isArray(const JSValueRef& value) const {
    return JSValueIsArray(impl_->getContext(), value);
  }

  [[nodiscard]] bool isObject(const JSValueRef& value) const {
    return value != nullptr && JSValueIsObject(impl_->getContext(), value);
  }

  [[nodiscard]] bool isNull(const JSValueRef& value) const {
    return JSValueIsNull(impl_->getContext(), value);
  }

  [[nodiscard]] bool isUndefined(const JSValueRef& value) const {
    return JSValueIsUndefined(impl_->getContext(), value);
  }

  void logErrorStackTrace(const JSValueRef& exception, const char* file, int line) const {
    impl_->logErrorStackTrace(exception, file, line);
  }

  template <typename T_RIME_TYPE>
  void registerType() {
    using WRAPPER = JsWrapper<T_RIME_TYPE, JSValueRef>;
    impl_->registerType(WRAPPER::TYPENAME, WRAPPER::classDefJsc, WRAPPER::constructorJsc,
                        WRAPPER::finalizerJsc, WRAPPER::functionsJsc, WRAPPER::FUNCTIONS_SIZE,
                        WRAPPER::propertiesJsc, WRAPPER::PROPERTIES_SIZE, WRAPPER::gettersJsc,
                        WRAPPER::GETTERS_SIZE);
  }

  template <typename T>
  [[nodiscard]] bool isTypeRegistered() const {
    return impl_->isTypeRegistered(JsWrapper<T, JSValueRef>::TYPENAME);
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

    const JSClassRef& jsClass = impl_->getRegisteredClass(JsWrapper<T, JSValueRef>::TYPENAME);
    return JSObjectMake(impl_->getContext(), jsClass, ptrValue);
  }

  template <typename T>
  JSObjectRef wrapShared(std::shared_ptr<T> value) const {  // pass by value to copy the shared_ptr
    if (!value || !isTypeRegistered<T>()) {
      return nullptr;
    }

    const JSClassRef& jsClass = impl_->getRegisteredClass(JsWrapper<T, JSValueRef>::TYPENAME);
    auto ptr = std::make_unique<std::shared_ptr<T>>(value);
    return JSObjectMake(impl_->getContext(), jsClass, ptr.release());
  }

  void setBaseFolderPath(const char* absolutePath) { impl_->setBaseFolderPath(absolutePath); }

  JSValueRef loadJsFile(const char* fileName) { return impl_->loadJsFile(fileName); }

  JSValueRef eval(const char* code, const char* filename = "<eval>") {
    return impl_->eval(code, filename);
  }

  JSValueRef getGlobalObject() { return impl_->getGlobalObject(); }
};
