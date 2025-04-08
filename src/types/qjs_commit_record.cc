#include "qjs_commit_record.h"  // IWYU pragma: keep

namespace rime {

DEFINE_GETTER(CommitRecord, type, jsNewStringFromStd(ctx, obj->type))
DEFINE_GETTER(CommitRecord, text, jsNewStringFromStd(ctx, obj->text))

DEFINE_STRING_SETTER(CommitRecord, text, obj->text = str)
DEFINE_STRING_SETTER(CommitRecord, type, obj->type = str)

DEFINE_JS_CLASS_WITH_RAW_POINTER(CommitRecord,
                                 NO_CONSTRUCTOR_TO_REGISTER,
                                 DEFINE_PROPERTIES(text, type),
                                 NO_GETTER_TO_REGISTER,
                                 NO_FUNCTION_TO_REGISTER)

}  // namespace rime
