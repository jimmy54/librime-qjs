#ifndef RIME_QJS_CONFIG_MAP_H_
#define RIME_QJS_CONFIG_MAP_H_

#include "qjs_macros.h"
#include "qjs_type_registry.h"
#include <rime/config.h>

DECLARE_JS_CLASS_WITH_SHARED_POINTER(ConfigMap,
  NO_PROPERTY_TO_DECLARE,
  DECLARE_FUNCTIONS(getType, hasKey, getItem, getValue, setItem)
)

#endif // RIME_QJS_CONFIG_MAP_H_
