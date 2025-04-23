#pragma once

#include <rime/config.h>
#include <filesystem>
#include <string>
#include "engines/js_macros.h"
#include "js_wrapper.h"

using namespace rime;

template <>
class JsWrapper<rime::Config> {
  DEFINE_CFUNCTION_ARGC(loadFromFile, 1, {
    std::string path = engine.toStdString(argv[0]);
    auto obj = engine.unwrap<rime::Config>(thisVal);
    return engine.wrap(obj->LoadFromFile(std::filesystem::path(path.c_str())));
  })

  DEFINE_CFUNCTION_ARGC(saveToFile, 1, {
    std::string path = engine.toStdString(argv[0]);
    auto obj = engine.unwrap<rime::Config>(thisVal);
    return engine.wrap(obj->SaveToFile(std::filesystem::path(path.c_str())));
  })

  DEFINE_CFUNCTION_ARGC(getBool, 1, {
    std::string key = engine.toStdString(argv[0]);
    auto obj = engine.unwrap<rime::Config>(thisVal);
    bool value = false;
    bool success = obj->GetBool(key, &value);
    return success ? engine.wrap(value) : engine.null();
  })

  DEFINE_CFUNCTION_ARGC(getInt, 1, {
    std::string key = engine.toStdString(argv[0]);
    auto obj = engine.unwrap<rime::Config>(thisVal);
    int value = 0;
    bool success = obj->GetInt(key, &value);
    return success ? engine.wrap(value) : engine.null();
  })

  DEFINE_CFUNCTION_ARGC(getDouble, 1, {
    std::string key = engine.toStdString(argv[0]);
    auto obj = engine.unwrap<rime::Config>(thisVal);
    double value = 0.0;
    bool success = obj->GetDouble(key, &value);
    return success ? engine.wrap(value) : engine.null();
  })

  DEFINE_CFUNCTION_ARGC(getString, 1, {
    std::string key = engine.toStdString(argv[0]);
    auto obj = engine.unwrap<rime::Config>(thisVal);
    std::string value;
    bool success = obj->GetString(key, &value);
    return success ? engine.wrap(value.c_str()) : engine.null();
  })

  DEFINE_CFUNCTION_ARGC(getList, 1, {
    std::string key = engine.toStdString(argv[0]);
    auto obj = engine.unwrap<rime::Config>(thisVal);
    auto list = obj->GetList(key);
    return engine.wrap(list);
  })

  DEFINE_CFUNCTION_ARGC(setBool, 2, {
    std::string key = engine.toStdString(argv[0]);
    auto obj = engine.unwrap<rime::Config>(thisVal);
    bool value = engine.toBool(argv[1]);
    bool success = obj->SetBool(key, value);
    return engine.wrap(success);
  })

  DEFINE_CFUNCTION_ARGC(setInt, 2, {
    std::string key = engine.toStdString(argv[0]);
    auto obj = engine.unwrap<rime::Config>(thisVal);
    int value = engine.toInt(argv[1]);
    bool success = obj->SetInt(key, value);
    return engine.wrap(success);
  })

  DEFINE_CFUNCTION_ARGC(setDouble, 2, {
    std::string key = engine.toStdString(argv[0]);
    auto obj = engine.unwrap<rime::Config>(thisVal);
    double value = engine.toDouble(argv[1]);
    bool success = obj->SetDouble(key, value);
    return engine.wrap(success);
  })

  DEFINE_CFUNCTION_ARGC(setString, 2, {
    std::string key = engine.toStdString(argv[0]);
    auto obj = engine.unwrap<rime::Config>(thisVal);
    std::string value = engine.toStdString(argv[1]);
    bool success = obj->SetString(key, value);
    return engine.wrap(success);
  })

public:
  EXPORT_CLASS_WITH_RAW_POINTER(Config,
                                WITHOUT_CONSTRUCTOR,
                                WITHOUT_PROPERTIES,
                                WITHOUT_GETTERS,
                                WITH_FUNCTIONS(loadFromFile,
                                               1,
                                               saveToFile,
                                               1,
                                               getBool,
                                               1,
                                               getInt,
                                               1,
                                               getDouble,
                                               1,
                                               getString,
                                               1,
                                               getList,
                                               1,
                                               setBool,
                                               2,
                                               setInt,
                                               2,
                                               setDouble,
                                               2,
                                               setString,
                                               2));
};
