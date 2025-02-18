#ifndef RIME_QJS_CONFIG_LIST_H_
#define RIME_QJS_CONFIG_LIST_H_

#include "qjs_macros.h"
#include "qjs_type_registry.h"
#include <rime/config.h>

DECLARE_JS_CLASS_WITH_SHARED_POINTER(ConfigList,
  NO_PROPERTY_TO_DECLARE,
  DECLARE_FUNCTIONS(get_type, size, get_item_at, get_value_at, push_back, clear)
)

#endif // RIME_QJS_CONFIG_LIST_H_
