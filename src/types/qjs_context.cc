#include "qjs_context.h"
#include "qjs_preedit.h"
#include "qjs_segment.h"

namespace rime {

DEFINE_GETTER(Context, input, jsNewStringFromStd(ctx, obj->input()))
DEFINE_GETTER(Context, caretPos, JS_NewInt32(ctx, obj->caret_pos()))

DEFINE_STRING_SETTER(Context, input,
  obj->set_input(str);
)
DEFINE_SETTER(Context, caretPos, int32_t, JS_ToInt32, obj->set_caret_pos(value))

static JSValue get_preedit(JSContext* ctx, JSValueConst thisVal) {
  if (auto *obj = QjsContext::Unwrap(ctx, thisVal)) {
    Preedit preedit = obj->GetPreedit();
    return QjsPreedit::Wrap(ctx, &preedit); // <-- incompatible to DEFINE_GETTER
  }
  return JS_UNDEFINED;
}

static JSValue get_lastSegment(JSContext* ctx, JSValueConst thisVal) {
  if (auto *obj = QjsContext::Unwrap(ctx, thisVal)) {
    if (obj->composition().empty()) {
      DLOG(ERROR) << "no segment available in context->composition()";
      return JS_NULL;
    }

    // must be set as reference [Segment&] here, otherwise fetching its prompt would crash the program
    Segment& segment = obj->composition().back();
    return QjsSegment::Wrap(ctx, &segment);

  }
  return JS_UNDEFINED;
}

DEFINE_FUNCTION(Context, commit,
  obj->Commit();
  return JS_UNDEFINED;
)

DEFINE_FUNCTION(Context, get_commit_text,
  return JS_NewString(ctx, obj->GetCommitText().c_str());
)

DEFINE_FUNCTION(Context, clear,
  obj->Clear();
  return JS_UNDEFINED;
)

DEFINE_FUNCTION(Context, has_menu,
  return JS_NewBool(ctx, obj->HasMenu());
)

DEFINE_FUNCTION(Context, is_composing,
  return JS_NewBool(ctx, obj->IsComposing());
)

DEFINE_JS_CLASS_WITH_RAW_POINTER(
  Context,
  NO_CONSTRUCTOR_TO_REGISTER,
  DEFINE_PROPERTIES(input, caretPos),
  DEFINE_GETTERS(preedit, lastSegment),
  DEFINE_FUNCTIONS(
    // Input methods
    JS_CFUNC_DEF("commit", 0, commit),
    JS_CFUNC_DEF("getCommitText", 0, get_commit_text),
    JS_CFUNC_DEF("clear", 0, clear),

    // Menu methods
    JS_CFUNC_DEF("hasMenu", 0, has_menu),
    JS_CFUNC_DEF("isComposing", 0, is_composing),
  )
)


} // namespace rime
