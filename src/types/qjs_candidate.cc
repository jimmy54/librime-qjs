#include <rime/gear/translator_commons.h>
#include "qjs_candidate.h"
#include <memory>

namespace rime {

static JSValue makeCandidate(JSContext* ctx, JSValueConst newTarget, int argc, JSValueConst* argv) {
  if (argc < 5) {
    return JS_ThrowSyntaxError(ctx, "new Candidate(...) expects 5 or 6 arguments");
  }

  auto obj = std::make_shared<SimpleCandidate>();
  STRING_ASSIGNMENT_FROM_JS_ARGV(type, 0)
  NUMERIC_ASSIGNMENT_FROM_JS_ARGV(start, 1, int64_t, JS_ToInt64)
  NUMERIC_ASSIGNMENT_FROM_JS_ARGV(end, 2, int64_t, JS_ToInt64)
  STRING_ASSIGNMENT_FROM_JS_ARGV(text, 3)
  STRING_ASSIGNMENT_FROM_JS_ARGV(comment, 4)
  if (argc >= 6) {
    NUMERIC_ASSIGNMENT_FROM_JS_ARGV(quality, 5, int32_t, JS_ToInt32)
  }

  return QjsCandidate::Wrap(ctx, obj);
}

DEFINE_GETTER(Candidate, text, js_new_string_from_std(ctx, obj->text()))
DEFINE_GETTER(Candidate, comment, js_new_string_from_std(ctx, obj->comment()))
DEFINE_GETTER(Candidate, type, js_new_string_from_std(ctx, obj->type()))
DEFINE_GETTER(Candidate, start, JS_NewInt64(ctx, obj->start()))
DEFINE_GETTER(Candidate, end, JS_NewInt64(ctx, obj->end()))
DEFINE_GETTER(Candidate, quality, JS_NewInt32(ctx, obj->quality()))
DEFINE_GETTER(Candidate, preedit, js_new_string_from_std(ctx, obj->preedit()))

DEFINE_STRING_SETTER(Candidate, text,
  if (auto simpleCandidate = dynamic_cast<SimpleCandidate*>(obj.get())) {
    simpleCandidate->set_text(str);
  }
)

DEFINE_STRING_SETTER(Candidate, comment,
  if (auto simpleCandidate = dynamic_cast<SimpleCandidate*>(obj.get())) {
    simpleCandidate->set_comment(str);
  } else if (auto phrase = dynamic_cast<Phrase*>(obj.get())) {
    phrase->set_comment(str);
  }
)

DEFINE_STRING_SETTER(Candidate, type,
  obj->set_type(str);
)

DEFINE_SETTER(Candidate, start, int64_t, JS_ToInt64, obj->set_start(value))
DEFINE_SETTER(Candidate, end, int64_t, JS_ToInt64, obj->set_end(value))
DEFINE_SETTER(Candidate, quality, int32_t, JS_ToInt32, obj->set_quality(value))

DEFINE_STRING_SETTER(Candidate, preedit,
  if (auto simpleCandidate = dynamic_cast<SimpleCandidate*>(obj.get())) {
    simpleCandidate->set_preedit(str);
  } else if (auto phrase = dynamic_cast<Phrase*>(obj.get())) {
    phrase->set_preedit(str);
  }
)

DEFINE_JS_CLASS_WITH_SHARED_POINTER(
  Candidate,
  DEFINE_CONSTRUCTOR(Candidate, makeCandidate, 5),
  DEFINE_PROPERTIES(text, comment, type, start, end, quality, preedit),
  NO_GETTER_TO_REGISTER,
  NO_FUNCTION_TO_REGISTER
)

} // namespace rime
