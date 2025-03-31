#pragma once

#include <rime/composition.h>
#include "engines/engine_manager.h"
#include "engines/js_macros.h"
#include "js_wrapper.h"

using namespace rime;

template <typename T_JS_VALUE>
class JsWrapper<rime::Preedit, T_JS_VALUE> : public JsWrapperBase<T_JS_VALUE> {
  DEFINE_GETTER_2(Preedit, text, engine.toJsString(obj->text))
  DEFINE_GETTER_2(Preedit, caretPos, engine.toJsInt(obj->caret_pos))
  DEFINE_GETTER_2(Preedit, selectStart, engine.toJsInt(obj->sel_start))
  DEFINE_GETTER_2(Preedit, selectEnd, engine.toJsInt(obj->sel_end))

  DEFINE_STRING_SETTER_2(Preedit, text, obj->text = str)
  DEFINE_SETTER_2(Preedit, caretPos, engine.toInt, obj->caret_pos = value)
  DEFINE_SETTER_2(Preedit, selectStart, engine.toInt, obj->sel_start = value)
  DEFINE_SETTER_2(Preedit, selectEnd, engine.toInt, obj->sel_end = value)

public:
  static const char* getTypeName() { return "Preedit"; }

  typename TypeMap<T_JS_VALUE>::ExposePropertyType* getProperties() override {
    auto& engine = getJsEngine<T_JS_VALUE>();
    static typename TypeMap<T_JS_VALUE>::ExposePropertyType properties[] = {
        engine.defineProperty("text", get_text, set_text),
        engine.defineProperty("caretPos", get_caretPos, set_caretPos),
        engine.defineProperty("selectStart", get_selectStart, set_selectStart),
        engine.defineProperty("selectEnd", get_selectEnd, set_selectEnd),
    };
    this->setPropertyCount(countof(properties));
    return properties;
  }
};
