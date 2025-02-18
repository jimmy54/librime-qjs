#ifndef RIME_QJS_CONFIG_H_
#define RIME_QJS_CONFIG_H_

#include "qjs_macros.h"
#include "qjs_type_registry.h"
#include <rime/config.h>

DECLARE_JS_CLASS_WITH_RAW_POINTER(Config,
  NO_PROPERTY_TO_DECLARE,
  DECLARE_FUNCTIONS(load_from_file, save_to_file,
    get_bool, get_int, get_double, get_string, get_list,
    set_bool, set_int, set_double, set_string)
)

#endif // RIME_QJS_CONFIG_H_
