#pragma once

#include <rime/config.h>
#include <rime/config/config_types.h>
#include "engines/js_macros.h"
#include "js_wrapper.h"

using namespace rime;

template <typename T_JS_VALUE>
class JsWrapper<rime::ConfigList, T_JS_VALUE> {
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

  DEFINE_CFUNCTION_ARGC(pushBack, 1, {
    if (auto item = engine.unwrapShared<ConfigItem>(argv[0])) {
      auto obj = engine.unwrapShared<rime::ConfigList>(thisVal);
      obj->Append(item);
    }
    return engine.undefined();
  })

  DEFINE_CFUNCTION(clear, {
    auto obj = engine.unwrapShared<rime::ConfigList>(thisVal);
    obj->Clear();
    return engine.undefined();
  })

public:
  EXPORT_CLASS_WITH_SHARED_POINTER(
      ConfigList,
      WITHOUT_CONSTRUCTOR,
      WITHOUT_PROPERTIES,
      WITHOUT_GETTERS,
      WITH_FUNCTIONS(getType, 0, getSize, 0, getItemAt, 1, getValueAt, 1, pushBack, 1, clear, 0));
};
