#pragma once

#include <glog/logging.h>
#include <quickjs.h>
#include <cstdint>
#include <memory>
#include <string>
#include <unordered_map>
#include <utility>

#include "engines/js_engine.h"
#include "engines/quickjs/quickjs_code_loader.h"
#include "engines/quickjs/quickjs_type_registry.h"
#include "engines/quickjs/quickjs_utils.h"
#include "patch/quickjs/node_module_loader.h"
#include "types/js_wrapper.h"

template <>
class JsEngine<JSValue> {
  // define them in a static way to avoid the initialization order fiasco
  static JSContext* newContext() {
    static auto* runtime = JS_NewRuntime();
    static auto* context = JS_NewContext(runtime);

    // Do not trigger GC when heap size is less than 16MB
    // default: rt->malloc_gc_threshold = 256 * 1024
    constexpr size_t SIXTEEN_MEGABYTES = 16L * 1024 * 1024;
    JS_SetGCThreshold(JS_GetRuntime(context), SIXTEEN_MEGABYTES);

    JS_SetModuleLoaderFunc(JS_GetRuntime(context), nullptr, js_module_loader, nullptr);
    QuickJSCodeLoader::exposeLogToJsConsole(context);

    return context;
  }

  // All qjs plugins should share the same runtime and context
  // Never call JS_FreeContext or JS_FreeRuntime to free them, otherwise the following error will occur:
  // Assertion failed: (p->ref_count > 0), function gc_decref_child, file quickjs.c, line 5868.
  inline static JSContext* context = newContext();

  JsEngine<JSValue>() {}

public:
  static JsEngine<JSValue>& instance() {
    static auto instance = JsEngine<JSValue>();
    return instance;
  }

  static JsEngine<JSValue>& getEngineByContext(JSContext* ctx) { return instance(); }

  [[nodiscard]] int64_t getMemoryUsage() const {
    JSMemoryUsage qjsMemStats;
    JS_ComputeMemoryUsage(JS_GetRuntime(context), &qjsMemStats);
    return qjsMemStats.memory_used_size;
  }

  template <typename T>
  [[nodiscard]] static void* getOpaque(JSValue value) {
    return JS_GetOpaque(value, QuickJSTypeRegistry::getJsClassId<T>());
  }

  static void setOpaque(JSValue value, void* opaque) { JS_SetOpaque(value, opaque); }

  [[nodiscard]] JSValue null() const { return JS_NULL; }
  [[nodiscard]] JSValue undefined() const { return JS_UNDEFINED; }

  [[nodiscard]] JSValue jsTrue() const { return JS_TRUE; }
  [[nodiscard]] JSValue jsFalse() const { return JS_FALSE; }

  [[nodiscard]] JSValue newArray() const { return JS_NewArray(context); }

  [[nodiscard]] size_t getArrayLength(const JSValue& array) const {
    auto lengthVal = JS_GetPropertyStr(context, array, "length");
    if (JS_IsException(lengthVal)) {
      return 0;
    }
    return toInt(lengthVal);
  }

  int insertItemToArray(JSValue array, size_t index, const JSValue& value) const {
    return JS_SetPropertyUint32(context, array, index, value);
  }

  [[nodiscard]] JSValue getArrayItem(const JSValue& array, size_t index) const {
    return JS_GetPropertyUint32(context, array, index);
  }

  [[nodiscard]] JSValue newObject() const { return JS_NewObject(context); }

  [[nodiscard]] JSValue getObjectProperty(const JSValue& obj, const char* propertyName) const {
    return JS_GetPropertyStr(context, obj, propertyName);
  }

  int setObjectProperty(JSValue obj, const char* propertyName, const JSValue& value) const {
    return JS_SetPropertyStr(context, obj, propertyName, value);
  }

  using ExposeFunction = JSCFunction*;
  int setObjectFunction(JSValue obj,
                        const char* functionName,
                        ExposeFunction cppFunction,
                        int expectingArgc) const {
    return JS_SetPropertyStr(context, obj, functionName,
                             JS_NewCFunction(context, cppFunction, functionName, expectingArgc));
  }

  [[nodiscard]] JSValue toObject(const JSValue& value) const { return value; }

  [[nodiscard]] JSValue toJsString(const char* str) const {
    return TypeConverter::toJsString(context, str);
  }
  [[nodiscard]] JSValue toJsString(const std::string& str) const {
    return TypeConverter::toJsString(context, str);
  }

  [[nodiscard]] std::string toStdString(const JSValue& value) const {
    return TypeConverter::toStdString(context, value);
  }

  [[nodiscard]] JSValue toJsBool(bool value) const { return JS_NewBool(context, value); }

  [[nodiscard]] bool toBool(const JSValue& value) const { return JS_ToBool(context, value); }

  [[nodiscard]] JSValue toJsInt(size_t value) const {
    return TypeConverter::toJsNumber(context, static_cast<int64_t>(value));
  }

  [[nodiscard]] size_t toInt(const JSValue& value) const {
    return TypeConverter::toUint32(context, value);
  }

  [[nodiscard]] JSValue toJsDouble(double value) const {
    return TypeConverter::toJsNumber(context, value);
  }

  [[nodiscard]] double toDouble(const JSValue& value) const {
    return TypeConverter::toDouble(context, value);
  }

  [[nodiscard]] JSValue callFunction(const JSValue& func,
                                     const JSValue& thisArg,
                                     int argc,
                                     JSValue* argv) const {
    return JS_Call(context, func, thisArg, argc, argv);
  }

