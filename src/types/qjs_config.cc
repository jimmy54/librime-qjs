#include "qjs_config.h"
#include "qjs_config_list.h"
#include <filesystem>
#include <string>

namespace rime {

DEF_FUNC_WITH_ARGC(Config, load_from_file, 1,
  JSStringRAII param(JS_ToCString(ctx, argv[0]));
  return JS_NewBool(ctx, obj->LoadFromFile(std::filesystem::path(param)));
)
DEF_FUNC_WITH_ARGC(Config, save_to_file, 1,
  JSStringRAII param(JS_ToCString(ctx, argv[0]));
  return JS_NewBool(ctx, obj->SaveToFile(std::filesystem::path(param)));
)
DEF_FUNC_WITH_ARGC(Config, get_bool, 1,
  JSStringRAII param(JS_ToCString(ctx, argv[0]));
  bool value = false;
  bool success = obj->GetBool(param, &value);
  return success ? JS_NewBool(ctx, value) : JS_NULL;
)
DEF_FUNC_WITH_ARGC(Config, get_int, 1,
  JSStringRAII param(JS_ToCString(ctx, argv[0]));
  int value = 0;
  bool success = obj->GetInt(param, &value);
  return success ? JS_NewInt32(ctx, value) : JS_NULL;
)
DEF_FUNC_WITH_ARGC(Config, get_double, 1,
  JSStringRAII param(JS_ToCString(ctx, argv[0]));
  double value = 0.0;
  bool success = obj->GetDouble(param, &value);
  return success ? JS_NewFloat64(ctx, value) : JS_NULL;
)
DEF_FUNC_WITH_ARGC(Config, get_string, 1,
  JSStringRAII param(JS_ToCString(ctx, argv[0]));
  std::string value;
  bool success = obj->GetString(param, &value);
  return success ? JS_NewString(ctx, value.c_str()) : JS_NULL;
)
DEF_FUNC_WITH_ARGC(Config, get_list, 1,
  JSStringRAII param(JS_ToCString(ctx, argv[0]));
  auto list = obj->GetList(param);
  return QjsConfigList::Wrap(ctx, list);
)

DEF_FUNC_WITH_ARGC(Config, set_bool, 2,
  JSStringRAII firstParam(JS_ToCString(ctx, argv[0]));
  bool value = JS_ToBool(ctx, argv[1]);
  bool success = obj->SetBool(firstParam, value);
  return JS_NewBool(ctx, success);
)
DEF_FUNC_WITH_ARGC(Config, set_int, 2,
  JSStringRAII firstParam(JS_ToCString(ctx, argv[0]));
  int32_t value;
  JS_ToInt32(ctx, &value, argv[1]);
  bool success = obj->SetInt(firstParam, value);
  return JS_NewBool(ctx, success);
)
DEF_FUNC_WITH_ARGC(Config, set_double, 2,
  JSStringRAII firstParam(JS_ToCString(ctx, argv[0]));
  double value;
  JS_ToFloat64(ctx, &value, argv[1]);
  bool success = obj->SetDouble(firstParam, value);
  return JS_NewBool(ctx, success);
)
DEF_FUNC_WITH_ARGC(Config, set_string, 2,
  JSStringRAII firstParam(JS_ToCString(ctx, argv[0]));
  JSStringRAII value(JS_ToCString(ctx, argv[1]));
  bool success = obj->SetString(firstParam, value);
  return JS_NewBool(ctx, success);
)

DEFINE_JS_CLASS_WITH_RAW_POINTER(
  Config,
  NO_CONSTRUCTOR_TO_REGISTER,
  NO_PROPERTY_TO_REGISTER,
  DEFINE_FUNCTIONS(
    JS_CFUNC_DEF("loadFromFile", 1, load_from_file),
    JS_CFUNC_DEF("saveToFile", 1, save_to_file),
    JS_CFUNC_DEF("getBool", 1, get_bool),
    JS_CFUNC_DEF("getInt", 1, get_int),
    JS_CFUNC_DEF("getDouble", 1, get_double),
    JS_CFUNC_DEF("getString", 1, get_string),
    JS_CFUNC_DEF("getList", 1, get_list),
    JS_CFUNC_DEF("setBool", 2, set_bool),
    JS_CFUNC_DEF("setInt", 2, set_int),
    JS_CFUNC_DEF("setDouble", 2, set_double),
    JS_CFUNC_DEF("setString", 2, set_string),
  )
)

} // namespace rime
