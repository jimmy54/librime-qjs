#pragma once

#include <type_traits>

#include "engines/js_engine.h"
#include "engines/quickjs/quickjs_engine.h"

#ifdef __APPLE__
#include "engines/javascriptcore/javascriptcore_engine.h"
#endif

template <typename T_JS_VALUE>
inline JsEngine<T_JS_VALUE>& getJsEngine() {
#ifdef __APPLE__
  if constexpr (std::is_same_v<T_JS_VALUE, JSValueRef>) {
    return JsEngine<JSValueRef>::getInstance();
  } else
#endif
      if constexpr (std::is_same_v<T_JS_VALUE, JSValue>) {
    return JsEngine<JSValue>::getInstance();
  } else {
    // Ensure type safety at compile time
    static_assert(std::is_same_v<T_JS_VALUE, JSValue>, "Unsupported JS engine type");
  }
}
