#pragma once

#include <rime/key_event.h>
#include "engines/engine_manager.h"
#include "engines/js_macros.h"
#include "js_wrapper.h"

using namespace rime;

template <typename T_JS_VALUE>
class JsWrapper<rime::KeyEvent, T_JS_VALUE> : public JsWrapperBase<T_JS_VALUE> {
  DEFINE_GETTER(KeyEvent, shift, engine.toJsBool(obj->shift()))
  DEFINE_GETTER(KeyEvent, ctrl, engine.toJsBool(obj->ctrl()))
  DEFINE_GETTER(KeyEvent, alt, engine.toJsBool(obj->alt()))
  DEFINE_GETTER(KeyEvent, release, engine.toJsBool(obj->release()))
  DEFINE_GETTER(KeyEvent, repr, engine.toJsString(obj->repr()))

public:
  static const char* getTypeName() { return "KeyEvent"; }

  typename TypeMap<T_JS_VALUE>::ExposePropertyType* getGetters() override {
    auto& engine = getJsEngine<T_JS_VALUE>();
    static typename TypeMap<T_JS_VALUE>::ExposePropertyType getters[] = {
        engine.defineProperty("shift", get_shift, nullptr),
        engine.defineProperty("ctrl", get_ctrl, nullptr),
        engine.defineProperty("alt", get_alt, nullptr),
        engine.defineProperty("release", get_release, nullptr),
        engine.defineProperty("repr", get_repr, nullptr),
    };
    this->setGetterCount(countof(getters));

    return getters;
  }
};
