#include "qjs_config_item.h"

namespace rime {

DEFINE_FUNCTION(ConfigItem, get_type,
  const char* strType;
  switch (obj->type()) {
    case ConfigItem::kNull:
      strType = "null";
      break;
    case ConfigItem::kScalar:
      strType = "scalar";
      break;
    case ConfigItem::kList:
      strType = "list";
      break;
    case ConfigItem::kMap:
      strType = "map";
      break;
    default:
      strType = "unknown";
  }
  return JS_NewString(ctx, strType);
)

DEFINE_JS_CLASS_WITH_SHARED_POINTER(
  ConfigItem,
  NO_CONSTRUCTOR_TO_REGISTER,
  NO_PROPERTY_TO_REGISTER,
  NO_GETTER_TO_REGISTER,
  DEFINE_FUNCTIONS(
    JS_CFUNC_DEF("getType", 0, get_type)
  )
)

} // namespace rime
