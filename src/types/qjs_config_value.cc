#include "qjs_config_value.h"  // IWYU pragma: keep

namespace rime {

DEFINE_FUNCTION(ConfigValue, get_type, return JS_NewString(ctx, "scalar");)
DEFINE_FUNCTION(ConfigValue, get_bool, {
  bool value = false;
  bool success = obj->GetBool(&value);
  return success ? JS_NewBool(ctx, value) : JS_NULL;
})
DEFINE_FUNCTION(ConfigValue, get_int, {
  int value = 0;
  bool success = obj->GetInt(&value);
  return success ? JS_NewInt32(ctx, value) : JS_NULL;
})
DEFINE_FUNCTION(ConfigValue, get_double, {
  double value = 0.0;
  bool success = obj->GetDouble(&value);
  return success ? JS_NewFloat64(ctx, value) : JS_NULL;
})
DEFINE_FUNCTION(ConfigValue, get_string, {
  string value;
  bool success = obj->GetString(&value);
  return success ? JS_NewString(ctx, value.c_str()) : JS_NULL;
})

DEFINE_JS_CLASS_WITH_SHARED_POINTER(ConfigValue,
                                    NO_CONSTRUCTOR_TO_REGISTER,
                                    NO_PROPERTY_TO_REGISTER,
                                    NO_GETTER_TO_REGISTER,
                                    DEFINE_FUNCTIONS(JS_CFUNC_DEF("getType", 0, get_type),
                                                     JS_CFUNC_DEF("getBool", 0, get_bool),
                                                     JS_CFUNC_DEF("getInt", 0, get_int),
                                                     JS_CFUNC_DEF("getDouble", 1, get_double),
                                                     JS_CFUNC_DEF("getString", 1, get_string), ))

}  // namespace rime
