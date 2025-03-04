#include "qjs_preedit.h"

namespace rime {

DEFINE_JS_CLASS_WITH_RAW_POINTER(
  Preedit,
  NO_CONSTRUCTOR_TO_REGISTER,
  DEFINE_PROPERTIES(text, caretPos, selectStart, selectEnd),
  NO_FUNCTION_TO_REGISTER
)

DEFINE_GETTER(Preedit, text, js_new_string_from_std(ctx, obj->text))
DEFINE_GETTER(Preedit, caretPos, JS_NewInt64(ctx, obj->caret_pos))
DEFINE_GETTER(Preedit, selectStart, JS_NewInt64(ctx, obj->sel_start))
DEFINE_GETTER(Preedit, selectEnd, JS_NewInt64(ctx, obj->sel_end))

DEFINE_STRING_SETTER(Preedit, text,
  obj->text = str;
)

DEFINE_SETTER(Preedit, caretPos, int64_t, JS_ToInt64, obj->caret_pos = value)
DEFINE_SETTER(Preedit, selectStart, int64_t, JS_ToInt64, obj->sel_start = value)
DEFINE_SETTER(Preedit, selectEnd, int64_t, JS_ToInt64, obj->sel_end = value)

} // namespace rime
