#pragma once

#include <type_traits>

#include "engines/js_engine.h"
#include "engines/quickjs/quickjs_engine.h"

#ifdef __APPLE__
#include "engines/javascriptcore/javascriptcore_engine.h"
#endif

#ifdef __APPLE__
template <typename T_JS_VALUE>
inline JsEngine<T_JS_VALUE> newOrShareEngine() {
  if constexpr (std::is_same_v<T_JS_VALUE, JSValueRef>) {
    // return a new instance of the JavaScriptCore engine, because
    // loading multiple js files into the same context may conflict names
    return JsEngine<JSValueRef>();
  } else if constexpr (std::is_same_v<T_JS_VALUE, JSValue>) {
    // all the rime plugins are using the same context, so we can share the same instance
    return JsEngine<JSValue>::instance();
  } else {
    // Ensure type safety at compile time
    static_assert(std::is_same_v<T_JS_VALUE, JSValue>, "Unsupported JS engine type");
  }
}
#else
template <typename T_JS_VALUE>
inline JsEngine<T_JS_VALUE>& newOrShareEngine() {
  if constexpr (std::is_same_v<T_JS_VALUE, JSValue>) {
    return JsEngine<JSValue>::instance();
  } else {
    // Ensure type safety at compile time
    static_assert(std::is_same_v<T_JS_VALUE, JSValue>, "Unsupported JS engine type");
  }
}
#endif
