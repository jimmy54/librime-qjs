#include <rime/menu.h>
#include "qjs_segment.h" // IWYU pragma: keep
#include "qjs_candidate.h"

namespace rime {

DEFINE_GETTER(Segment, selectedIndex, JS_NewInt32(ctx, obj->selected_index))
DEFINE_SETTER(Segment, selectedIndex, int32_t, JS_ToInt32, obj->selected_index = value)

DEFINE_GETTER(Segment, prompt, js_new_string_from_std(ctx, obj->prompt))
DEFINE_STRING_SETTER(Segment, prompt, obj->prompt = str)

DEFINE_GETTER(Segment, start, JS_NewInt32(ctx, obj->start))
DEFINE_GETTER(Segment, end, JS_NewInt32(ctx, obj->end))
DEFINE_GETTER(Segment, selectedCandidate, QjsCandidate::Wrap(ctx, obj->GetSelectedCandidate()))
DEFINE_GETTER(Segment, candidateSize, JS_NewInt32(ctx, obj->menu->candidate_count()))

DEFINE_FUNCTION_ARGC(Segment, getCandidateAt, 1,
  int32_t index;
  JS_ToInt32(ctx, &index, argv[0]);
  if (index < 0 || size_t(index) >= obj->menu->candidate_count()) return JS_NULL;

  return QjsCandidate::Wrap(ctx, obj->menu->GetCandidateAt(index));
)

DEFINE_FUNCTION_ARGC(Segment, hasTag, 1,
  JSStringRAII param(JS_ToCString(ctx, argv[0]));
  return JS_NewBool(ctx, obj->HasTag(param));
)

DEFINE_JS_CLASS_WITH_RAW_POINTER(
  Segment,
  NO_CONSTRUCTOR_TO_REGISTER,
  DEFINE_PROPERTIES(selectedIndex, prompt),
  DEFINE_GETTERS(start, end, selectedCandidate, candidateSize),
  DEFINE_FUNCTIONS(
    JS_CFUNC_DEF("getCandidateAt", 1, getCandidateAt),
    JS_CFUNC_DEF("hasTag", 1, hasTag),
  )
)

} // namespace rime
