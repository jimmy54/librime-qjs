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
#include "engines/js_macros.h"
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
    logErrorStackTrace(exception, __FILE_NAME__, __LINE__);
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

  int setObjectProperty(const JSObjectRef& obj, const char* propertyName, const JSValueRef& value) {
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

  JSValueRef getJsClassHavingMethod(const JSValueRef& module, const char* methodName) {
    JSObjectRef globalObj = JSContextGetGlobalObject(ctx_);
    JSPropertyNameArrayRef propertyNames = JSObjectCopyPropertyNames(ctx_, globalObj);
    size_t count = JSPropertyNameArrayGetCount(propertyNames);

    // TODO: Implement proper isolation by loading JavaScript files into separate contexts
    // Currently using a workaround that finds the most recently loaded class with the method
    // Note: This is unstable - if the latest file doesn't contain the expected method, it may return an incorrect class from previously loaded files
    for (size_t i = count - 1; i >= 0; i--) {
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
    return undefined();
  }

  JSObjectRef getMethodOfClassOrInstance(JSObjectRef jsClass,
                                         JSObjectRef instance,
                                         const char* methodName) {
    JSStringRef methodStr = JSStringCreateWithUTF8CString(methodName);
    JSValueRef exception = nullptr;
    JSValueRef method = JSObjectGetProperty(ctx_, instance, methodStr, &exception);
    JSStringRelease(methodStr);
    logErrorStackTrace(exception, __FILE_NAME__, __LINE__);
    return toObject(method);
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

  bool isException(const JSValueRef& value) {
    // nullptr is returned if an exception is thrown in `callFunction` and `newClassInstance`
    return value == nullptr;
  }

  void logErrorStackTrace(const JSValueRef& exception,
                          const char* file = __FILE_NAME__,
                          int line = __LINE__) {
    if (exception == nullptr) {
      return;
    }
    JSStringRef exceptionStr = JSValueToStringCopy(ctx_, exception, nullptr);
    size_t bufferSize = JSStringGetMaximumUTF8CStringSize(exceptionStr);
    std::vector<char> buffer(bufferSize);
    JSStringGetUTF8CString(exceptionStr, buffer.data(), bufferSize);
    LOG(ERROR) << "[qjs] JS exception at " << file << ':' << line << " => " << buffer.data()
               << '\n';
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
      LOG(INFO) << "[jsc] type: " << typeName << " has already been registered.";
      return;
    }
    LOG(INFO) << "[jsc] registering type: " << typeName;

    // the counts would be available after getting the properties and getters
    auto properties = wrapper.getPropertiesJsc();
    auto getters = wrapper.getGettersJsc();
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
                                  .staticFunctions = wrapper.getFunctionsJsc(),
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
    JSStringRef classNameStr = JSStringCreateWithUTF8CString(typeName);
    JSObjectSetProperty(ctx_, globalObj, classNameStr, constructorObj, kJSPropertyAttributeNone,
                        nullptr);
    JSStringRelease(classNameStr);

    JSStringRelease(classNameStr);
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
    const auto* globalThis =
        JscCodeLoader::loadJsModuleToGlobalThis(ctx_, baseFolderPath_, fileName, &exception);
    if (exception != nullptr) {
      logErrorStackTrace(exception, __FILE_NAME__, __LINE__);
      return undefined();
    }
    return globalThis;
  }

  JSValueRef eval(const char* code, const char* filename = "<eval>") {
    JSStringRef jsCode = JSStringCreateWithUTF8CString(code);
    JSValueRef exception = nullptr;
    JSValueRef result = JSEvaluateScript(ctx_, jsCode, nullptr,
                                         JSStringCreateWithUTF8CString(filename), 0, &exception);
    JSStringRelease(jsCode);
    logErrorStackTrace(exception, __FILE_NAME__, __LINE__);
    return result;
  }

  JSValueRef getGlobalObject() { return JSContextGetGlobalObject(ctx_); }

private:
  JsEngine<JSValueRef>() : ctx_(JSGlobalContextCreate(nullptr)) {
    JscCodeLoader::exposeLogToJsConsole(ctx_);
  }

  ~JsEngine<JSValueRef>() { JSGlobalContextRelease(ctx_); }

  JSGlobalContextRef ctx_{nullptr};

  std::string baseFolderPath_;

  inline static std::unordered_map<
      std::string,
      std::tuple<std::string, JSClassRef, JSClassDefinition, std::vector<JSStaticValue>>>
      clazzes{};
};
