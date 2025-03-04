#ifndef RIME_QJS_MACROS_H_
#define RIME_QJS_MACROS_H_

#include <quickjs.h>
#include "jsstring_raii.h"

#define NO_PROPERTY_TO_DECLARE void useless() {};
#define NO_PROPERTY_TO_REGISTER {}
#define NO_FUNCTION_TO_REGISTER {}
#define NO_CONSTRUCTOR_TO_REGISTER {}

// Helper macro for FOR_EACH implementation
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
#define COUNT_ARGS_(_1, _2, _3, _4, _5, _6, _7, _8, _9, _10, \
                    _11, _12, _13, _14, _15, _16, _17, _18, _19, _20,  N, ...) N
#define COUNT_ARGS(...) COUNT_ARGS_(__VA_ARGS__, \
  20, 19, 18, 17, 16, 15, 14, 13, 12, 11, \
  10, 9, 8, 7, 6, 5, 4, 3, 2, 1)

// Select the appropriate FOR_EACH macro based on argument count
#define _FOR_EACH_N(N, macro, ...) FOR_EACH_##N(macro, __VA_ARGS__)
#define FOR_EACH_N(N, macro, ...) _FOR_EACH_N(N, macro, __VA_ARGS__)
#define FOR_EACH(macro, ...) FOR_EACH_N(COUNT_ARGS(__VA_ARGS__), macro, __VA_ARGS__)

#define countof(x) (sizeof(x) / sizeof((x)[0]))

// ========== class declaration ==========
#define DECLARE_WRAP_UNWRAP_WITH_RAW_POINTER(class_name) \
  static JSValue Wrap(JSContext* ctx, class_name* obj); \
  static class_name* Unwrap(JSContext* ctx, JSValue value);

#define DECLARE_WRAP_UNWRAP_WITH_SHARED_POINTER(class_name) \
  static JSValue Wrap(JSContext* ctx, std::shared_ptr<class_name> obj); \
  static std::shared_ptr<class_name> Unwrap(JSContext* ctx, JSValue value);

#define DECLARE_JS_CLASS(class_name, declareWrapUnwrap) \
namespace rime { \
class Qjs##class_name : public QjsTypeRegistry { \
public: \
  void Register(JSContext* ctx) override; \
  const char* GetClassName() const override { return #class_name; } \
  declareWrapUnwrap \
}; \
} // namespace rime

#define DECLARE_JS_CLASS_WITH_RAW_POINTER(class_name) \
  DECLARE_JS_CLASS(class_name, DECLARE_WRAP_UNWRAP_WITH_RAW_POINTER(class_name))

#define DECLARE_JS_CLASS_WITH_SHARED_POINTER(class_name) \
  DECLARE_JS_CLASS(class_name, DECLARE_WRAP_UNWRAP_WITH_SHARED_POINTER(class_name))

