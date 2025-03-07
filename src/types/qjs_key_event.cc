#include "qjs_key_event.h" // IWYU pragma: keep

namespace rime {

DEFINE_GETTER(KeyEvent, shift, JS_NewBool(ctx, obj->shift()))
DEFINE_GETTER(KeyEvent, ctrl, JS_NewBool(ctx, obj->ctrl()))
DEFINE_GETTER(KeyEvent, alt, JS_NewBool(ctx, obj->alt()))
DEFINE_GETTER(KeyEvent, release, JS_NewBool(ctx, obj->release()))
DEFINE_GETTER(KeyEvent, repr, js_new_string_from_std(ctx, obj->repr()))

DEFINE_JS_CLASS_WITH_RAW_POINTER(
  KeyEvent,
  NO_CONSTRUCTOR_TO_REGISTER,
  NO_PROPERTY_TO_REGISTER,
  DEFINE_GETTERS(shift, ctrl, alt, release, repr),
  NO_FUNCTION_TO_REGISTER
)

} // namespace rime
