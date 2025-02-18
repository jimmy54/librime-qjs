#ifndef RIME_QJS_CONFIG_VALUE_H_
#define RIME_QJS_CONFIG_VALUE_H_

#include "qjs_type_registry.h"
#include "qjs_macros.h"
#include <rime/config.h>

DECLARE_JS_CLASS_WITH_SHARED_POINTER(ConfigValue,
  NO_PROPERTY_TO_DECLARE,
  DECLARE_FUNCTIONS(get_type, get_bool, get_int, get_double, get_string)
)

#endif // RIME_QJS_CONFIG_VALUE_H_
