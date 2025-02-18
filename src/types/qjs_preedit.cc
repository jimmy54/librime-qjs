#include "qjs_preedit.h"

namespace rime {

DEFINE_JS_CLASS_WITH_RAW_POINTER(
  Preedit,
  NO_CONSTRUCTOR_TO_REGISTER,
  DEFINE_PROPERTIES(text, caretPos, selectStart, selectEnd),
  NO_FUNCTION_TO_REGISTER
)

DEFINE_GETTER_3(Preedit, text, text, const string&, js_new_string_from_std)
DEFINE_GETTER_3(Preedit, caretPos, caret_pos, int64_t, JS_NewInt64)
DEFINE_GETTER_3(Preedit, selectStart, sel_start, int64_t, JS_NewInt64)
DEFINE_GETTER_3(Preedit, selectEnd, sel_end, int64_t, JS_NewInt64)

DEFINE_STRING_SETTER(Preedit, text,
  obj->text = str;
)

DEFINE_NUMERIC_SETTER_3(Preedit, caretPos, caret_pos, int64_t, JS_ToInt64)
DEFINE_NUMERIC_SETTER_3(Preedit, selectStart, sel_start, int64_t, JS_ToInt64)
DEFINE_NUMERIC_SETTER_3(Preedit, selectEnd, sel_end, int64_t, JS_ToInt64)

} // namespace rime
