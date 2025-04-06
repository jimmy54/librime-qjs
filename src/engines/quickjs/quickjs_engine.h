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
#include "patch/quickjs/node_module_loader.h"
#include "types/js_wrapper.h"

template <>
class JsEngine<JSValue> {
  // All qjs plugins should share the same runtime and context
  // Never call JS_FreeContext or JS_FreeRuntime to free them, otherwise the following error will occur:
  // Assertion failed: (p->ref_count > 0), function gc_decref_child, file quickjs.c, line 5868.
  inline static JSRuntime* runtime = JS_NewRuntime();
  inline static JSContext* context = JS_NewContext(runtime);

  // the types will be registered only once to the shared runtime
  // the key is the typeid of the registering type: typeid(T_RIME_TYPE).name()
  inline static std::unordered_map<std::string, std::pair<std::string, JSClassID>> clazzes{};

  JsEngine<JSValue>() {
    JS_SetModuleLoaderFunc(runtime, nullptr, js_module_loader, nullptr);

    // Do not trigger GC when heap size is less than 16MB
    // default: rt->malloc_gc_threshold = 256 * 1024
    constexpr size_t SIXTEEN_MEGABYTES = 16L * 1024 * 1024;
    JS_SetGCThreshold(runtime, SIXTEEN_MEGABYTES);

    QuickJSCodeLoader::exposeLogToJsConsole(context);
  }

public:
  static JsEngine<JSValue>& instance() {
    static auto instance = JsEngine<JSValue>();
    return instance;
  }

  ~JsEngine<JSValue>() { JS_FreeContext(context); }

  JsEngine(const JsEngine& other) = default;
  JsEngine(JsEngine&&) = delete;
  JsEngine& operator=(const JsEngine& other) = delete;
  JsEngine& operator=(JsEngine&&) = delete;

  static JsEngine<JSValue>& getEngineByContext(JSContext* ctx) { return instance(); }

  [[nodiscard]] int64_t getMemoryUsage() const {
    JSMemoryUsage qjsMemStats;
    JS_ComputeMemoryUsage(runtime, &qjsMemStats);
    return qjsMemStats.memory_used_size;
  }

