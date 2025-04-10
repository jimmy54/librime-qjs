#pragma once

#include <glog/logging.h>
#include <quickjs.h>
#include <string>
#include <unordered_map>

#include "engines/quickjs/quickjs_utils.h"
#include "types/js_wrapper.h"

class QuickJSTypeRegistry {
public:
  template <typename T_RIME_TYPE>
  static void registerType(JSContext* context) {
    using WRAPPER = JsWrapper<T_RIME_TYPE, JSValue>;

    auto* typeName = WRAPPER::TYPENAME;
    JSClassID& classId = WRAPPER::JS_CLASS_ID;

    if (registeredTypes.count(typeName) > 0) {
      DLOG(INFO) << "type [" << typeName
                 << "] has already been registered with classId = " << classId;
      return;
    }

    JSClassDef& classDef = WRAPPER::JS_CLASS_DEF;
    if (auto& finalizer = WRAPPER::finalizerQjs) {
      classDef.finalizer = finalizer;
    }
    JS_NewClassID(JS_GetRuntime(context), &classId);
    JS_NewClass(JS_GetRuntime(context), classId, &classDef);

    registeredTypes[typeName] = classId;

    JSValue proto = JS_NewObject(context);
    if (auto* constructor = WRAPPER::constructorQjs) {
      int argc = WRAPPER::CONSTRUCTOR_ARGC;
      JSValue jsConstructor =
          JS_NewCFunction2(context, constructor, typeName, argc, JS_CFUNC_constructor, 0);
      JS_SetConstructor(context, jsConstructor, proto);
      auto jsGlobal = JS_GetGlobalObject(context);
      JS_SetPropertyStr(context, jsGlobal, typeName, jsConstructor);
      JS_FreeValue(context, jsGlobal);
    }

    if (auto& properties = WRAPPER::PROPERTIES_QJS) {
      auto size = WRAPPER::PROPERTIES_SIZE;
      JS_SetPropertyFunctionList(context, proto, properties, size);
    }
    if (auto& getters = WRAPPER::GETTERS_QJS) {
      auto size = WRAPPER::GETTERS_SIZE;
      JS_SetPropertyFunctionList(context, proto, getters, size);
    }
    if (auto& functions = WRAPPER::FUNCTIONS_QJS) {
      auto size = WRAPPER::FUNCTIONS_SIZE;
      JS_SetPropertyFunctionList(context, proto, functions, size);
    }

    JS_SetClassProto(context, classId, proto);
    DLOG(INFO) << "registered type [" << typeName << "] with classId = " << classId;
  }

  template <typename T>
  static JSValue createJsObjectForType(JSContext* context, bool hasValue) {
    if (!hasValue) {
      return JS_NULL;
    }
    JSValue jsobj = JS_NewObjectClass(context, JsWrapper<T, JSValue>::JS_CLASS_ID);
    if (JS_IsUndefined(jsobj)) {
      LOG(ERROR) << "Failed to create a new object of type " << JsWrapper<T, JSValue>::TYPENAME
                 << " with classId = " << JsWrapper<T, JSValue>::JS_CLASS_ID;
      return jsobj;
    }
    if (JS_IsException(jsobj)) {
      ErrorHandler::logErrorStackTrace(context, jsobj, __FILE_NAME__, __LINE__);
    }
    return jsobj;
  }

private:
  inline static std::unordered_map<std::string, JSClassID> registeredTypes{};
};
