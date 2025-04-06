#pragma once

#include <quickjs.h>

#include "engines/engine_manager.h"  // IWYU pragma: export
#include "engines/for_each_macros.h"
#include "engines/quickjs/quickjs_engine.h"  // IWYU pragma: export

#ifdef __APPLE__
#include "engines/javascriptcore/jsc_macros.h"
#else
#define DEFINE_GETTER_IMPL(T_RIME_TYPE, propertieyName, statement, unwrap) \
  DEFINE_GETTER_IMPL_QJS(T_RIME_TYPE, propertieyName, statement, unwrap)

#define DEFINE_STRING_SETTER_IMPL(T_RIME_TYPE, name, assignment, unwrap) \
  DEFINE_STRING_SETTER_IMPL_QJS(T_RIME_TYPE, name, assignment, unwrap)

#define DEFINE_SETTER_IMPL(T_RIME_TYPE, jsName, converter, assignment, unwrap) \
  DEFINE_SETTER_IMPL_QJS(T_RIME_TYPE, jsName, converter, assignment, unwrap)

#define DEFINE_CFUNCTION(funcName, funcBody) DEFINE_CFUNCTION_QJS(funcName, funcBody)

#define DEFINE_CFUNCTION_ARGC(funcName, expectingArgc, statements) \
  DEFINE_CFUNCTION_ARGC_QJS(funcName, expectingArgc, statements)

#define EXPORT_CONSTRUCTOR(funcName, funcBody) EXPORT_CONSTRUCTOR_QJS(funcName, funcBody)
#define EXPORT_FINALIZER(T_RIME_TYPE, funcName) EXPORT_FINALIZER_QJS(T_RIME_TYPE, funcName)
#define EXPORT_PROPERTIES(...) EXPORT_PROPERTIES_QJS(__VA_ARGS__)
#define EXPORT_FUNCTIONS(...) EXPORT_FUNCTIONS_QJS(__VA_ARGS__)
#define EXPORT_GETTERS(...) EXPORT_GETTER_QJS(__VA_ARGS__)
#endif

#define countof(array) (sizeof(array) / sizeof(array[0]))

#define EXPORT_CLASS(name)            \
  static const char* getTypeName() {  \
    return #name;                     \
  }                                   \
  static JSClassID getJSClassId() {   \
    static JSClassID classId = 0;     \
    return classId;                   \
  }                                   \
  static JSClassDef getJSClassDef() { \
    static JSClassDef classDef = {    \
        .class_name = #name,          \
        .finalizer = nullptr,         \
        .gc_mark = nullptr,           \
        .call = nullptr,              \
        .exotic = nullptr,            \
    };                                \
    return classDef;                  \
  }

// =============== GETTER ===============
#define DEFINE_GETTER(T_RIME_TYPE, propertieyName, statement) \
  DEFINE_GETTER_IMPL(T_RIME_TYPE, propertieyName, statement, engine.unwrap<T_RIME_TYPE>(thisVal))

#define DEFINE_GETTER_2(T_RIME_TYPE, propertieyName, statement) \
  DEFINE_GETTER_IMPL(T_RIME_TYPE, propertieyName, statement,    \
                     engine.unwrapShared<T_RIME_TYPE>(thisVal))

#define DEFINE_GETTER_IMPL_QJS(T_RIME_TYPE, propertieyName, statement, unwrap) \
  static JSValue get_##propertieyName(JSContext* ctx, JSValueConst thisVal) {  \
    auto& engine = JsEngine<JSValue>::getEngineByContext(ctx);                 \
    if (auto obj = unwrap) {                                                   \
      return statement;                                                        \
    }                                                                          \
    return JS_UNDEFINED;                                                       \
  }

// =============== SETTER ===============

#define DEFINE_STRING_SETTER(T_RIME_TYPE, name, assignment) \
  DEFINE_STRING_SETTER_IMPL(T_RIME_TYPE, name, assignment, engine.unwrap<T_RIME_TYPE>(thisVal))

#define DEFINE_STRING_SETTER_2(T_RIME_TYPE, name, assignment) \
  DEFINE_STRING_SETTER_IMPL(T_RIME_TYPE, name, assignment,    \
                            engine.unwrapShared<T_RIME_TYPE>(thisVal))

