#pragma once

#include <rime/config.h>
#include <filesystem>
#include <string>
#include "engines/engine_manager.h"
#include "engines/js_macros.h"
#include "js_wrapper.h"

template <typename T_JS_VALUE>
class JsWrapper<rime::Config, T_JS_VALUE> : public JsWrapperBase<T_JS_VALUE> {
  DEFINE_CFUNCTION_ARGC(loadFromFile, 1, {
    std::string path = engine.toStdString(argv[0]);
    auto obj = engine.unwrap<rime::Config>(thisVal);
    return engine.toJsBool(obj->LoadFromFile(std::filesystem::path(path.c_str())));
  })

  DEFINE_CFUNCTION_ARGC(saveToFile, 1, {
    std::string path = engine.toStdString(argv[0]);
    auto obj = engine.unwrap<rime::Config>(thisVal);
    return engine.toJsBool(obj->SaveToFile(std::filesystem::path(path.c_str())));
  })

  DEFINE_CFUNCTION_ARGC(getBool, 1, {
    std::string key = engine.toStdString(argv[0]);
    auto obj = engine.unwrap<rime::Config>(thisVal);
    bool value = false;
    bool success = obj->GetBool(key, &value);
    return success ? engine.toJsBool(value) : engine.null();
  })

  DEFINE_CFUNCTION_ARGC(getInt, 1, {
    std::string key = engine.toStdString(argv[0]);
    auto obj = engine.unwrap<rime::Config>(thisVal);
    int value = 0;
    bool success = obj->GetInt(key, &value);
    return success ? engine.toJsInt(value) : engine.null();
  })

  DEFINE_CFUNCTION_ARGC(getDouble, 1, {
    std::string key = engine.toStdString(argv[0]);
    auto obj = engine.unwrap<rime::Config>(thisVal);
    double value = 0.0;
    bool success = obj->GetDouble(key, &value);
    return success ? engine.toJsDouble(value) : engine.null();
  })

  DEFINE_CFUNCTION_ARGC(getString, 1, {
    std::string key = engine.toStdString(argv[0]);
    auto obj = engine.unwrap<rime::Config>(thisVal);
    std::string value;
    bool success = obj->GetString(key, &value);
    return success ? engine.toJsString(value.c_str()) : engine.null();
  })

  DEFINE_CFUNCTION_ARGC(getList, 1, {
    std::string key = engine.toStdString(argv[0]);
    auto obj = engine.unwrap<rime::Config>(thisVal);
    auto list = obj->GetList(key);
    return engine.wrapShared(list);
  })

  DEFINE_CFUNCTION_ARGC(setBool, 2, {
    std::string key = engine.toStdString(argv[0]);
    auto obj = engine.unwrap<rime::Config>(thisVal);
    bool value = JS_ToBool(ctx, argv[1]);
    bool success = obj->SetBool(key, value);
    return engine.toJsBool(success);
  })

  DEFINE_CFUNCTION_ARGC(setInt, 2, {
    std::string key = engine.toStdString(argv[0]);
    auto obj = engine.unwrap<rime::Config>(thisVal);
    int32_t value;
    JS_ToInt32(ctx, &value, argv[1]);
    bool success = obj->SetInt(key, value);
    return engine.toJsBool(success);
  })

  DEFINE_CFUNCTION_ARGC(setDouble, 2, {
    std::string key = engine.toStdString(argv[0]);
    auto obj = engine.unwrap<rime::Config>(thisVal);
    double value;
    JS_ToFloat64(ctx, &value, argv[1]);
    bool success = obj->SetDouble(key, value);
    return engine.toJsBool(success);
  })

  DEFINE_CFUNCTION_ARGC(setString, 2, {
    std::string key = engine.toStdString(argv[0]);
    auto obj = engine.unwrap<rime::Config>(thisVal);
    std::string value = engine.toStdString(argv[1]);
    bool success = obj->SetString(key, value);
    return engine.toJsBool(success);
  })

public:
  static const char* getTypeName() { return "Config"; }

  typename TypeMap<T_JS_VALUE>::ExposeFunctionType* getFunctions() override {
    auto& engine = getJsEngine<T_JS_VALUE>();
    static typename TypeMap<T_JS_VALUE>::ExposeFunctionType functions[] = {
        engine.defineFunction("loadFromFile", 1, loadFromFile),
        engine.defineFunction("saveToFile", 1, saveToFile),
        engine.defineFunction("getBool", 1, getBool),
        engine.defineFunction("getInt", 1, getInt),
        engine.defineFunction("getDouble", 1, getDouble),
        engine.defineFunction("getString", 1, getString),
        engine.defineFunction("getList", 1, getList),
        engine.defineFunction("setBool", 2, setBool),
        engine.defineFunction("setInt", 2, setInt),
        engine.defineFunction("setDouble", 2, setDouble),
        engine.defineFunction("setString", 2, setString),
    };
    this->setFunctionCount(countof(functions));
    return functions;
  }
};
