#pragma once

#include <rime/schema.h>
#include "engines/engine_manager.h"
#include "engines/js_macros.h"
#include "js_wrapper.h"

using namespace rime;

template <typename T_JS_VALUE>
class JsWrapper<rime::Schema, T_JS_VALUE> : public JsWrapperBase<T_JS_VALUE> {
  DEFINE_GETTER(Schema, id, engine.toJsString(obj->schema_id()))
  DEFINE_GETTER(Schema, name, engine.toJsString(obj->schema_name()))
  DEFINE_GETTER(Schema, config, engine.wrap<Config>(obj->config()))
  DEFINE_GETTER(Schema, pageSize, engine.toJsInt(obj->page_size()))
  DEFINE_GETTER(Schema, selectKeys, engine.toJsString(obj->select_keys()))

public:
  static const char* getTypeName() { return "Schema"; }

  typename TypeMap<T_JS_VALUE>::ExposePropertyType* getGetters() override {
    auto& engine = getJsEngine<T_JS_VALUE>();
    static typename TypeMap<T_JS_VALUE>::ExposePropertyType getters[] = {
        engine.defineProperty("id", get_id, nullptr),
        engine.defineProperty("name", get_name, nullptr),
        engine.defineProperty("config", get_config, nullptr),
        engine.defineProperty("pageSize", get_pageSize, nullptr),
        engine.defineProperty("selectKeys", get_selectKeys, nullptr),
    };
    this->setGetterCount(countof(getters));

    return getters;
  }
};
