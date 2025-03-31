#pragma once

#include <rime/config.h>
#include <rime/config/config_types.h>
#include "engines/engine_manager.h"
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

  typename TypeMap<T_JS_VALUE>::ExposeFunctionType* getFunctions() override {
    auto& engine = getJsEngine<T_JS_VALUE>();
    static typename TypeMap<T_JS_VALUE>::ExposeFunctionType functions[] = {
        engine.defineFunction("getType", 0, getType),
        engine.defineFunction("getBool", 0, getBool),
        engine.defineFunction("getInt", 0, getInt),
        engine.defineFunction("getDouble", 0, getDouble),
        engine.defineFunction("getString", 0, getString),
    };
    this->setFunctionCount(countof(functions));
    return functions;
  }
};
