#include "qjs_config_list.h" // IWYU pragma: keep
#include "qjs_config_item.h"
#include "qjs_config_value.h"

namespace rime {

DEFINE_FUNCTION(ConfigList, get_type,
  return JS_NewString(ctx, "list");
)
DEFINE_FUNCTION(ConfigList, size,
  return JS_NewInt32(ctx, obj->size());
)
DEFINE_FUNCTION_ARGC(ConfigList, get_item_at, 1,
  int index;
  JS_ToInt32(ctx, &index, argv[0]);
  if (index < 0 || size_t(index) >= obj->size()) return JS_NULL;

  auto item = obj->GetAt(index);
  if (!item) return JS_NULL;
  return QjsConfigItem::Wrap(ctx, item);
)
DEFINE_FUNCTION_ARGC(ConfigList, get_value_at, 1,
  int32_t index;
  JS_ToInt32(ctx, &index, argv[0]);
  if (index < 0 || size_t(index) >= obj->size()) return JS_NULL;

  auto value = obj->GetValueAt(index);
  if (!value) return JS_NULL;
  return QjsConfigValue::Wrap(ctx, value);
)
DEFINE_FUNCTION_ARGC(ConfigList, push_back, 1,
  if (auto item = QjsConfigItem::Unwrap(ctx, argv[0])) {
    obj->Append(item);
  }
  return JS_UNDEFINED;
)
DEFINE_FUNCTION(ConfigList, clear,
  obj->Clear();
  return JS_UNDEFINED;
)

DEFINE_JS_CLASS_WITH_SHARED_POINTER(
  ConfigList,
  NO_CONSTRUCTOR_TO_REGISTER,
  NO_PROPERTY_TO_REGISTER,
  NO_GETTER_TO_REGISTER,
  DEFINE_FUNCTIONS(
    JS_CFUNC_DEF("getType", 0, get_type),
    JS_CFUNC_DEF("getSize", 0, size),
    JS_CFUNC_DEF("getItemAt", 1, get_item_at),
    JS_CFUNC_DEF("getValueAt", 1, get_value_at),
    JS_CFUNC_DEF("pushBack", 1, push_back),
    JS_CFUNC_DEF("clear", 0, clear)
  )
)
} // namespace rime
