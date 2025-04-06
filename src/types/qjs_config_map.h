#pragma once

#include <rime/config.h>
#include <rime/config/config_types.h>
#include "engines/js_macros.h"
#include "js_wrapper.h"

template <typename T_JS_VALUE>
class JsWrapper<rime::ConfigMap, T_JS_VALUE> : public JsWrapperBase<T_JS_VALUE> {
  DEFINE_CFUNCTION(getType, { return engine.toJsString("map"); })

  DEFINE_CFUNCTION_ARGC(hasKey, 1, {
    auto key = engine.toStdString(argv[0]);
    auto obj = engine.unwrapShared<rime::ConfigMap>(thisVal);
    return engine.toJsBool(obj->HasKey(key));
  })

  DEFINE_CFUNCTION_ARGC(getItem, 1, {
    auto key = engine.toStdString(argv[0]);
    auto obj = engine.unwrapShared<rime::ConfigMap>(thisVal);
    auto value = obj->Get(key);
    if (!value) {
      return engine.null();
    }
    return engine.wrapShared<rime::ConfigItem>(value);
  })

  DEFINE_CFUNCTION_ARGC(getValue, 1, {
    auto key = engine.toStdString(argv[0]);
    auto obj = engine.unwrapShared<rime::ConfigMap>(thisVal);
    auto value = obj->GetValue(key);
    if (!value) {
      return engine.null();
    }
    return engine.wrapShared<rime::ConfigItem>(value);
  })

  DEFINE_CFUNCTION_ARGC(setItem, 2, {
    auto key = engine.toStdString(argv[0]);
    auto obj = engine.unwrapShared<rime::ConfigMap>(thisVal);
    if (auto item = engine.unwrapShared<rime::ConfigItem>(argv[1])) {
      obj->Set(key, item);
    }
    return engine.undefined();
  })

public:
  EXPORT_CLASS(ConfigMap);

  EXPORT_FINALIZER(rime::ConfigMap, finalizer);
  EXPORT_FUNCTIONS(getType, 0, hasKey, 1, getItem, 1, getValue, 1, setItem, 2);
};
