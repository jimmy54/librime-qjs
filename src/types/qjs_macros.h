#ifndef RIME_QJS_MACROS_H_
#define RIME_QJS_MACROS_H_

#include <quickjs.h>

// Helper macro for FOR_EACH implementation
#define EXPAND(x) x
#define FOR_EACH_1(macro, x) macro(x)
#define FOR_EACH_2(macro, x, ...) macro(x) EXPAND(FOR_EACH_1(macro, __VA_ARGS__))
#define FOR_EACH_3(macro, x, ...) macro(x) EXPAND(FOR_EACH_2(macro, __VA_ARGS__))
#define FOR_EACH_4(macro, x, ...) macro(x) EXPAND(FOR_EACH_3(macro, __VA_ARGS__))
#define FOR_EACH_5(macro, x, ...) macro(x) EXPAND(FOR_EACH_4(macro, __VA_ARGS__))
#define FOR_EACH_6(macro, x, ...) macro(x) EXPAND(FOR_EACH_5(macro, __VA_ARGS__))
#define FOR_EACH_7(macro, x, ...) macro(x) EXPAND(FOR_EACH_6(macro, __VA_ARGS__))
#define FOR_EACH_8(macro, x, ...) macro(x) EXPAND(FOR_EACH_7(macro, __VA_ARGS__))

// Get number of arguments
#define COUNT_ARGS_(_1, _2, _3, _4, _5, _6, _7, _8, N, ...) N
#define COUNT_ARGS(...) COUNT_ARGS_(__VA_ARGS__, 8, 7, 6, 5, 4, 3, 2, 1)

// Select the appropriate FOR_EACH macro based on argument count
#define _FOR_EACH_N(N, macro, ...) FOR_EACH_##N(macro, __VA_ARGS__)
#define FOR_EACH_N(N, macro, ...) _FOR_EACH_N(N, macro, __VA_ARGS__)
#define FOR_EACH(macro, ...) FOR_EACH_N(COUNT_ARGS(__VA_ARGS__), macro, __VA_ARGS__)

#define countof(x) (sizeof(x) / sizeof((x)[0]))

// ========== class declaration ==========
#define DECLARE_PROPERTIES(...) \
  FOR_EACH(DECLARE_GETTER_SETTER, __VA_ARGS__)

#define DECLARE_GETTER_SETTER(name) \
  static JSValue get_##name(JSContext* ctx, JSValueConst this_val); \
  static JSValue set_##name(JSContext* ctx, JSValueConst this_val, JSValue val);

#define DECLARE_JS_CLASS(class_name, declareProperties, declareFunctions) \
namespace rime { \
class Qjs##class_name : public QjsTypeRegistry { \
public: \
  void Register(JSContext* ctx) override; \
  const char* GetClassName() const override { return #class_name; } \
\
  static JSValue Wrap(JSContext* ctx, std::shared_ptr<class_name> obj); \
  static std::shared_ptr<class_name> Unwrap(JSContext* ctx, JSValue value); \
\
private: \
  declareProperties \
  declareFunctions \
}; \
} // namespace rime


// ========== class implementation ==========
// Helper macro to expand property definitions
#define DEFINE_PROPERTY(name) JS_CGETSET_DEF(#name, get_##name, set_##name),

// Macro to define multiple properties at once
#define DEFINE_PROPERTIES(...)                                           \
  const JSCFunctionListEntry properties[] = {                            \
    FOR_EACH(DEFINE_PROPERTY, __VA_ARGS__)                        \
  };                                                                     \
  JS_SetPropertyFunctionList(ctx, proto, properties, countof(properties))

#define DEFINE_JS_CLASS(class_name, registerProperties, registerFunctions)   \
static JSClassID js_##class_name##_class_id;                                 \
                                                                             \
