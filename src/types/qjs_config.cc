#include "qjs_config.h"
#include <filesystem>

namespace rime {

DEFINE_JS_CLASS_WITH_RAW_POINTER(Config,
  NO_PROPERTY_TO_REGISTER,
  DEFINE_FUNCTIONS(
    JS_CFUNC_DEF("loadFromFile", 1, load_from_file),
    JS_CFUNC_DEF("saveToFile", 1, save_to_file),
    JS_CFUNC_DEF("getBool", 1, get_bool),
    JS_CFUNC_DEF("getInt", 1, get_int),
    JS_CFUNC_DEF("getDouble", 1, get_double),
    JS_CFUNC_DEF("getString", 1, get_string),
    JS_CFUNC_DEF("setBool", 2, set_bool),
    JS_CFUNC_DEF("setInt", 2, set_int),
    JS_CFUNC_DEF("setDouble", 2, set_double),
    JS_CFUNC_DEF("setString", 2, set_string),
  )
)

DEF_FUNC_WITH_SINGLE_STRING_PARAM(Config, load_from_file,
  return JS_NewBool(ctx, obj->LoadFromFile(path(param)));
)
DEF_FUNC_WITH_SINGLE_STRING_PARAM(Config, save_to_file,
  return JS_NewBool(ctx, obj->SaveToFile(path(param)));
)
DEF_FUNC_WITH_SINGLE_STRING_PARAM(Config, get_bool,
  bool value = false;
  bool success = obj->GetBool(param, &value);
  return success ? JS_NewBool(ctx, value) : JS_NULL;
)
DEF_FUNC_WITH_SINGLE_STRING_PARAM(Config, get_int,
  int value = 0;
  bool success = obj->GetInt(param, &value);
  return success ? JS_NewBool(ctx, value) : JS_NULL;
)
DEF_FUNC_WITH_SINGLE_STRING_PARAM(Config, get_double,
  double value = 0.0;
  bool success = obj->GetDouble(param, &value);
  return success ? JS_NewFloat64(ctx, value) : JS_NULL;
)
DEF_FUNC_WITH_SINGLE_STRING_PARAM(Config, get_string,
  string value;
  bool success = obj->GetString(param, &value);
  return success ? JS_NewString(ctx, value.c_str()) : JS_NULL;
)

DEF_FUNC_WITH_STRING_AND_ANOTHER(Config, set_bool,
  bool value = JS_ToBool(ctx, argv[1]);
  bool success = obj->SetBool(firstParam, value);
  return JS_NewBool(ctx, success);
)
DEF_FUNC_WITH_STRING_AND_ANOTHER(Config, set_int,
  int32_t value;
  JS_ToInt32(ctx, &value, argv[1]);
  bool success = obj->SetInt(firstParam, value);
  return JS_NewBool(ctx, success);
)
DEF_FUNC_WITH_STRING_AND_ANOTHER(Config, set_double,
  double value;
  JS_ToFloat64(ctx, &value, argv[1]);
  bool success = obj->SetDouble(firstParam, value);
  return JS_NewBool(ctx, success);
)
DEF_FUNC_WITH_STRING_AND_ANOTHER(Config, set_string,
  JSStringRAII value(JS_ToCString(ctx, argv[1]));
  bool success = obj->SetString(firstParam, value);
  return JS_NewBool(ctx, success);
)

} // namespace rime
