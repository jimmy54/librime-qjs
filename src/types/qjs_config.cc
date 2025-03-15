#include "qjs_config.h"  // IWYU pragma: keep

#include <filesystem>
#include <string>

#include "qjs_config_list.h"

namespace rime {

DEFINE_FUNCTION_ARGC(Config, loadFromFile, 1, {
  JSStringRAII param(JS_ToCString(ctx, argv[0]));
  return JS_NewBool(ctx, obj->LoadFromFile(std::filesystem::path(param)));
})
DEFINE_FUNCTION_ARGC(Config, saveToFile, 1, {
  JSStringRAII param(JS_ToCString(ctx, argv[0]));
  return JS_NewBool(ctx, obj->SaveToFile(std::filesystem::path(param)));
})
DEFINE_FUNCTION_ARGC(Config, getBool, 1, {
  JSStringRAII param(JS_ToCString(ctx, argv[0]));
  bool value = false;
  bool success = obj->GetBool(param, &value);
  return success ? JS_NewBool(ctx, value) : JS_NULL;
})
DEFINE_FUNCTION_ARGC(Config, getInt, 1, {
  JSStringRAII param(JS_ToCString(ctx, argv[0]));
  int value = 0;
  bool success = obj->GetInt(param, &value);
  return success ? JS_NewInt32(ctx, value) : JS_NULL;
})
DEFINE_FUNCTION_ARGC(Config, getDouble, 1, {
  JSStringRAII param(JS_ToCString(ctx, argv[0]));
  double value = 0.0;
  bool success = obj->GetDouble(param, &value);
  return success ? JS_NewFloat64(ctx, value) : JS_NULL;
})
DEFINE_FUNCTION_ARGC(Config, getString, 1, {
  JSStringRAII param(JS_ToCString(ctx, argv[0]));
  std::string value;
  bool success = obj->GetString(param, &value);
  return success ? JS_NewString(ctx, value.c_str()) : JS_NULL;
})
DEFINE_FUNCTION_ARGC(Config, getList, 1, {
  JSStringRAII param(JS_ToCString(ctx, argv[0]));
  auto list = obj->GetList(param);
  return QjsConfigList::wrap(ctx, list);
})

DEFINE_FUNCTION_ARGC(Config, setBool, 2, {
  JSStringRAII firstParam(JS_ToCString(ctx, argv[0]));
  bool value = JS_ToBool(ctx, argv[1]);
  bool success = obj->SetBool(firstParam, value);
  return JS_NewBool(ctx, success);
})
DEFINE_FUNCTION_ARGC(Config, setInt, 2, {
  JSStringRAII firstParam(JS_ToCString(ctx, argv[0]));
  int32_t value;
  JS_ToInt32(ctx, &value, argv[1]);
  bool success = obj->SetInt(firstParam, value);
  return JS_NewBool(ctx, success);
})
DEFINE_FUNCTION_ARGC(Config, setDouble, 2, {
  JSStringRAII firstParam(JS_ToCString(ctx, argv[0]));
  double value;
  JS_ToFloat64(ctx, &value, argv[1]);
  bool success = obj->SetDouble(firstParam, value);
  return JS_NewBool(ctx, success);
})
DEFINE_FUNCTION_ARGC(Config, setString, 2, {
  JSStringRAII firstParam(JS_ToCString(ctx, argv[0]));
  JSStringRAII value(JS_ToCString(ctx, argv[1]));
  bool success = obj->SetString(firstParam, value);
  return JS_NewBool(ctx, success);
})

DEFINE_JS_CLASS_WITH_RAW_POINTER(Config,
                                 NO_CONSTRUCTOR_TO_REGISTER,
                                 NO_PROPERTY_TO_REGISTER,
                                 NO_GETTER_TO_REGISTER,
                                 DEFINE_FUNCTIONS(JS_CFUNC_DEF("loadFromFile", 1, loadFromFile),
                                                  JS_CFUNC_DEF("saveToFile", 1, saveToFile),
                                                  JS_CFUNC_DEF("getBool", 1, getBool),
                                                  JS_CFUNC_DEF("getInt", 1, getInt),
                                                  JS_CFUNC_DEF("getDouble", 1, getDouble),
                                                  JS_CFUNC_DEF("getString", 1, getString),
                                                  JS_CFUNC_DEF("getList", 1, getList),
                                                  JS_CFUNC_DEF("setBool", 2, setBool),
                                                  JS_CFUNC_DEF("setInt", 2, setInt),
                                                  JS_CFUNC_DEF("setDouble", 2, setDouble),
                                                  JS_CFUNC_DEF("setString", 2, setString), ))

}  // namespace rime
