#include "qjs_key_event.h"

namespace rime {

DEFINE_JS_CLASS_WITH_RAW_POINTER(
  KeyEvent,
  NO_CONSTRUCTOR_TO_REGISTER,
  DEFINE_GETTERS(shift, ctrl, alt, release, repr),
  NO_FUNCTION_TO_REGISTER
)

DEFINE_GETTER(KeyEvent, shift, bool, JS_NewBool)
DEFINE_GETTER(KeyEvent, ctrl, bool, JS_NewBool)
DEFINE_GETTER(KeyEvent, alt, bool, JS_NewBool)
DEFINE_GETTER(KeyEvent, release, bool, JS_NewBool)
DEFINE_GETTER(KeyEvent, repr, const string&, js_new_string_from_std)

} // namespace rime
