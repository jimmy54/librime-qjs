#include "qjs_config_list.h"  // IWYU pragma: keep

#include "qjs_config_item.h"
#include "qjs_config_value.h"

namespace rime {

DEFINE_FUNCTION(ConfigList, getType, return JS_NewString(ctx, "list");)
DEFINE_FUNCTION(ConfigList, size, return JS_NewInt32(ctx, obj->size());)
DEFINE_FUNCTION_ARGC(ConfigList, getItemAt, 1, {
  int index;
  JS_ToInt32(ctx, &index, argv[0]);
  if (index < 0 || size_t(index) >= obj->size())
    return JS_NULL;

  auto item = obj->GetAt(index);
  if (!item)
    return JS_NULL;
  return QjsConfigItem::wrap(ctx, item);
})

DEFINE_FUNCTION_ARGC(ConfigList, getValueAt, 1, {
  int32_t index;
  JS_ToInt32(ctx, &index, argv[0]);
  if (index < 0 || size_t(index) >= obj->size())
    return JS_NULL;

  auto value = obj->GetValueAt(index);
  if (!value)
    return JS_NULL;
  return QjsConfigValue::wrap(ctx, value);
})

DEFINE_FUNCTION_ARGC(ConfigList, pushBack, 1, {
  if (auto item = QjsConfigItem::unwrap(ctx, argv[0])) {
    obj->Append(item);
  }
  return JS_UNDEFINED;
})

DEFINE_FUNCTION(ConfigList, clear, obj->Clear(); return JS_UNDEFINED;)

DEFINE_JS_CLASS_WITH_SHARED_POINTER(ConfigList,
                                    NO_CONSTRUCTOR_TO_REGISTER,
                                    NO_PROPERTY_TO_REGISTER,
                                    NO_GETTER_TO_REGISTER,
                                    DEFINE_FUNCTIONS(JS_CFUNC_DEF("getType", 0, getType),
                                                     JS_CFUNC_DEF("getSize", 0, size),
                                                     JS_CFUNC_DEF("getItemAt", 1, getItemAt),
                                                     JS_CFUNC_DEF("getValueAt", 1, getValueAt),
                                                     JS_CFUNC_DEF("pushBack", 1, pushBack),
                                                     JS_CFUNC_DEF("clear", 0, clear)))
}  // namespace rime
