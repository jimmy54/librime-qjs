#include "qjs_commit_history.h"  // IWYU pragma: keep

#include "qjs_commit_record.h"

namespace rime {

DEFINE_FUNCTION_ARGC(CommitHistory, push, 2, {
  JSStringRAII type = JS_ToCString(ctx, argv[0]);
  JSStringRAII text = JS_ToCString(ctx, argv[1]);
  obj->Push(CommitRecord(type, text));
  return JS_UNDEFINED;
})

DEFINE_GETTER(CommitHistory,
              last,
              obj->empty() ? JS_UNDEFINED : QjsCommitRecord::wrap(ctx, &obj->back()));

DEFINE_GETTER(CommitHistory, repr, jsNewStringFromStd(ctx, obj->repr()));

DEFINE_JS_CLASS_WITH_RAW_POINTER(CommitHistory,
                                 NO_CONSTRUCTOR_TO_REGISTER,
                                 NO_PROPERTY_TO_REGISTER,
                                 DEFINE_GETTERS(last, repr),
                                 DEFINE_FUNCTIONS(JS_CFUNC_DEF("push", 2, push)))

}  // namespace rime
