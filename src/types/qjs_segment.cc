#include <rime/menu.h>
#include "qjs_segment.h"
#include "qjs_candidate.h"

namespace rime {

DEFINE_JS_CLASS_WITH_RAW_POINTER(
  Segment,
  NO_CONSTRUCTOR_TO_REGISTER,
  DEFINE_PROPERTIES(selectedIndex, selectedCandidate, candidateSize, start, end, prompt),
  DEFINE_FUNCTIONS(
    JS_CFUNC_DEF("getCandidateAt", 1, getCandidateAt),
    JS_CFUNC_DEF("hasTag", 1, hasTag),
  )
)

DEFINE_GETTER_3(Segment, selectedIndex, selected_index, size_t, JS_NewInt32)
DEFINE_SETTER_3(Segment, selectedIndex, selected_index, int32_t, JS_ToInt32)

DEFINE_GETTER_3(Segment, start, start, size_t, JS_NewInt32)
DEFINE_FORBIDDEN_SETTER(Segment, start)

DEFINE_GETTER_3(Segment, end, end, size_t, JS_NewInt32)
DEFINE_FORBIDDEN_SETTER(Segment, end)

DEFINE_GETTER_3(Segment, prompt, prompt, string, js_new_string_from_std)
DEFINE_STRING_SETTER(Segment, prompt, obj->prompt = str)

DEFINE_GETTER_2(Segment, selectedCandidate, GetSelectedCandidate, an<Candidate>, QjsCandidate::Wrap)
DEFINE_FORBIDDEN_SETTER(Segment, selectedCandidate)

[[nodiscard]]
JSValue QjsSegment::get_candidateSize(JSContext* ctx, JSValueConst this_val) {
  if (auto obj = Unwrap(ctx, this_val)) {
    return JS_NewInt32(ctx, obj->menu->candidate_count()); // <-- incompatible to DEFINE_GETTER_3
  }
  return JS_UNDEFINED;
}
DEFINE_FORBIDDEN_SETTER(Segment, candidateSize)

DEF_FUNC_WITH_ARGC(Segment, getCandidateAt, 1,
  int32_t index;
  JS_ToInt32(ctx, &index, argv[0]);
  if (index < 0 || index >= obj->menu->candidate_count()) return JS_NULL;

  return QjsCandidate::Wrap(ctx, obj->menu->GetCandidateAt(index));
)

DEF_FUNC_WITH_ARGC(Segment, hasTag, 1,
  JSStringRAII param(JS_ToCString(ctx, argv[0]));
  return JS_NewBool(ctx, obj->HasTag(param));
)

} // namespace rime
