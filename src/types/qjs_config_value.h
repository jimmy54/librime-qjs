#pragma once

#include <rime/config.h>
#include <rime/config/config_types.h>
#include "engines/js_macros.h"
#include "js_wrapper.h"

template <typename T_JS_VALUE>
class JsWrapper<rime::ConfigValue, T_JS_VALUE> : public JsWrapperBase<T_JS_VALUE> {
  DEFINE_CFUNCTION(getType, { return engine.toJsString("scalar"); })

  DEFINE_CFUNCTION(getBool, {
    auto obj = engine.unwrapShared<rime::ConfigValue>(thisVal);
    bool value = false;
    bool success = obj->GetBool(&value);
    return success ? engine.toJsBool(value) : engine.null();
  })

  DEFINE_CFUNCTION(getInt, {
    auto obj = engine.unwrapShared<rime::ConfigValue>(thisVal);
    int value = 0;
    bool success = obj->GetInt(&value);
    return success ? engine.toJsInt(value) : engine.null();
  })

  DEFINE_CFUNCTION(getDouble, {
    auto obj = engine.unwrapShared<rime::ConfigValue>(thisVal);
    double value = 0;
    bool success = obj->GetDouble(&value);
    return success ? engine.toJsDouble(value) : engine.null();
  })

  DEFINE_CFUNCTION(getString, {
    auto obj = engine.unwrapShared<rime::ConfigValue>(thisVal);
    std::string value;
    bool success = obj->GetString(&value);
    return success ? engine.toJsString(value.c_str()) : engine.null();
  })

public:
  static const char* getTypeName() { return "ConfigValue"; }

  EXPORT_FUNCTIONS(getType, 0, getBool, 0, getInt, 0, getDouble, 0, getString, 0);
};
