#pragma once

#include <quickjs.h>

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
#define EXPORT_FINALIZER(funcName, funcBody) EXPORT_FINALIZER_QJS(funcName, funcBody)
#define EXPORT_PROPERTIES(...) EXPORT_PROPERTIES_QJS(__VA_ARGS__)
#endif

#define countof(array) (sizeof(array) / sizeof(array[0]))

// =============== GETTER ===============
#define DEFINE_GETTER(T_RIME_TYPE, propertieyName, statement) \
  DEFINE_GETTER_IMPL(T_RIME_TYPE, propertieyName, statement, engine.unwrap<T_RIME_TYPE>(thisVal))

#define DEFINE_GETTER_2(T_RIME_TYPE, propertieyName, statement) \
  DEFINE_GETTER_IMPL(T_RIME_TYPE, propertieyName, statement,    \
                     engine.unwrapShared<T_RIME_TYPE>(thisVal))

#define DEFINE_GETTER_IMPL_QJS(T_RIME_TYPE, propertieyName, statement, unwrap) \
  static JSValue get_##propertieyName(JSContext* ctx, JSValueConst thisVal) {  \
    auto& engine = getJsEngine<JSValue>();                                     \
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
    auto& engine = getJsEngine<JSValue>();                                                       \
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
    auto& engine = getJsEngine<JSValue>();                                             \
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
    auto& engine = getJsEngine<JSValue>();                                            \
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
    auto& engine = getJsEngine<JSValue>();                                                       \
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

#define EXPORT_FINALIZER_QJS(funcName, funcBody)                                    \
  typename TypeMap<JSValue>::FinalizerFunctionPionterType getFinalizer() override { \
    return funcName;                                                                \
  }                                                                                 \
                                                                                    \
  static void funcName(typename TypeMap<JSValue>::RuntimeType* rt, JSValue val) {   \
    auto& engine = getJsEngine<JSValue>();                                          \
    funcBody;                                                                       \
  }

#define DEFINE_PROPERTY(name) engine.defineProperty(#name, get_##name, set_##name),

#define EXPORT_PROPERTIES_QJS(...)                                          \
  typename TypeMap<JSValue>::ExposePropertyType* getProperties() override { \
    auto& engine = getJsEngine<JSValue>();                                  \
    static typename TypeMap<JSValue>::ExposePropertyType properties[] = {   \
        FOR_EACH(DEFINE_PROPERTY, __VA_ARGS__)};                            \
                                                                            \
    this->setPropertyCount(countof(properties));                            \
                                                                            \
    return properties;                                                      \
  }

// =============== FOR_EACH ===============
#define EXPAND(...) __VA_ARGS__
#define FOR_EACH_1(macro, x) macro(x)
#define FOR_EACH_2(macro, x, ...) macro(x) EXPAND(FOR_EACH_1(macro, __VA_ARGS__))
#define FOR_EACH_3(macro, x, ...) macro(x) EXPAND(FOR_EACH_2(macro, __VA_ARGS__))
#define FOR_EACH_4(macro, x, ...) macro(x) EXPAND(FOR_EACH_3(macro, __VA_ARGS__))
#define FOR_EACH_5(macro, x, ...) macro(x) EXPAND(FOR_EACH_4(macro, __VA_ARGS__))
#define FOR_EACH_6(macro, x, ...) macro(x) EXPAND(FOR_EACH_5(macro, __VA_ARGS__))
#define FOR_EACH_7(macro, x, ...) macro(x) EXPAND(FOR_EACH_6(macro, __VA_ARGS__))
#define FOR_EACH_8(macro, x, ...) macro(x) EXPAND(FOR_EACH_7(macro, __VA_ARGS__))
#define FOR_EACH_9(macro, x, ...) macro(x) EXPAND(FOR_EACH_8(macro, __VA_ARGS__))
#define FOR_EACH_10(macro, x, ...) macro(x) EXPAND(FOR_EACH_9(macro, __VA_ARGS__))
#define FOR_EACH_11(macro, x, ...) macro(x) EXPAND(FOR_EACH_10(macro, __VA_ARGS__))
#define FOR_EACH_12(macro, x, ...) macro(x) EXPAND(FOR_EACH_11(macro, __VA_ARGS__))
#define FOR_EACH_13(macro, x, ...) macro(x) EXPAND(FOR_EACH_12(macro, __VA_ARGS__))
#define FOR_EACH_14(macro, x, ...) macro(x) EXPAND(FOR_EACH_13(macro, __VA_ARGS__))
#define FOR_EACH_15(macro, x, ...) macro(x) EXPAND(FOR_EACH_14(macro, __VA_ARGS__))
#define FOR_EACH_16(macro, x, ...) macro(x) EXPAND(FOR_EACH_15(macro, __VA_ARGS__))
#define FOR_EACH_17(macro, x, ...) macro(x) EXPAND(FOR_EACH_16(macro, __VA_ARGS__))
#define FOR_EACH_18(macro, x, ...) macro(x) EXPAND(FOR_EACH_17(macro, __VA_ARGS__))
#define FOR_EACH_19(macro, x, ...) macro(x) EXPAND(FOR_EACH_18(macro, __VA_ARGS__))
#define FOR_EACH_20(macro, x, ...) macro(x) EXPAND(FOR_EACH_19(macro, __VA_ARGS__))

// Get number of arguments
#define COUNT_ARGS_(_1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, \
                    _18, _19, _20, N, ...)                                                      \
  N
#define COUNT_ARGS(...) \
  COUNT_ARGS_(__VA_ARGS__, 20, 19, 18, 17, 16, 15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1)

// Select the appropriate FOR_EACH macro based on argument count
#define _FOR_EACH_N(N, macro, ...) FOR_EACH_##N(macro, __VA_ARGS__)
#define FOR_EACH_N(N, macro, ...) _FOR_EACH_N(N, macro, __VA_ARGS__)
#define FOR_EACH(macro, ...) FOR_EACH_N(COUNT_ARGS(__VA_ARGS__), macro, __VA_ARGS__)