  [[nodiscard]] JSValue newClassInstance(const JSValue& clazz, int argc, JSValue* argv) const {
    JSValue constructor = getMethodOfClassOrInstance(clazz, JS_UNDEFINED, "constructor");
    auto instance = JS_CallConstructor(context, constructor, argc, argv);
    JS_FreeValue(context, constructor);
    return instance;
  }

  [[nodiscard]] JSValue getJsClassHavingMethod(const JSValue& module,
                                               const char* methodName) const {
    return QuickJSCodeLoader::getExportedClassHavingMethodNameInModule(context, module, methodName);
  }

  [[nodiscard]] JSValue getMethodOfClassOrInstance(JSValue jsClass,
                                                   JSValue instance,
                                                   const char* methodName) const {
    return QuickJSCodeLoader::getMethodByNameInClass(context, jsClass, methodName);
  }

  [[nodiscard]] JSValue throwError(JsErrorType errorType, const char* format, ...) const {
    va_list args;
    va_start(args, format);
    JSValue ret = ErrorHandler::throwError(context, errorType, format, args);
    va_end(args);
    return ret;
  }

  [[nodiscard]] bool isObject(const JSValue& value) const { return JS_IsObject(value); }
  [[nodiscard]] bool isNull(const JSValue& value) const { return JS_IsNull(value); }
  [[nodiscard]] bool isUndefined(const JSValue& value) const { return JS_IsUndefined(value); }
  [[nodiscard]] bool isException(const JSValue& value) const { return JS_IsException(value); }
  [[nodiscard]] JSValue getLatestException() const { return JS_GetException(context); }

  void logErrorStackTrace(const JSValue& exception, const char* file, int line) const {
    ErrorHandler::logErrorStackTrace(context, exception, file, line);
  }

  template <typename... Args>
  void freeValue(const Args&... args) const {
    (JS_FreeValue(context, args), ...);
  }

  template <typename T_RIME_TYPE>
  void registerType(JsWrapper<T_RIME_TYPE, JSValue>& wrapper) {
    QuickJSTypeRegistry::registerType(context, wrapper);
  }

  template <typename T>
  [[nodiscard]] T* unwrap(const JSValue& value) const {
    if (auto* ptr = QuickJSTypeRegistry::getOpaqueIfTypeRegistered<T>(value)) {
      return static_cast<T*>(ptr);
    }
    return nullptr;
  }

  template <typename T>
  [[nodiscard]] std::shared_ptr<T> unwrapShared(const JSValue& value) const {
    if (auto* ptr = QuickJSTypeRegistry::getOpaqueIfTypeRegistered<T>(value)) {
      if (auto sharedPtr = static_cast<std::shared_ptr<T>*>(ptr)) {
        return *sharedPtr;
      }
    }
    return nullptr;
  }

  template <typename T>
  [[nodiscard]] JSValue wrap(T* ptrValue) const {
    JSValue jsobj = QuickJSTypeRegistry::createJsObjectForType<T>(context, ptrValue != nullptr);
    if (JS_IsNull(jsobj) || JS_IsException(jsobj)) {
      return jsobj;
    }
    if (JS_SetOpaque(jsobj, ptrValue) < 0) {
      JS_FreeValue(context, jsobj);
      return JS_ThrowInternalError(context, "Failed to set a raw pointer to a %s object",
                                   QuickJSTypeRegistry::getRegisteredTypeName<T>().c_str());
    }
    return jsobj;
  }

  template <typename T>
  [[nodiscard]] JSValue wrapShared(const std::shared_ptr<T>& value) const {
    JSValue jsobj = QuickJSTypeRegistry::createJsObjectForType<T>(context, value != nullptr);
    if (JS_IsNull(jsobj) || JS_IsException(jsobj)) {
      return jsobj;
    }
    auto ptr = std::make_unique<std::shared_ptr<T>>(value);
    if (JS_SetOpaque(jsobj, ptr.release()) < 0) {
      JS_FreeValue(context, jsobj);
      return JS_ThrowInternalError(context, "Failed to set a shared pointer to a %s object",
                                   QuickJSTypeRegistry::getRegisteredTypeName<T>().c_str());
    }
    return jsobj;
  }

  [[nodiscard]] typename TypeMap<JSValue>::ExposeFunctionType
  defineFunction(const char* name, int avgc, ExposeFunction func) const {
    return JS_CFUNC_DEF(name, static_cast<uint8_t>(avgc), func);
  }

  [[nodiscard]] typename TypeMap<JSValue>::ExposePropertyType defineProperty(
      const char* name,
      typename TypeMap<JSValue>::GetterFunctionType getter,
      typename TypeMap<JSValue>::SetterFunctionType setter) const {
    return JS_CGETSET_DEF(name, getter, setter);
  }

  void setBaseFolderPath(const char* absolutePath) { setQjsBaseFolder(absolutePath); }

  [[nodiscard]] JSValue loadJsFile(const char* fileName) const {
    return QuickJSCodeLoader::loadJsModuleToNamespace(context, fileName);
  }

  [[nodiscard]] JSValue eval(const char* code, const char* filename = "<eval>") const {
    return JS_Eval(context, code, strlen(code), filename, JS_EVAL_TYPE_GLOBAL);
  }

  [[nodiscard]] JSValue getGlobalObject() const { return JS_GetGlobalObject(context); }
};
