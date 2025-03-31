#pragma once

#include <type_traits>

#include "engines/js_engine.h"
#include "engines/quickjs/quickjs_engine.h"

template <typename T_JS_VALUE>
inline JsEngine<T_JS_VALUE>& getJsEngine() {
  if constexpr (std::is_same_v<T_JS_VALUE, JSValue>) {
    return JsEngine<JSValue>::getInstance();
  } else {
    // compile time error
    static_assert(false, "Unsupported JS engine");
  }
}
