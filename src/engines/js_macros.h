#pragma once

#include <rime/candidate.h>

#define countof(array) (sizeof(array) / sizeof(array[0]))

#define DEFINE_GETTER(T_RIME_TYPE, propertieyName, statement) \
  DEFINE_GETTER_IMPL(T_RIME_TYPE, propertieyName, statement, engine.unwrap<T_RIME_TYPE>(thisVal))

#define DEFINE_GETTER_2(T_RIME_TYPE, propertieyName, statement) \
  DEFINE_GETTER_IMPL(T_RIME_TYPE, propertieyName, statement,    \
                     engine.unwrapShared<T_RIME_TYPE>(thisVal))

#define DEFINE_GETTER_IMPL(T_RIME_TYPE, propertieyName, statement, unwrap)    \
  static JSValue get_##propertieyName(JSContext* ctx, JSValueConst thisVal) { \
    auto& engine = getJsEngine<JSValue>();                                    \
    if (auto obj = unwrap) {                                                  \
      return statement;                                                       \
    }                                                                         \
    return JS_UNDEFINED;                                                      \
  }

#define DEFINE_STRING_SETTER(T_RIME_TYPE, name, assignment) \
  DEFINE_STRING_SETTER_IMPL(T_RIME_TYPE, name, assignment, engine.unwrap<T_RIME_TYPE>(thisVal))

#define DEFINE_STRING_SETTER_2(T_RIME_TYPE, name, assignment) \
  DEFINE_STRING_SETTER_IMPL(T_RIME_TYPE, name, assignment,    \
                            engine.unwrapShared<T_RIME_TYPE>(thisVal))

#define DEFINE_STRING_SETTER_IMPL(T_RIME_TYPE, name, assignment, unwrap)                     \
  static JSValue set_##name(JSContext* ctx, JSValueConst thisVal, JSValue val) {             \
    auto& engine = getJsEngine<JSValue>();                                                   \
    if (auto obj = unwrap) {                                                                 \
      if (const char* str = JS_ToCString(ctx, val)) {                                        \
        assignment;                                                                          \
        JS_FreeCString(ctx, str);                                                            \
        return JS_UNDEFINED;                                                                 \
      }                                                                                      \
      return JS_ThrowTypeError(ctx, " %s.%s = rvalue: rvalue is not a string", #T_RIME_TYPE, \
                               #name);                                                       \
    }                                                                                        \
    return JS_ThrowTypeError(ctx, "Failed to unwrap the js object to a cpp %s object",       \
                             #T_RIME_TYPE);                                                  \
  }

#define DEFINE_SETTER(T_RIME_TYPE, jsName, converter, assignment) \
  DEFINE_SETTER_IMPL(T_RIME_TYPE, jsName, converter, assignment,  \
                     engine.unwrap<T_RIME_TYPE>(thisVal))

#define DEFINE_SETTER_2(T_RIME_TYPE, jsName, converter, assignment) \
  DEFINE_SETTER_IMPL(T_RIME_TYPE, jsName, converter, assignment,    \
                     engine.unwrapShared<T_RIME_TYPE>(thisVal))

#define DEFINE_SETTER_IMPL(T_RIME_TYPE, jsName, converter, assignment, unwrap)         \
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

#define DEFINE_CFUNCTION(funcName, funcBody)                                          \
  static JSValue funcName(JSContext* ctx, JSValue thisVal, int argc, JSValue* argv) { \
    auto& engine = getJsEngine<JSValue>();                                            \
    funcBody                                                                          \
  }

// define a js function with ${expectingArgc} arguments
#define DEFINE_CFUNCTION_ARGC(funcName, expectingArgc, statements)                               \
  static JSValue funcName(JSContext* ctx, JSValueConst thisVal, int argc, JSValueConst* argv) {  \
    if (argc < expectingArgc) {                                                                  \
      return JS_ThrowSyntaxError(ctx, "%s(...) expects %d arguments", #funcName, expectingArgc); \
    }                                                                                            \
    auto& engine = getJsEngine<JSValue>();                                                       \
    statements;                                                                                  \
  }

#define STRING_ASSIGNMENT_FROM_JS_ARGV(name, index)                            \
  if (const char* str = JS_ToCString(ctx, argv[index])) {                      \
    obj->set_##name(str);                                                      \
    JS_FreeCString(ctx, str);                                                  \
  } else {                                                                     \
    return JS_ThrowTypeError(ctx, "argv[%d] should be of string type", index); \
  }

#define NUMERIC_ASSIGNMENT_FROM_JS_ARGV(name, index, type, converter)             \
  type val;                                                                       \
  if (converter(ctx, &val, argv[index]) == 0) {                                   \
    obj->set_##name(val);                                                         \
  } else {                                                                        \
    return JS_ThrowTypeError(ctx, "argv[%d] should be of %s type", index, #type); \
  }
