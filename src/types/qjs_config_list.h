#pragma once

#include <rime/config.h>
#include <rime/config/config_types.h>
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
  EXPORT_CLASS(ConfigList);

  EXPORT_FINALIZER(rime::ConfigList, finalizer);
  EXPORT_FUNCTIONS(getType, 0, getSize, 0, getItemAt, 1, getValueAt, 1);
};
