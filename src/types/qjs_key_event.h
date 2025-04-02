#pragma once

#include <rime/key_event.h>
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

  EXPORT_GETTERS(shift, ctrl, alt, release, repr);
};
