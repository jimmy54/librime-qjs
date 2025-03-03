#ifndef RIME_QJS_KEY_EVENT_H_
#define RIME_QJS_KEY_EVENT_H_

#include "qjs_macros.h"
#include "qjs_type_registry.h"
#include <rime/key_event.h>

DECLARE_JS_CLASS_WITH_RAW_POINTER(KeyEvent,
  DECLARE_GETTERS(shift, ctrl, alt, release, repr),
)

#endif // RIME_QJS_KEY_EVENT_H_
