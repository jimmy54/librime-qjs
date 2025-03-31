#pragma once

#include <rime/config.h>
#include <rime/config/config_types.h>
#include "engines/engine_manager.h"
#include "engines/js_macros.h"
#include "js_wrapper.h"

template <typename T_JS_VALUE>
class JsWrapper<rime::ConfigList, T_JS_VALUE> : public JsWrapperBase<T_JS_VALUE> {
  DEFINE_CFUNCTION(getType, { return engine.toJsString("list"); })

  DEFINE_CFUNCTION(getSize, {
    auto obj = engine.unwrapShared<rime::ConfigList>(thisVal);
    return engine.toJsInt(obj->size());
  })

  DEFINE_CFUNCTION_ARGC(getItemAt, 1, {
    int index = engine.toInt(argv[0]);
    auto obj = engine.unwrapShared<rime::ConfigList>(thisVal);

    if (index < 0 || size_t(index) >= obj->size()) {
      return engine.null();
    }

    auto item = obj->GetAt(index);
    if (!item) {
      return engine.null();
    }
    return engine.wrapShared<rime::ConfigItem>(item);
  })

  DEFINE_CFUNCTION_ARGC(getValueAt, 1, {
    int index = engine.toInt(argv[0]);
    auto obj = engine.unwrapShared<rime::ConfigList>(thisVal);

    if (index < 0 || size_t(index) >= obj->size()) {
      return engine.null();
    }

    auto value = obj->GetValueAt(index);
    if (!value) {
      return engine.null();
    }
    return engine.wrapShared<rime::ConfigValue>(value);
  })

public:
  static const char* getTypeName() { return "ConfigList"; }

  typename TypeMap<T_JS_VALUE>::ExposeFunctionType* getFunctions() override {
    auto& engine = getJsEngine<T_JS_VALUE>();
    static typename TypeMap<T_JS_VALUE>::ExposeFunctionType functions[] = {
        engine.defineFunction("getType", 0, getType),
        engine.defineFunction("getSize", 0, getSize),
        engine.defineFunction("getItemAt", 1, getItemAt),
        engine.defineFunction("getValueAt", 1, getValueAt),
    };
    this->setFunctionCount(countof(functions));
    return functions;
  }
};
