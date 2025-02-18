#ifndef RIME_QJS_PREEDIT_H_
#define RIME_QJS_PREEDIT_H_

#include "qjs_macros.h"
#include "qjs_type_registry.h"
#include <rime/composition.h>

DECLARE_JS_CLASS_WITH_RAW_POINTER(Preedit,
  DECLARE_PROPERTIES(text, caretPos, selectStart, selectEnd),
  // no functions to declare
)

#endif // RIME_QJS_CRIME_QJS_PREEDIT_H_ONFIG_H_
