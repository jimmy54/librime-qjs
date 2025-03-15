#include "qjs_config_map.h"  // IWYU pragma: keep

#include "qjs_config_item.h"
#include "qjs_config_value.h"

namespace rime {

DEFINE_FUNCTION(ConfigMap, getType, return JS_NewString(ctx, "map");)
DEFINE_FUNCTION_ARGC(ConfigMap, hasKey, 1, {
  JSStringRAII param(JS_ToCString(ctx, argv[0]));
  return JS_NewBool(ctx, obj->HasKey(param));
})
DEFINE_FUNCTION_ARGC(ConfigMap, getItem, 1, {
  JSStringRAII param(JS_ToCString(ctx, argv[0]));
  auto value = obj->Get(param);
  if (!value)
    return JS_NULL;
  return QjsConfigItem::wrap(ctx, value);
})
DEFINE_FUNCTION_ARGC(ConfigMap, getValue, 1, {
  JSStringRAII param(JS_ToCString(ctx, argv[0]));
  auto value = obj->GetValue(param);
  if (!value)
    return JS_NULL;
  return QjsConfigValue::wrap(ctx, value);
})
DEFINE_FUNCTION_ARGC(ConfigMap, setItem, 2, {
  JSStringRAII param(JS_ToCString(ctx, argv[0]));
  if (auto item = QjsConfigItem::unwrap(ctx, argv[1])) {
    obj->Set(param, item);
  }
  return JS_UNDEFINED;
})

DEFINE_JS_CLASS_WITH_SHARED_POINTER(ConfigMap,
                                    NO_CONSTRUCTOR_TO_REGISTER,
                                    NO_PROPERTY_TO_REGISTER,
                                    NO_GETTER_TO_REGISTER,
                                    DEFINE_FUNCTIONS(JS_CFUNC_DEF("getType", 0, getType),
                                                     JS_CFUNC_DEF("hasKey", 1, hasKey),
                                                     JS_CFUNC_DEF("getItem", 1, getItem),
                                                     JS_CFUNC_DEF("getValue", 1, getValue),
                                                     JS_CFUNC_DEF("setItem", 2, setItem), ))
}  // namespace rime
