#pragma once

#include <JavaScriptCore/JavaScript.h>
#include <JavaScriptCore/JavaScriptCore.h>

#define DEFINE_GETTER_IMPL(T_RIME_TYPE, propertieyName, statement, unwrap)                       \
                                                                                                 \
  DEFINE_GETTER_IMPL_QJS(T_RIME_TYPE, propertieyName, statement, unwrap)                         \
                                                                                                 \
  static JSValueRef get_##propertieyName##Jsc(JSContextRef ctx, JSObjectRef thisVal,             \
                                              JSStringRef functionName, JSValueRef* exception) { \
    auto& engine = getJsEngine<JSValueRef>();                                                    \
    if (auto obj = unwrap) {                                                                     \
      return statement;                                                                          \
    }                                                                                            \
    return engine.undefined();                                                                   \
  }

#define DEFINE_STRING_SETTER_IMPL(T_RIME_TYPE, name, assignment, unwrap)                           \
                                                                                                   \
  DEFINE_STRING_SETTER_IMPL_QJS(T_RIME_TYPE, name, assignment, unwrap)                             \
                                                                                                   \
  static bool set_##name##Jsc(JSContextRef ctx, JSObjectRef thisVal, JSStringRef propertyName,     \
                              JSValueRef val, JSValueRef* exception) {                             \
    auto& engine = getJsEngine<JSValueRef>();                                                      \
    if (auto obj = unwrap) {                                                                       \
      auto str = engine.toStdString(val);                                                          \
      if (!str.empty()) {                                                                          \
        assignment;                                                                                \
        return true;                                                                               \
      }                                                                                            \
      *exception = engine.throwError(JsErrorType::TYPE, " %s.%s = rvalue: rvalue is not a string", \
                                     #T_RIME_TYPE, #name);                                         \
      return false;                                                                                \
    }                                                                                              \
    *exception = engine.throwError(                                                                \
        JsErrorType::TYPE, "Failed to unwrap the js object to a cpp %s object", #T_RIME_TYPE);     \
    return false;                                                                                  \
  }

#define DEFINE_SETTER_IMPL(T_RIME_TYPE, jsName, converter, assignment, unwrap)                   \
                                                                                                 \
  DEFINE_SETTER_IMPL_QJS(T_RIME_TYPE, jsName, converter, assignment, unwrap)                     \
                                                                                                 \
  static bool set_##jsName##Jsc(JSContextRef ctx, JSObjectRef thisVal, JSStringRef propertyName, \
                                JSValueRef val, JSValueRef* exception) {                         \
    auto& engine = getJsEngine<JSValueRef>();                                                    \
    if (auto obj = unwrap) {                                                                     \
      auto value = converter(val);                                                               \
      assignment;                                                                                \
      return true;                                                                               \
    }                                                                                            \
    *exception = engine.throwError(                                                              \
        JsErrorType::TYPE, "Failed to unwrap the js object to a cpp %s object", #T_RIME_TYPE);   \
    return false;                                                                                \
  }

#define DEFINE_CFUNCTION(funcName, funcBody)                                                     \
                                                                                                 \
  DEFINE_CFUNCTION_QJS(funcName, funcBody)                                                       \
                                                                                                 \
  static JSValueRef funcName##Jsc(JSContextRef ctx, JSObjectRef function, JSObjectRef thisVal,   \
                                  size_t argc, const JSValueRef argv[], JSValueRef* exception) { \
    auto& engine = getJsEngine<JSValueRef>();                                                    \
    try {                                                                                        \
      funcBody;                                                                                  \
    } catch (const JsException& e) {                                                             \
      *exception = engine.throwError(JsErrorType::TYPE, e.what());                               \
      return nullptr;                                                                            \
    }                                                                                            \
  }

#define DEFINE_CFUNCTION_ARGC(funcName, expectingArgc, statements)                               \
                                                                                                 \
  DEFINE_CFUNCTION_ARGC_QJS(funcName, expectingArgc, statements)                                 \
                                                                                                 \
  static JSValueRef funcName##Jsc(JSContextRef ctx, JSObjectRef function, JSObjectRef thisVal,   \
                                  size_t argc, const JSValueRef argv[], JSValueRef* exception) { \
    auto& engine = getJsEngine<JSValueRef>();                                                    \
    if (argc < expectingArgc) {                                                                  \
      return engine.throwError(JsErrorType::SYNTAX, "%s(...) expects %d arguments", #funcName,   \
                               expectingArgc);                                                   \
    }                                                                                            \
    try {                                                                                        \
      statements;                                                                                \
    } catch (const JsException& e) {                                                             \
      *exception = engine.throwError(JsErrorType::TYPE, e.what());                               \
      return nullptr;                                                                            \
    }                                                                                            \
  }

#define EXPORT_CONSTRUCTOR(funcName, funcBody)                                                \
                                                                                              \
  EXPORT_CONSTRUCTOR_QJS(funcName, funcBody)                                                  \
                                                                                              \
  static JSObjectRef funcName##Jsc(JSContextRef ctx, JSObjectRef function, size_t argc,       \
                                   const JSValueRef argv[], JSValueRef* exception) {          \
    auto& engine = getJsEngine<JSValueRef>();                                                 \
    try {                                                                                     \
      funcBody;                                                                               \
    } catch (const JsException& e) {                                                          \
      *exception = engine.throwError(JsErrorType::TYPE, e.what());                            \
      return nullptr;                                                                         \
    }                                                                                         \
  }                                                                                           \
                                                                                              \
  typename TypeMap<JSValueRef>::ConstructorFunctionPionterType getConstructorJsc() override { \
    return funcName##Jsc;                                                                     \
  }

#define EXPORT_FINALIZER(funcName, funcBody)                                              \
                                                                                          \
  EXPORT_FINALIZER_QJS(funcName, funcBody)                                                \
                                                                                          \
  typename TypeMap<JSValueRef>::FinalizerFunctionPionterType getFinalizerJsc() override { \
    return funcName##Jsc;                                                                 \
  }                                                                                       \
                                                                                          \
  static void funcName##Jsc(JSObjectRef val) {                                            \
    auto& engine = getJsEngine<JSValueRef>();                                             \
    funcBody;                                                                             \
  }

#define DEFINE_PROPERTY_JSC(name) engine.defineProperty(#name, get_##name##Jsc, set_##name##Jsc),

#define EXPORT_PROPERTIES(...)                                                    \
                                                                                  \
  EXPORT_PROPERTIES_QJS(__VA_ARGS__)                                              \
                                                                                  \
  typename TypeMap<JSValueRef>::ExposePropertyType* getPropertiesJsc() override { \
    auto& engine = getJsEngine<JSValueRef>();                                     \
    static typename TypeMap<JSValueRef>::ExposePropertyType properties[] = {      \
        FOR_EACH(DEFINE_PROPERTY_JSC, __VA_ARGS__)};                              \
                                                                                  \
    this->setPropertyCount(countof(properties));                                  \
                                                                                  \
    return properties;                                                            \
  }