  template <typename T>
  [[nodiscard]] static void* getOpaque(JSValue value) {
    auto classId = getJsClassId<T>();
    return JS_GetOpaque(value, classId);
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

    uint32_t length = 0;
    JS_ToUint32(context, &length, lengthVal);
    JS_FreeValue(context, lengthVal);

    return length;
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

  [[nodiscard]] JSValue toJsString(const char* str) const { return JS_NewString(context, str); }
  [[nodiscard]] JSValue toJsString(const std::string& str) const {
    return JS_NewString(context, str.c_str());
  }

  [[nodiscard]] std::string toStdString(const JSValue& value) const {
    const char* str = JS_ToCString(context, value);
    std::string ret(str);
    JS_FreeCString(context, str);
    return ret;
  }

  [[nodiscard]] JSValue toJsBool(bool value) const { return JS_NewBool(context, value); }

  [[nodiscard]] bool toBool(const JSValue& value) const { return JS_ToBool(context, value); }

  [[nodiscard]] JSValue toJsInt(size_t value) const {
    return JS_NewInt64(context, static_cast<int64_t>(value));
  }

  [[nodiscard]] size_t toInt(const JSValue& value) const {
    uint32_t ret = 0;
    JS_ToUint32(context, &ret, value);
    return ret;
  }

  [[nodiscard]] JSValue toJsDouble(double value) const { return JS_NewFloat64(context, value); }

  [[nodiscard]] double toDouble(const JSValue& value) const {
    double ret = 0;
    JS_ToFloat64(context, &ret, value);
    return ret;
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
    JSValue ret;
    va_list args;
    va_start(args, format);
    switch (errorType) {
      case JsErrorType::SYNTAX:
      case JsErrorType::EVAL:
        ret = JS_ThrowSyntaxError(context, format, args);
        break;
      case JsErrorType::RANGE:
        ret = JS_ThrowRangeError(context, format, args);
        break;
      case JsErrorType::REFERENCE:
        ret = JS_ThrowReferenceError(context, format, args);
        break;
      case JsErrorType::TYPE:
        ret = JS_ThrowTypeError(context, format, args);
        break;
      case JsErrorType::GENERIC:
      case JsErrorType::UNKNOWN:
        ret = JS_ThrowPlainError(context, format, args);
        break;
    }
    va_end(args);
    return ret;
  }

  [[nodiscard]] bool isObject(const JSValue& value) const { return JS_IsObject(value); }
  [[nodiscard]] bool isNull(const JSValue& value) const { return JS_IsNull(value); }
  [[nodiscard]] bool isUndefined(const JSValue& value) const { return JS_IsUndefined(value); }
  [[nodiscard]] bool isException(const JSValue& value) const { return JS_IsException(value); }
  [[nodiscard]] JSValue getLatestException() const { return JS_GetException(context); }

  void logErrorStackTrace(const JSValue& exception,
                          const char* file = __FILE_NAME__,
                          int line = __LINE__) const {
    JSValue actualException = JS_GetException(context);
    auto message = toStdString(actualException);
    LOG(ERROR) << "[qjs] JS exception at " << file << ':' << line << " => " << message;

    JSValue stack = JS_GetPropertyStr(context, actualException, "stack");
    auto stackTrace = toStdString(stack);
    if (stackTrace.empty()) {
      LOG(ERROR) << "[qjs] JS stack trace is null.";
    } else {
      LOG(ERROR) << "[qjs] JS stack trace: " << stackTrace;
    }

    freeValue(stack, exception);
  }

  void freeValue(const JSValue& value) const { JS_FreeValue(context, value); }

  template <typename T, typename... Args>
  void freeValue(const T& first, const Args&... rest) const {
    freeValue(first);
    freeValue(rest...);
  }

  void freeValue() const {}

  template <typename T_RIME_TYPE>
  void registerType(JsWrapper<T_RIME_TYPE, JSValue>& wrapper) {
    const char* key = typeid(T_RIME_TYPE).name();                           // N4rime6EngineE
    const char* typeName = JsWrapper<T_RIME_TYPE, JSValue>::getTypeName();  // Engine
    if (clazzes.find(key) != clazzes.end()) {
      DLOG(INFO) << "type: " << typeName
                 << " has already been registered with classId = " << getJsClassId<T_RIME_TYPE>();
      return;
    }

    JSClassID classId = JsWrapper<T_RIME_TYPE, JSValue>::getJSClassId();
    JSClassDef classDef = JsWrapper<T_RIME_TYPE, JSValue>::getJSClassDef();
    classDef.finalizer = wrapper.getFinalizer();

    JS_NewClassID(runtime, &classId);
    JS_NewClass(runtime, classId, &classDef);

    clazzes[key] = std::make_pair(typeName, classId);

    JSValue proto = JS_NewObject(context);
    if (auto* constructor = wrapper.getConstructor()) {
      JSValue jsConstructor = JS_NewCFunction2(
          context, constructor, typeName, wrapper.getConstructorArgc(), JS_CFUNC_constructor, 0);
      JS_SetConstructor(context, jsConstructor, proto);
      auto jsGlobal = JS_GetGlobalObject(context);
      JS_SetPropertyStr(context, jsGlobal, typeName, jsConstructor);
      JS_FreeValue(context, jsGlobal);
    }

    JS_SetPropertyFunctionList(context, proto, wrapper.getProperties(context),
                               wrapper.getPropertiesCount());
    JS_SetPropertyFunctionList(context, proto, wrapper.getGetters(context),
                               wrapper.getGettersCount());
    JS_SetPropertyFunctionList(context, proto, wrapper.getFunctions(context),
                               wrapper.getFunctionsCount());
    JS_SetClassProto(context, classId, proto);
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

  template <typename T>
  [[nodiscard]] T* unwrap(const JSValue& value) const {
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
  [[nodiscard]] std::shared_ptr<T> unwrapShared(const JSValue& value) const {
    if (auto* ptr = JS_GetOpaque(value, getJsClassId<T>())) {
      if (auto sharedPtr = static_cast<std::shared_ptr<T>*>(ptr)) {
        return *sharedPtr;
      }
    }
    return nullptr;
  }

  template <typename T>
  [[nodiscard]] JSValue wrap(T* ptrValue) const {
    if (!ptrValue) {
      return JS_NULL;
    }

    const char* key = typeid(T).name();
    if (clazzes.find(key) == clazzes.end()) {
      LOG(ERROR) << "type: " << key << " has not been registered.";
      return JS_NULL;
    }
    JSValue jsobj = JS_NewObjectClass(context, getJsClassId<T>());
    if (isUndefined(jsobj)) {
      LOG(ERROR) << "Failed to create a new object with classId = " << getJsClassId<T>();
      return jsobj;
    }
    if (JS_IsException(jsobj)) {
      logErrorStackTrace(jsobj);
      return jsobj;
    }
    if (JS_SetOpaque(jsobj, ptrValue) < 0) {
      JS_FreeValue(context, jsobj);
      const char* typeName = clazzes[key].first.c_str();
      return JS_ThrowInternalError(context, "Failed to set a raw pointer to a %s object", typeName);
    };
    return jsobj;
  }

  template <typename T>
  [[nodiscard]] JSValue wrapShared(const std::shared_ptr<T>& value) const {
    if (!value) {
      return JS_NULL;
    }
    JSValue jsobj = JS_NewObjectClass(context, getJsClassId<T>());
    if (JS_IsException(jsobj)) {
      return jsobj;
    }
    auto ptr = std::make_unique<std::shared_ptr<T>>(value);
    if (JS_SetOpaque(jsobj, ptr.release()) < 0) {
      JS_FreeValue(context, jsobj);
      return JS_ThrowInternalError(context, "Failed to set a raw pointer to a %s object",
                                   typeid(T).name());
    };
    return jsobj;
  }

  void setBaseFolderPath(const char* absolutePath) { setQjsBaseFolder(absolutePath); }

  [[nodiscard]] JSValue loadJsFile(const char* fileName) const {
    return QuickJSCodeLoader::loadJsModuleToNamespace(context, fileName);
  }

  [[nodiscard]] JSValue eval(const char* code, const char* filename = "<eval>") const {
    return JS_Eval(context, code, strlen(code), filename, JS_EVAL_TYPE_GLOBAL);
  }

  [[nodiscard]] JSValue getGlobalObject() const { return JS_GetGlobalObject(context); }

private:
  template <typename T>
  static JSClassID getJsClassId() {
    const char* key = typeid(T).name();
    if (clazzes.find(key) == clazzes.end()) {
      LOG(ERROR) << "type: " << key << " has not been registered.";
      return 0;
    }
    return clazzes.at(key).second;
  }
};
