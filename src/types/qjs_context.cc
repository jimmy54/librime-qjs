#include "qjs_context.h"
#include <memory>

#include "qjs_commit_history.h"
#include "qjs_notifier.h"
#include "qjs_preedit.h"
#include "qjs_segment.h"

namespace rime {

DEFINE_GETTER(Context, input, jsNewStringFromStd(ctx, obj->input()))
DEFINE_GETTER(Context, caretPos, JS_NewInt32(ctx, obj->caret_pos()))

DEFINE_STRING_SETTER(Context, input, obj->set_input(str);)
DEFINE_SETTER(Context, caretPos, int32_t, JS_ToInt32, obj->set_caret_pos(value))

DEFINE_GETTER(Context, preedit, QjsPreedit::wrap(ctx, std::make_shared<Preedit>(obj->GetPreedit())))
DEFINE_GETTER(Context, commitNotifier, QjsNotifier::wrap(ctx, &obj->commit_notifier()))
DEFINE_GETTER(Context, selectNotifier, QjsNotifier::wrap(ctx, &obj->select_notifier()))
DEFINE_GETTER(Context, updateNotifier, QjsNotifier::wrap(ctx, &obj->update_notifier()))
DEFINE_GETTER(Context, deleteNotifier, QjsNotifier::wrap(ctx, &obj->delete_notifier()))

DEFINE_GETTER(Context, commitHistory, QjsCommitHistory::wrap(ctx, &obj->commit_history()))

static JSValue get_lastSegment(JSContext* ctx, JSValueConst thisVal) {
  if (auto* obj = QjsContext::unwrap(ctx, thisVal)) {
    if (obj->composition().empty()) {
      // The composition could be empty when there is not a menu listing.
      // In the javascript plugins, it should check `context.hasMenu()` before fetching the segment.
      return JS_NULL;
    }

    // must be set as reference [Segment&] here, otherwise fetching its prompt would crash the program
    Segment& segment = obj->composition().back();
    return QjsSegment::wrap(ctx, &segment);
  }
  return JS_UNDEFINED;
}

DEFINE_FUNCTION(Context, commit, {
  obj->Commit();
  return JS_UNDEFINED;
})

DEFINE_FUNCTION(Context, getCommitText, return JS_NewString(ctx, obj->GetCommitText().c_str());)

DEFINE_FUNCTION(Context, clear, {
  obj->Clear();
  return JS_UNDEFINED;
})

DEFINE_FUNCTION(Context, hasMenu, return JS_NewBool(ctx, obj->HasMenu());)

DEFINE_JS_CLASS_WITH_RAW_POINTER(Context,
                                 NO_CONSTRUCTOR_TO_REGISTER,
                                 DEFINE_PROPERTIES(input, caretPos),
                                 DEFINE_GETTERS(preedit,
                                                lastSegment,
                                                commitHistory,
                                                commitNotifier,
                                                selectNotifier,
                                                updateNotifier,
                                                deleteNotifier),
                                 DEFINE_FUNCTIONS(
                                     // Input methods
                                     JS_CFUNC_DEF("commit", 0, commit),
                                     JS_CFUNC_DEF("getCommitText", 0, getCommitText),
                                     JS_CFUNC_DEF("clear", 0, clear),

                                     // Menu methods
                                     JS_CFUNC_DEF("hasMenu", 0, hasMenu), ))

}  // namespace rime
