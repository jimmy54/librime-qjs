#pragma once

#include <glog/logging.h>
#include <quickjs.h>
#include <cstdint>
#include <memory>
#include <string>
#include <tuple>
#include <unordered_map>
#include <utility>

#include "engines/js_engine.h"
#include "engines/quickjs/quickjs_code_loader.h"
#include "patch/quickjs/node_module_loader.h"
#include "types/js_wrapper.h"

template <>
class JsEngine<JSValue> {
public:
  JsEngine(const JsEngine&) = delete;
  JsEngine(JsEngine&&) = delete;
  JsEngine& operator=(const JsEngine&) = delete;
  JsEngine& operator=(JsEngine&&) = delete;

  static JsEngine<JSValue>& getInstance() {
    static JsEngine<JSValue> instance;
    return instance;
  }

  int64_t getMemoryUsage() {
    JSMemoryUsage qjsMemStats;
    JS_ComputeMemoryUsage(rt_, &qjsMemStats);
    return qjsMemStats.memory_used_size;
  }

  typename TypeMap<JSValue>::ContextType& getContext() { return ctx_; }

  template <typename T>
  void* getOpaque(JSValue value) {
    auto classId = getJsClassId<T>();
    return JS_GetOpaque(value, classId);
  }

  void setOpaque(JSValue value, void* opaque) { JS_SetOpaque(value, opaque); }

  JSValue null() { return JS_NULL; }
  JSValue undefined() { return JS_UNDEFINED; }

  JSValue jsTrue() { return JS_TRUE; }
  JSValue jsFalse() { return JS_FALSE; }

  JSValue newArray() { return JS_NewArray(ctx_); }

  size_t getArrayLength(const JSValue& array) {
    auto lengthVal = JS_GetPropertyStr(ctx_, array, "length");
    if (JS_IsException(lengthVal)) {
      return 0;
    }

    uint32_t length = 0;
    JS_ToUint32(ctx_, &length, lengthVal);
    JS_FreeValue(ctx_, lengthVal);

    return length;
  }

  int insertItemToArray(JSValue array, size_t index, const JSValue& value) {
    return JS_SetPropertyUint32(ctx_, array, index, value);
  }

  JSValue getArrayItem(const JSValue& array, size_t index) {
    return JS_GetPropertyUint32(ctx_, array, index);
  }

  JSValue newObject() { return JS_NewObject(ctx_); }

  JSValue getObjectProperty(const JSValue& obj, const char* propertyName) {
    return JS_GetPropertyStr(ctx_, obj, propertyName);
  }

  int setObjectProperty(JSValue obj, const char* propertyName, const JSValue& value) {
    return JS_SetPropertyStr(ctx_, obj, propertyName, value);
  }

  using ExposeFunction = JSCFunction*;
  int setObjectFunction(JSValue obj,
                        const char* functionName,
                        ExposeFunction cppFunction,
                        int expectingArgc) {
    return JS_SetPropertyStr(ctx_, obj, functionName,
                             JS_NewCFunction(ctx_, cppFunction, functionName, expectingArgc));
  }

  JSValue toObject(const JSValue& value) { return value; }

  JSValue toJsString(const char* str) { return JS_NewString(ctx_, str); }
  JSValue toJsString(const std::string& str) { return JS_NewString(ctx_, str.c_str()); }

  std::string toStdString(const JSValue& value) {
    const char* str = JS_ToCString(ctx_, value);
    std::string ret(str);
    JS_FreeCString(ctx_, str);
    return ret;
  }

  JSValue toJsBool(bool value) { return JS_NewBool(ctx_, value); }

  bool toBool(const JSValue& value) { return JS_ToBool(ctx_, value); }

  JSValue toJsInt(size_t value) { return JS_NewInt64(ctx_, static_cast<int64_t>(value)); }

  size_t toInt(const JSValue& value) {
    uint32_t ret = 0;
    JS_ToUint32(ctx_, &ret, value);
    return ret;
  }

  JSValue toJsDouble(double value) { return JS_NewFloat64(ctx_, value); }

  double toDouble(const JSValue& value) {
    double ret = 0;
    JS_ToFloat64(ctx_, &ret, value);
    return ret;
  }

  JSValue callFunction(const JSValue& func, const JSValue& thisArg, int argc, JSValue* argv) {
    return JS_Call(ctx_, func, thisArg, argc, argv);
  }

