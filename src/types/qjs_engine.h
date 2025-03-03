#ifndef RIME_QJS_ENGINE_H_
#define RIME_QJS_ENGINE_H_

#include "qjs_macros.h"
#include "qjs_type_registry.h"
#include <rime/engine.h>

DECLARE_JS_CLASS_WITH_RAW_POINTER(Engine,
  DECLARE_GETTERS(schema, activeEngine, context),
  DECLARE_FUNCTIONS(processKey, commitText, applySchema)
)

#endif // RIME_QJS_ENGINE_H_
