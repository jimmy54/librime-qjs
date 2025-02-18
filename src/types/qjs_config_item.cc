#include "qjs_config_item.h"

namespace rime {

DEFINE_JS_CLASS_WITH_SHARED_POINTER(ConfigItem,
  NO_PROPERTY_TO_REGISTER,
  DEFINE_FUNCTIONS(
    JS_CFUNC_DEF("getType", 0, get_type)
  )
)

DEF_FUNC(ConfigItem, get_type,
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

} // namespace rime