  JSValue callConstructor(JSValue func, int argc, JSValue* argv) {
    return JS_CallConstructor(ctx_, func, argc, argv);
  }

  JSValue getJsClassHavingMethod(const JSValue& module, const char* methodName) {
    return QuickJSCodeLoader::getExportedClassHavingMethodNameInModule(ctx_, module, methodName);
  }
  JSValue getMethodOfClass(JSValue jsClass, const char* methodName) {
    return QuickJSCodeLoader::getMethodByNameInClass(ctx_, jsClass, methodName);
  }

  JSValue throwError(JsErrorType errorType, const char* format, ...) {
    JSValue ret;
    va_list args;
    va_start(args, format);
    switch (errorType) {
      case JsErrorType::SYNTAX:
      case JsErrorType::EVAL:
        ret = JS_ThrowSyntaxError(ctx_, format, args);
        break;
      case JsErrorType::RANGE:
        ret = JS_ThrowRangeError(ctx_, format, args);
        break;
      case JsErrorType::REFERENCE:
        ret = JS_ThrowReferenceError(ctx_, format, args);
        break;
      case JsErrorType::TYPE:
        ret = JS_ThrowTypeError(ctx_, format, args);
        break;
      case JsErrorType::GENERIC:
      case JsErrorType::UNKNOWN:
        ret = JS_ThrowPlainError(ctx_, format, args);
        break;
    }
    va_end(args);
    return ret;
  }

  bool isObject(const JSValue& value) { return JS_IsObject(value); }
  bool isNull(const JSValue& value) { return JS_IsNull(value); }
  bool isUndefined(const JSValue& value) { return JS_IsUndefined(value); }
  bool isException(const JSValue& value) { return JS_IsException(value); }

  void logErrorStackTrace(const JSValue& exception) {
    JSValue actualException = JS_GetException(ctx_);
    auto message = toStdString(actualException);
    LOG(ERROR) << "[qjs] JS exception: " << message;

    JSValue stack = JS_GetPropertyStr(ctx_, actualException, "stack");
    auto stackTrace = toStdString(stack);
    if (stackTrace.empty()) {
      LOG(ERROR) << "[qjs] JS stack trace is null.";
    } else {
      LOG(ERROR) << "[qjs] JS stack trace: " << stackTrace;
    }

    JS_FreeValue(ctx_, stack);
    JS_FreeValue(ctx_, exception);
  }

  void freeValue(const JSValue& value) { JS_FreeValue(ctx_, value); }

  template <typename T_RIME_TYPE>
  void registerType(JsWrapper<T_RIME_TYPE, JSValue>& wrapper) {
    const char* key = typeid(T_RIME_TYPE).name();                           // N4rime6EngineE
    const char* typeName = JsWrapper<T_RIME_TYPE, JSValue>::getTypeName();  // Engine
    if (clazzes.find(key) != clazzes.end()) {
      LOG(INFO) << "type: " << typeName << " has already been registered.";
      return;
    }

    JSClassID classId = 0;
    JS_NewClassID(rt_, &classId);
    JSClassDef classDef = {
        .class_name = typeName,
        .finalizer = wrapper.getFinalizer(),
        .gc_mark = nullptr,
        .call = nullptr,
        .exotic = nullptr,
    };
    JS_NewClass(rt_, classId, &classDef);

    clazzes[key] = std::make_tuple(typeName, classId, classDef);

    JSValue proto = JS_NewObject(ctx_);
    JS_SetClassProto(ctx_, classId, proto);

    auto* constructor = wrapper.getConstructor();
    if (constructor) {
      JSValue jsConstructor = JS_NewCFunction2(
          ctx_, constructor, typeName, wrapper.getConstructorArgc(), JS_CFUNC_constructor, 0);
      JS_SetConstructor(ctx_, jsConstructor, proto);
      auto jsGlobal = JS_GetGlobalObject(ctx_);
      JS_SetPropertyStr(ctx_, jsGlobal, typeName, jsConstructor);
      JS_FreeValue(ctx_, jsGlobal);
    }

    JS_SetPropertyFunctionList(ctx_, proto, wrapper.getProperties(), wrapper.getPropertiesCount());
    JS_SetPropertyFunctionList(ctx_, proto, wrapper.getGetters(), wrapper.getGettersCount());
    JS_SetPropertyFunctionList(ctx_, proto, wrapper.getFunctions(), wrapper.getFunctionsCount());
  }