static void js_##class_name##_finalizer(JSRuntime* rt, JSValue val) {        \
    if (auto ptr = JS_GetOpaque(val, js_##class_name##_class_id)) {          \
      delete static_cast<std::shared_ptr<class_name>*>(ptr);                 \
    }                                                                        \
}                                                                            \
                                                                             \
static JSClassDef js_##class_name##_class = {                                \
  #class_name,                                                            \
  .finalizer = js_##class_name##_finalizer                                   \
};                                                                           \
                                                                             \
                                                                             \
void Qjs##class_name::Register(JSContext* ctx) {                              \
  auto rt = JS_GetRuntime(ctx);                                              \
  JS_NewClassID(rt, &js_##class_name##_class_id);                            \
  JS_NewClass(rt, js_##class_name##_class_id, &js_##class_name##_class);     \
                                                                             \
  JSValue proto = JS_NewObject(ctx);                                         \
                                                                             \
  registerProperties;                                                         \
  registerFunctions;                                                          \
                                                                             \
  JSValue dup_proto = JS_DupValue(ctx, proto);                               \
  JS_SetClassProto(ctx, js_##class_name##_class_id, dup_proto);              \
                                                                             \
  JS_FreeValue(ctx, proto);                                                  \
}                                                                            \
                                                                             \
[[nodiscard]]                                                                \
JSValue Qjs##class_name::Wrap(JSContext* ctx, std::shared_ptr<class_name> obj) {              \
  if (!obj) return JS_NULL;                                                  \
  JSValue jsobj = JS_NewObjectClass(ctx, js_##class_name##_class_id);        \
  auto ptr = std::make_unique<std::shared_ptr<class_name>>(obj);             \
  if (JS_SetOpaque(jsobj, ptr.release()) < 0) {                              \
    JS_FreeValue(ctx, jsobj);                                                \
    return JS_ThrowInternalError(ctx,                                        \
      "Failed to set opaque pointer for ##class_name object");               \
  }                                                                          \
  return jsobj;                                                              \
}                                                                            \
                                                                             \
[[nodiscard]]                                                                \
std::shared_ptr<class_name> Qjs##class_name::Unwrap(JSContext* ctx, JSValue value) {          \
  if (auto ptr = JS_GetOpaque(value, js_##class_name##_class_id)) {          \
    if (auto sharedPtr = static_cast<std::shared_ptr<class_name>*>(ptr)) {   \
      return *sharedPtr;                                                     \
    }                                                                        \
  }                                                                          \
  return nullptr;                                                            \
}                                                                            \

#define DEFINE_GETTER(class_name, name, type, converter)                             \
  [[nodiscard]]                                                         \
  JSValue Qjs##class_name::get_##name(JSContext* ctx, JSValueConst this_val) {           \
    if (auto obj = Unwrap(ctx, this_val)) {                            \
      return converter(ctx, obj->name());                              \
    }                                                                  \
    return JS_UNDEFINED;                                               \
  }

#define DEFINE_STRING_SETTER(class_name, name, assignment)                          \
  JSValue Qjs##class_name::set_##name(JSContext* ctx, JSValueConst this_val, JSValue val) { \
    if (auto obj = Unwrap(ctx, this_val)) {                            \
      if (const char* str = JS_ToCString(ctx, val)) {                  \
        assignment                                                      \
        JS_FreeCString(ctx, str);                                      \
      }                                                                \
    }                                                                  \
    return JS_UNDEFINED;                                               \
  }

#define DEFINE_NUMERIC_SETTER(class_name, name, type, converter)                    \
  JSValue Qjs##class_name::set_##name(JSContext* ctx, JSValueConst this_val, JSValue val) { \
    if (auto obj = Unwrap(ctx, this_val)) {                            \
      type value;                                                      \
      if (converter(ctx, &value, val) == 0) {                         \
        obj->set_##name(value);                                        \
      }                                                                \
    }                                                                  \
    return JS_UNDEFINED;                                               \
  }

#endif  // RIME_QJS_MACROS_H_
