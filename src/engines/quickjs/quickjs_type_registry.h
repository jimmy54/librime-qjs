#pragma once

#include <glog/logging.h>
#include <quickjs.h>
#include <string>
#include <unordered_map>
#include <utility>

#include "engines/quickjs/quickjs_utils.h"
#include "types/js_wrapper.h"

class QuickJSTypeRegistry {
public:
  template <typename T_RIME_TYPE>
  static void registerType(JSContext* context, JsWrapper<T_RIME_TYPE, JSValue>& wrapper) {
    const char* key = typeid(T_RIME_TYPE).name();                           // N4rime6EngineE
    const char* typeName = JsWrapper<T_RIME_TYPE, JSValue>::getTypeName();  // Engine
    if (isTypeRegistered<T_RIME_TYPE>(key, false)) {
      DLOG(INFO) << "type: " << typeName
                 << " has already been registered with classId = " << getJsClassId<T_RIME_TYPE>();
      return;
    }

    JSClassID classId = JsWrapper<T_RIME_TYPE, JSValue>::getJSClassId();
    JSClassDef classDef = JsWrapper<T_RIME_TYPE, JSValue>::getJSClassDef();
    classDef.finalizer = wrapper.getFinalizer();

    JS_NewClassID(JS_GetRuntime(context), &classId);
    JS_NewClass(JS_GetRuntime(context), classId, &classDef);

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

  template <typename T>
  static JSClassID getJsClassId() {
    const char* key = typeid(T).name();
    return isTypeRegistered<T>(key) ? clazzes.at(key).second : 0;
  }

  template <typename T>
  static std::string getRegisteredTypeName() {
    const char* key = typeid(T).name();
    return isTypeRegistered<T>(key) ? clazzes.at(key).first : key;
  }

  template <typename T>
  static void* getOpaqueIfTypeRegistered(const JSValue& value) {
    if (!isTypeRegistered<T>()) {
      return nullptr;
    }
    return JS_GetOpaque(value, getJsClassId<T>());
  }

  template <typename T>
  static JSValue createJsObjectForType(JSContext* context, bool hasValue) {
    if (!hasValue) {
      return JS_NULL;
    }
    const char* key = typeid(T).name();
    if (!isTypeRegistered<T>(key)) {
      return JS_NULL;
    }
    JSValue jsobj = JS_NewObjectClass(context, getJsClassId<T>());
    if (JS_IsUndefined(jsobj)) {
      LOG(ERROR) << "Failed to create a new object with classId = " << getJsClassId<T>();
      return jsobj;
    }
    if (JS_IsException(jsobj)) {
      ErrorHandler::logErrorStackTrace(context, jsobj, __FILE_NAME__, __LINE__);
    }
    return jsobj;
  }

private:
  template <typename T>
  static bool isTypeRegistered(const char* key = nullptr, bool isLog = true) {
    const char* typeKey = key ? key : typeid(T).name();
    auto isRegistered = clazzes.find(typeKey) != clazzes.end();
    if (isLog && !isRegistered) {
      LOG(ERROR) << "type: " << typeKey << " has not been registered.";
    }
    return isRegistered;
  }

  // the types will be registered only once to the shared runtime
  // the key is the typeid of the registering type: typeid(T_RIME_TYPE).name()
  inline static std::unordered_map<std::string, std::pair<std::string, JSClassID>> clazzes{};
};
