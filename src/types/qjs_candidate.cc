#include <rime/gear/translator_commons.h>
#include "qjs_candidate.h"
#include <memory>

namespace rime {

static JSValue makeCandidate(JSContext* ctx, JSValueConst newTarget, int argc, JSValueConst* argv) {
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

DEFINE_JS_CLASS_WITH_SHARED_POINTER(
  Candidate,
  DEFINE_CONSTRUCTOR(Candidate, makeCandidate, 5),
  DEFINE_PROPERTIES(text, comment, type, start, end, quality, preedit),
  NO_FUNCTION_TO_REGISTER
)

DEFINE_GETTER(Candidate, text, const string&, js_new_string_from_std)
DEFINE_GETTER(Candidate, comment, const string&, js_new_string_from_std)
DEFINE_GETTER(Candidate, type, const string&, js_new_string_from_std)
DEFINE_GETTER(Candidate, start, size_t, JS_NewInt64)
DEFINE_GETTER(Candidate, end, size_t, JS_NewInt64)
DEFINE_GETTER(Candidate, quality, int, JS_NewInt32)
DEFINE_GETTER(Candidate, preedit, const string&, js_new_string_from_std)

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

DEFINE_NUMERIC_SETTER(Candidate, start, int64_t, JS_ToInt64)
DEFINE_NUMERIC_SETTER(Candidate, end, int64_t, JS_ToInt64)
DEFINE_NUMERIC_SETTER(Candidate, quality, int32_t, JS_ToInt32)

DEFINE_STRING_SETTER(Candidate, preedit,
  if (auto simpleCandidate = dynamic_cast<SimpleCandidate*>(obj.get())) {
    simpleCandidate->set_preedit(str);
  } else if (auto phrase = dynamic_cast<Phrase*>(obj.get())) {
    phrase->set_preedit(str);
  }
)

} // namespace rime
