#include "qjs_config_value.h"  // IWYU pragma: keep

namespace rime {

DEFINE_FUNCTION(ConfigValue, getType, return JS_NewString(ctx, "scalar");)
DEFINE_FUNCTION(ConfigValue, getBool, {
  bool value = false;
  bool success = obj->GetBool(&value);
  return success ? JS_NewBool(ctx, value) : JS_NULL;
})
DEFINE_FUNCTION(ConfigValue, getInt, {
  int value = 0;
  bool success = obj->GetInt(&value);
  return success ? JS_NewInt32(ctx, value) : JS_NULL;
})
DEFINE_FUNCTION(ConfigValue, getDouble, {
  double value = 0.0;
  bool success = obj->GetDouble(&value);
  return success ? JS_NewFloat64(ctx, value) : JS_NULL;
})
DEFINE_FUNCTION(ConfigValue, getString, {
  string value;
  bool success = obj->GetString(&value);
  return success ? JS_NewString(ctx, value.c_str()) : JS_NULL;
})

DEFINE_JS_CLASS_WITH_SHARED_POINTER(ConfigValue,
                                    NO_CONSTRUCTOR_TO_REGISTER,
                                    NO_PROPERTY_TO_REGISTER,
                                    NO_GETTER_TO_REGISTER,
                                    DEFINE_FUNCTIONS(JS_CFUNC_DEF("getType", 0, getType),
                                                     JS_CFUNC_DEF("getBool", 0, getBool),
                                                     JS_CFUNC_DEF("getInt", 0, getInt),
                                                     JS_CFUNC_DEF("getDouble", 1, getDouble),
                                                     JS_CFUNC_DEF("getString", 1, getString), ))

}  // namespace rime