#define DEFINE_STRING_SETTER_IMPL_QJS(T_RIME_TYPE, name, assignment, unwrap)                     \
  static JSValue set_##name(JSContext* ctx, JSValueConst thisVal, JSValue val) {                 \
    auto& engine = JsEngine<JSValue>::getEngineByContext(ctx);                                   \
    if (auto obj = unwrap) {                                                                     \
      auto str = engine.toStdString(val);                                                        \
      if (!str.empty()) {                                                                        \
        assignment;                                                                              \
        return JS_UNDEFINED;                                                                     \
      }                                                                                          \
      return engine.throwError(JsErrorType::TYPE, " %s.%s = rvalue: rvalue is not a string",     \
                               #T_RIME_TYPE, #name);                                             \
    }                                                                                            \
    return engine.throwError(JsErrorType::TYPE,                                                  \
                             "Failed to unwrap the js object to a cpp %s object", #T_RIME_TYPE); \
  }

#define DEFINE_SETTER(T_RIME_TYPE, jsName, converter, assignment) \
  DEFINE_SETTER_IMPL(T_RIME_TYPE, jsName, converter, assignment,  \
                     engine.unwrap<T_RIME_TYPE>(thisVal))

#define DEFINE_SETTER_2(T_RIME_TYPE, jsName, converter, assignment) \
  DEFINE_SETTER_IMPL(T_RIME_TYPE, jsName, converter, assignment,    \
                     engine.unwrapShared<T_RIME_TYPE>(thisVal))

#define DEFINE_SETTER_IMPL_QJS(T_RIME_TYPE, jsName, converter, assignment, unwrap)     \
  static JSValue set_##jsName(JSContext* ctx, JSValueConst thisVal, JSValue val) {     \
    auto& engine = JsEngine<JSValue>::getEngineByContext(ctx);                         \
    if (auto obj = unwrap) {                                                           \
      auto value = converter(val);                                                     \
      assignment;                                                                      \
      return JS_UNDEFINED;                                                             \
    }                                                                                  \
    return JS_ThrowTypeError(ctx, "Failed to unwrap the js object to a cpp %s object", \
                             #T_RIME_TYPE);                                            \
  }

// =============== FUNCTION ===============

#define DEFINE_CFUNCTION_QJS(funcName, funcBody)                                      \
  static JSValue funcName(JSContext* ctx, JSValue thisVal, int argc, JSValue* argv) { \
    auto& engine = JsEngine<JSValue>::getEngineByContext(ctx);                        \
    try {                                                                             \
      funcBody;                                                                       \
    } catch (const JsException& e) {                                                  \
      return engine.throwError(JsErrorType::TYPE, e.what());                          \
    }                                                                                 \
  }

#define DEFINE_CFUNCTION_ARGC_QJS(funcName, expectingArgc, statements)                           \
  static JSValue funcName(JSContext* ctx, JSValueConst thisVal, int argc, JSValueConst* argv) {  \
    if (argc < expectingArgc) {                                                                  \
      return JS_ThrowSyntaxError(ctx, "%s(...) expects %d arguments", #funcName, expectingArgc); \
    }                                                                                            \
    auto& engine = JsEngine<JSValue>::getEngineByContext(ctx);                                   \
    try {                                                                                        \
      statements;                                                                                \
    } catch (const JsException& e) {                                                             \
      return engine.throwError(JsErrorType::TYPE, e.what());                                     \
    }                                                                                            \
  }

#define EXPORT_CONSTRUCTOR_QJS(funcName, funcBody)                           \
                                                                             \
  DEFINE_CFUNCTION_QJS(funcName, funcBody)                                   \
                                                                             \
  typename TypeMap<JSValue>::FunctionPionterType getConstructor() override { \
    return funcName;                                                         \
  }

#define EXPORT_FINALIZER_QJS(T_RIME_TYPE, funcName)                                 \
  typename TypeMap<JSValue>::FinalizerFunctionPionterType getFinalizer() override { \
    return funcName;                                                                \
  }                                                                                 \
                                                                                    \
  static void funcName(typename TypeMap<JSValue>::RuntimeType* rt, JSValue val) {   \
    if (void* ptr = JsEngine<JSValue>::getOpaque<T_RIME_TYPE>(val)) {               \
      auto* ppObj = static_cast<std::shared_ptr<T_RIME_TYPE>*>(ptr);                \
      ppObj->reset();                                                               \
      delete ppObj;                                                                 \
      JsEngine<JSValue>::setOpaque(val, nullptr);                                   \
    }                                                                               \
  }

#define DEFINE_PROPERTY(name) engine.defineProperty(#name, get_##name, set_##name),

#define EXPORT_PROPERTIES_QJS(...)                                                        \
  typename TypeMap<JSValue>::ExposePropertyType* getProperties(JSContext* ctx) override { \
    auto& engine = JsEngine<JSValue>::getEngineByContext(ctx);                            \
    static typename TypeMap<JSValue>::ExposePropertyType properties[] = {                 \
        FOR_EACH(DEFINE_PROPERTY, __VA_ARGS__)};                                          \
                                                                                          \
    this->setPropertyCount(countof(properties));                                          \
                                                                                          \
    return static_cast<TypeMap<JSValue>::ExposePropertyType*>(properties);                \
  }

#define DEFINE_GETTER_QJS(name) engine.defineProperty(#name, get_##name, nullptr),

#define EXPORT_GETTER_QJS(...)                                                         \
  typename TypeMap<JSValue>::ExposePropertyType* getGetters(JSContext* ctx) override { \
    auto& engine = JsEngine<JSValue>::getEngineByContext(ctx);                         \
    static typename TypeMap<JSValue>::ExposePropertyType getters[] = {                 \
        FOR_EACH(DEFINE_GETTER_QJS, __VA_ARGS__)};                                     \
                                                                                       \
    this->setGetterCount(countof(getters));                                            \
                                                                                       \
    return static_cast<TypeMap<JSValue>::ExposePropertyType*>(getters);                \
  }

#define DEFINE_FUNCTION_QJS(name, argc) engine.defineFunction(#name, argc, name),

#define EXPORT_FUNCTIONS_QJS(...)                                                        \
  typename TypeMap<JSValue>::ExposeFunctionType* getFunctions(JSContext* ctx) override { \
    auto& engine = JsEngine<JSValue>::getEngineByContext(ctx);                           \
    static typename TypeMap<JSValue>::ExposeFunctionType functions[] = {                 \
        FOR_EACH_PAIR(DEFINE_FUNCTION_QJS, __VA_ARGS__)};                                \
    this->setFunctionCount(countof(functions));                                          \
    return static_cast<TypeMap<JSValue>::ExposeFunctionType*>(functions);                \
  }