// ========== class implementation ==========
#define DEFINE_CONSTRUCTOR(class_name, constructor, expectingArgc)             \
  /* do not free the jsConstructor */                                          \
  JSValue jsConstructor = JS_NewCFunction2(ctx,                                \
    constructor, #class_name, expectingArgc, JS_CFUNC_constructor, 0);         \
  JS_SetConstructor(ctx, jsConstructor, proto);                                \
                                                                               \
  auto jsGlobal = JS_GetGlobalObject(ctx);                                     \
  JS_SetPropertyStr(ctx, jsGlobal, #class_name, jsConstructor);                \
  JS_FreeValue(ctx, jsGlobal);

#define DEFINE_PROPERTY(name) JS_CGETSET_DEF(#name, get_##name, set_##name),

#define DEFINE_PROPERTIES(...)                                                 \
  const JSCFunctionListEntry properties[] = {                                  \
    FOR_EACH(DEFINE_PROPERTY, __VA_ARGS__)                                     \
  };                                                                           \
  JS_SetPropertyFunctionList(ctx, proto, properties, countof(properties));

#define REGISTER_GETTER(name) JS_CGETSET_DEF(#name, get_##name, NULL),

#define DEFINE_GETTERS(...)                                                    \
  const JSCFunctionListEntry properties[] = {                                  \
    FOR_EACH(REGISTER_GETTER, __VA_ARGS__)                                     \
  };                                                                           \
  JS_SetPropertyFunctionList(ctx, proto, properties, countof(properties));

#define DEFINE_FUNCTIONS(...)                                                  \
  /* jsMethods must be defined static */                                       \
  /* otherwise the methods would be unavailable in js*/                        \
  static const JSCFunctionListEntry jsMethods[] = {                            \
     __VA_ARGS__                                                               \
  };                                                                           \
  JS_SetPropertyFunctionList(ctx, proto, jsMethods, countof(jsMethods));

#define WRAP_UNWRAP_WITH_RAW_POINTER(class_name)                               \
[[nodiscard]]                                                                  \
JSValue Qjs##class_name::Wrap(JSContext* ctx, class_name* obj) {               \
  if (!obj) return JS_NULL;                                                    \
  JSValue jsobj = JS_NewObjectClass(ctx, js_##class_name##_class_id);          \
  if (JS_IsException(jsobj)) {                                                 \
    return jsobj;                                                              \
  }                                                                            \
  if (JS_SetOpaque(jsobj, obj) < 0) {                                          \
    JS_FreeValue(ctx, jsobj);                                                  \
    return JS_ThrowInternalError(ctx,                                          \
      "Failed to set a raw pointer to a %s object", #class_name);              \
  };                                                                           \
  return jsobj;                                                                \
}                                                                              \
                                                                               \
[[nodiscard]]                                                                  \
class_name* Qjs##class_name::Unwrap(JSContext* ctx, JSValue value) {           \
  if (auto ptr = JS_GetOpaque(value, js_##class_name##_class_id)) {            \
    return static_cast<class_name*>(ptr);                                      \
  }                                                                            \
  return nullptr;                                                              \
}                                                                              \
                                                                               \
static void js_##class_name##_finalizer(JSRuntime* rt, JSValue val) {          \
  DLOG(INFO) << "Calling js_" << #class_name "_finalizer."                     \
             << "The raw pointer is passed from the rime engine,"              \
             << "so do not free it here.";                                     \
}                                                                              \

#define WRAP_UNWRAP_WITH_SHARED_POINTER(class_name)                            \
[[nodiscard]]                                                                  \
JSValue Qjs##class_name::Wrap(JSContext* ctx, std::shared_ptr<class_name> obj) { \
  if (!obj) return JS_NULL;                                                    \
  JSValue jsobj = JS_NewObjectClass(ctx, js_##class_name##_class_id);          \
  if (JS_IsException(jsobj)) {                                                 \
    return jsobj;                                                              \
  }                                                                            \
  auto ptr = std::make_unique<std::shared_ptr<class_name>>(obj);               \
  if (JS_SetOpaque(jsobj, ptr.release()) < 0) {                                \
    JS_FreeValue(ctx, jsobj);                                                  \
    return JS_ThrowInternalError(ctx,                                          \
      "Failed to set a shared pointer to a %s object", #class_name);           \
  };                                                                           \
  return jsobj;                                                                \
}                                                                              \
                                                                               \
[[nodiscard]]                                                                  \
std::shared_ptr<class_name> Qjs##class_name::Unwrap(JSContext* ctx, JSValue value) { \
  if (auto ptr = JS_GetOpaque(value, js_##class_name##_class_id)) {            \
    if (auto sharedPtr = static_cast<std::shared_ptr<class_name>*>(ptr)) {     \
      return *sharedPtr;                                                       \
    }                                                                          \
  }                                                                            \
  return nullptr;                                                              \
}                                                                              \
static void js_##class_name##_finalizer(JSRuntime* rt, JSValue val) {          \
  DLOG(INFO) << "Calling js_" << #class_name << "_finalizer.";                 \
  if (auto ptr = JS_GetOpaque(val, js_##class_name##_class_id)) {              \
    auto ppObj = static_cast<std::shared_ptr<class_name>*>(ptr);               \
    (*ppObj).reset();                                                          \
  }                                                                            \
}                                                                              \

#define DEFINE_JS_CLASS_IMPL(class_name, defineWrapUnwrap, constructor, properties, methods)       \
static JSClassID js_##class_name##_class_id;                                   \
                                                                               \
defineWrapUnwrap;                                                              \
                                                                               \
static JSClassDef js_##class_name##_class = {                                  \
  #class_name,                                                                 \
  .finalizer = js_##class_name##_finalizer                                     \
};                                                                             \
                                                                               \
void Qjs##class_name::Register(JSContext* ctx) {                               \
  auto rt = QjsHelper::getInstance().getRuntime();                             \
  JS_NewClassID(rt, &js_##class_name##_class_id);                              \
  JS_NewClass(rt, js_##class_name##_class_id, &js_##class_name##_class);       \
                                                                               \
  JSValue proto = JS_NewObject(ctx);                                           \
                                                                               \
  constructor                                                                  \
  properties                                                                   \
  methods                                                                      \
                                                                               \
  JS_SetClassProto(ctx, js_##class_name##_class_id, proto);                    \
}

#define DEFINE_JS_CLASS_WITH_SHARED_POINTER(class_name, constructor, properties, methods) \
  DEFINE_JS_CLASS_IMPL(class_name, WRAP_UNWRAP_WITH_SHARED_POINTER(class_name), EXPAND(constructor), EXPAND(properties), EXPAND(methods))

#define DEFINE_JS_CLASS_WITH_RAW_POINTER(class_name, constructor, properties, methods) \
  DEFINE_JS_CLASS_IMPL(class_name, WRAP_UNWRAP_WITH_RAW_POINTER(class_name), EXPAND(constructor), EXPAND(properties), EXPAND(methods))

#define DEFINE_GETTER(class_name, jsName, statement)                           \
static JSValue get_##jsName(JSContext* ctx, JSValueConst this_val) {                  \
  if (auto obj = Qjs##class_name::Unwrap(ctx, this_val)) {                     \
    return statement;                                                          \
  }                                                                            \
  return JS_UNDEFINED;                                                         \
}

#define DEFINE_STRING_SETTER(class_name, name, assignment)                     \
static JSValue set_##name(JSContext* ctx, JSValueConst this_val, JSValue val) { \
  if (auto obj = Qjs##class_name::Unwrap(ctx, this_val)) {                                    \
    if (const char* str = JS_ToCString(ctx, val)) {                          \
      assignment;                                                            \
      JS_FreeCString(ctx, str);                                              \
      return JS_UNDEFINED;                                                   \
    } else {                                                                 \
      return JS_ThrowTypeError(ctx,                                          \
        " %s.%s = rvalue: rvalue is not a string", #class_name, #name);      \
    }                                                                        \
  }                                                                          \
  return JS_ThrowTypeError(ctx,                                              \
    "Failed to unwrap the js object to a cpp %s object", #class_name);       \
}

#define DEFINE_SETTER(class_name, jsName, type, converter, assignment)         \
static JSValue set_##jsName(JSContext* ctx, JSValueConst this_val, JSValue val) { \
  if (auto obj = Qjs##class_name::Unwrap(ctx, this_val)) {                                    \
    type value;                                                              \
    if (converter(ctx, &value, val) == 0) {                                  \
      assignment;                                                            \
      return JS_UNDEFINED;                                                   \
    } else {                                                                 \
      return JS_ThrowTypeError(ctx,                                          \
        "%s.%s = rvalue: rvalue could not be extracted with %s",             \
        #class_name, #jsName, #converter);                                   \
    }                                                                        \
  }                                                                          \
  return JS_ThrowTypeError(ctx,                                              \
    "Failed to unwrap the js object to a cpp %s object", #class_name);       \
}

#define DEFINE_FORBIDDEN_SETTER(class_name, name)                              \
static JSValue set_##name(JSContext* ctx, JSValueConst this_val, JSValue val) { \
  return JS_ThrowTypeError(ctx,                                              \
    "Cannot assign to read only property '%s'", #name);                      \
}

#define DEF_FUNC(class_name, func_name, statements)                            \
static JSValue func_name(JSContext* ctx, JSValueConst this_val, int argc, JSValueConst* argv) {       \
  auto obj = Qjs##class_name::Unwrap(ctx, this_val);                                            \
  if (!obj) {                                                                  \
    return JS_ThrowTypeError(ctx,                                              \
      "Failed to unwrap the js object to a cpp %s object", #class_name);       \
  }                                                                            \
  statements;                                                                  \
}

// define a js function with ${expectingArgc} arguments
#define DEF_FUNC_WITH_ARGC(class_name, func_name, expectingArgc, statements)   \
static JSValue func_name(                                            \
  JSContext* ctx, JSValueConst this_val, int argc, JSValueConst* argv) {       \
                                                                               \
  if (argc < expectingArgc) {                                                  \
    return JS_ThrowSyntaxError(ctx, "%s.%s(...) expects %d arguments",         \
      #class_name, #func_name, expectingArgc);                                 \
  }                                                                            \
  auto obj = Qjs##class_name::Unwrap(ctx, this_val);                                            \
  if (!obj) {                                                                  \
    return JS_ThrowTypeError(ctx,                                              \
      "Failed to unwrap the js object to a cpp %s object", #class_name);       \
  }                                                                            \
  statements;                                                                  \
}

#define STRING_ASSIGNMENT_FROM_JS_ARGV(name, index)                            \
    if (const char* name = JS_ToCString(ctx, argv[index])) {                   \
      obj->set_##name(name);                                                   \
      JS_FreeCString(ctx, name);                                               \
    } else {                                                                   \
      return JS_ThrowTypeError(ctx,                                            \
        "argv[%d] should be of string type", index);                           \
    }

#define NUMERIC_ASSIGNMENT_FROM_JS_ARGV(name, index, type, converter)          \
    type name;                                                                 \
    if (converter(ctx, &name, argv[index]) == 0) {                             \
      obj->set_##name(name);                                                   \
    } else {                                                                   \
      return JS_ThrowTypeError(ctx,                                            \
        "argv[%d] should be of %s type", index, #type);                        \
    }

#endif  // RIME_QJS_MACROS_H_
