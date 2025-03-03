#ifndef RIME_QJS_SEGMENT_H_
#define RIME_QJS_SEGMENT_H_

#include "qjs_macros.h"
#include "qjs_type_registry.h"
#include <rime/segmentation.h>

DECLARE_JS_CLASS_WITH_RAW_POINTER(Segment,
  DECLARE_PROPERTIES(selectedIndex, selectedCandidate, candidateSize, start, end, prompt),
  DECLARE_FUNCTIONS(getCandidateAt, hasTag)
)

#endif // RIME_QJS_SEGMENT_H_
