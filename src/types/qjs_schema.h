#ifndef RIME_QJS_SCHEMA_H_
#define RIME_QJS_SCHEMA_H_

#include "qjs_macros.h"
#include "qjs_type_registry.h"
#include <rime/schema.h>

DECLARE_JS_CLASS_WITH_RAW_POINTER(Schema,
  DECLARE_GETTERS(id, name, config, pageSize, selectKeys),
  // no functions to declare
)

#endif // RIME_QJS_SCHEMA_H_
