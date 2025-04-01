#pragma once

#include <type_traits>

#include "engines/js_engine.h"
#include "engines/quickjs/quickjs_engine.h"

#ifdef __APPLE__
#include "engines/javascriptcore/javascriptcore_engine.h"
#endif

template <typename T_JS_VALUE>
inline JsEngine<T_JS_VALUE>& getJsEngine() {
  if constexpr (std::is_same_v<T_JS_VALUE, JSValue>) {
    return JsEngine<JSValue>::getInstance();
#ifdef __APPLE__
  } else if constexpr (std::is_same_v<T_JS_VALUE, JSValueRef>) {
    return JsEngine<JSValueRef>::getInstance();
#endif
  } else {
    // compile time error
    static_assert(false, "Unsupported JS engine");
  }
}
