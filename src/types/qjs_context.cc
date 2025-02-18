#include "qjs_context.h"

namespace rime {

DEFINE_JS_CLASS_WITH_RAW_POINTER(Context,
  DEFINE_PROPERTIES(input, caretPos),
  DEFINE_FUNCTIONS(
    // Input methods
    JS_CFUNC_DEF("commit", 0, commit),
    JS_CFUNC_DEF("getCommitText", 0, get_commit_text),
    JS_CFUNC_DEF("clear", 0, clear),
    JS_CFUNC_DEF("pushInput", 1, push_input),
    JS_CFUNC_DEF("popInput", 1, pop_input),

    // Menu methods
    JS_CFUNC_DEF("hasMenu", 0, has_menu),

    // Segment methods
    JS_CFUNC_DEF("clearPreviousSegment", 0, clear_previous_segment),
    JS_CFUNC_DEF("reopenPreviousSegment", 0, reopen_previous_segment),
    JS_CFUNC_DEF("clearNonConfirmedComposition", 0, clear_non_confirmed_composition),
    JS_CFUNC_DEF("refreshNonConfirmedComposition", 0, refresh_non_confirmed_composition),

    // Option methods
    JS_CFUNC_DEF("setOption", 2, set_option),
    JS_CFUNC_DEF("getOption", 1, get_option),
    JS_CFUNC_DEF("clearTransientOptions", 0, clear_transient_options),

    // Property methods
    JS_CFUNC_DEF("setProperty", 2, set_property),
    JS_CFUNC_DEF("getProperty", 1, get_property),
  )
)


DEFINE_GETTER(Context, input, const string&, js_new_string_from_std)
DEFINE_GETTER_2(Context, caretPos, caret_pos, int, JS_NewInt32)

DEFINE_STRING_SETTER(Context, input,
  obj->set_input(str);
)
DEFINE_NUMERIC_SETTER_2(Context, caretPos, set_caret_pos, int32_t, JS_ToInt32)

DEF_FUNC(Context, commit,
  obj->Commit();
  return JS_UNDEFINED;
)

DEF_FUNC(Context, get_commit_text,
  return JS_NewString(ctx, obj->GetCommitText().c_str());
)

DEF_FUNC(Context, clear,
  obj->Clear();
  return JS_UNDEFINED;
)

DEF_FUNC_WITH_SINGLE_STRING_PARAM(Context, push_input,
  bool result = obj->PushInput(param);
  return JS_NewBool(ctx, result);
)

DEF_FUNC(Context, pop_input,
  int32_t length;
  JS_ToInt32(ctx, &length, argv[0]);
  bool success = obj->PopInput(length);
  return JS_NewBool(ctx, success);
)

DEF_FUNC(Context, has_menu,
  return JS_NewBool(ctx, obj->HasMenu());
)

DEF_FUNC(Context, clear_previous_segment,
  obj->ClearPreviousSegment();
  return JS_UNDEFINED;
)

DEF_FUNC(Context, reopen_previous_segment,
  obj->ReopenPreviousSegment();
  return JS_UNDEFINED;
)

DEF_FUNC(Context, clear_non_confirmed_composition,
  obj->ClearNonConfirmedComposition();
  return JS_UNDEFINED;
)

DEF_FUNC(Context, refresh_non_confirmed_composition,
  obj->RefreshNonConfirmedComposition();
  return JS_UNDEFINED;
)

DEF_FUNC_WITH_STRING_AND_ANOTHER(Context, set_option,
  bool value = JS_ToBool(ctx, argv[1]);
  obj->set_option(firstParam, value);
  return JS_TRUE;
)

DEF_FUNC_WITH_SINGLE_STRING_PARAM(Context, get_option,
  bool result = obj->get_option(param);
  return JS_NewBool(ctx, result);
)

DEF_FUNC_WITH_STRING_AND_ANOTHER(Context, set_property,
  const char* value = JS_ToCString(ctx, argv[1]);
  obj->set_property(firstParam, value);
  JS_FreeCString(ctx, value);
  return JS_TRUE;
)

DEF_FUNC_WITH_SINGLE_STRING_PARAM(Context, get_property,
  string result = obj->get_property(param);
  return JS_NewString(ctx, result.c_str());
)

DEF_FUNC(Context, clear_transient_options,
  obj->ClearTransientOptions();
  return JS_UNDEFINED;
)

} // namespace rime