  typename TypeMap<JSValue>::ExposeFunctionType defineFunction(const char* name,
                                                               int avgc,
                                                               ExposeFunction func) {
    return JS_CFUNC_DEF(name, static_cast<uint8_t>(avgc), func);
  }

  typename TypeMap<JSValue>::ExposePropertyType defineProperty(
      const char* name,
      typename TypeMap<JSValue>::GetterFunctionType getter,
      typename TypeMap<JSValue>::SetterFunctionType setter) {
    return JS_CGETSET_DEF(name, getter, setter);
  }

  template <typename T>
  T* unwrap(const JSValue& value) {
    const char* key = typeid(T).name();
    if (clazzes.find(key) == clazzes.end()) {
      LOG(ERROR) << "type: " << key << " has not been registered.";
      return nullptr;
    }

    if (auto* ptr = JS_GetOpaque(value, getJsClassId<T>())) {
      return static_cast<T*>(ptr);
    }
    return nullptr;
  }

  template <typename T>
  std::shared_ptr<T> unwrapShared(const JSValue& value) {
    if (auto* ptr = JS_GetOpaque(value, getJsClassId<T>())) {
      if (auto sharedPtr = static_cast<std::shared_ptr<T>*>(ptr)) {
        return *sharedPtr;
      }
    }
    return nullptr;
  }

  template <typename T>
  JSValue wrap(T* ptrValue) {
    if (!ptrValue) {
      return JS_NULL;
    }

    const char* key = typeid(T).name();
    if (clazzes.find(key) == clazzes.end()) {
      LOG(ERROR) << "type: " << key << " has not been registered.";
      return JS_NULL;
    }
    JSValue jsobj = JS_NewObjectClass(ctx_, getJsClassId<T>());
    if (JS_IsException(jsobj)) {
      logErrorStackTrace(jsobj);
      return jsobj;
    }
    if (JS_SetOpaque(jsobj, ptrValue) < 0) {
      JS_FreeValue(ctx_, jsobj);
      const char* typeName = std::get<0>(clazzes[key]).c_str();
      return JS_ThrowInternalError(ctx_, "Failed to set a raw pointer to a %s object", typeName);
    };
    return jsobj;
  }

  template <typename T>
  JSValue wrapShared(const std::shared_ptr<T>& value) {
    if (!value) {
      return JS_NULL;
    }
    JSValue jsobj = JS_NewObjectClass(ctx_, getJsClassId<T>());
    if (JS_IsException(jsobj)) {
      return jsobj;
    }
    auto ptr = std::make_unique<std::shared_ptr<T>>(value);
    if (JS_SetOpaque(jsobj, ptr.release()) < 0) {
      JS_FreeValue(ctx_, jsobj);
      return JS_ThrowInternalError(ctx_, "Failed to set a raw pointer to a %s object",
                                   typeid(T).name());
    };
    return jsobj;
  }

  void setBaseFolderPath(const char* absolutePath) { setQjsBaseFolder(absolutePath); }

  JSValue loadJsFile(const char* fileName) {
    return QuickJSCodeLoader::loadJsModuleToNamespace(ctx_, fileName);
  }

private:
  template <typename T>
  static JSClassID getJsClassId() {
    const char* key = typeid(T).name();
    if (clazzes.find(key) == clazzes.end()) {
      LOG(ERROR) << "type: " << key << " has not been registered.";
      return 0;
    }
    return std::get<1>(clazzes.at(key));
  }

  JsEngine<JSValue>() : rt_(JS_NewRuntime()), ctx_(JS_NewContext(rt_)) {
    JS_SetModuleLoaderFunc(rt_, nullptr, js_module_loader, nullptr);

    // Do not trigger GC when heap size is less than 16MB
    // default: rt->malloc_gc_threshold = 256 * 1024
    constexpr size_t SIXTEEN_MEGABYTES = 16L * 1024 * 1024;
    JS_SetGCThreshold(rt_, SIXTEEN_MEGABYTES);

    QuickJSCodeLoader::exposeLogToJsConsole(ctx_);
  }

  ~JsEngine<JSValue>() {
    JS_FreeContext(ctx_);
    JS_FreeRuntime(rt_);
  }

  // Private non-static members
  JSRuntime* rt_{nullptr};
  JSContext* ctx_{nullptr};

  // Static member initialization
  inline static std::unordered_map<std::string, std::tuple<std::string, JSClassID, JSClassDef>>
      clazzes{};
};
