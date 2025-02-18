#ifndef RIME_QJS_CONFIG_ITEM_H_
#define RIME_QJS_CONFIG_ITEM_H_

#include "qjs_macros.h"
#include "qjs_type_registry.h"
#include <rime/config/config_types.h>

DECLARE_JS_CLASS_WITH_SHARED_POINTER(ConfigItem,
  NO_PROPERTY_TO_DECLARE,
  DECLARE_FUNCTIONS(get_type)
)

#endif // RIME_QJS_CONFIG_ITEM_H_
