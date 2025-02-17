#ifndef RIME_QJS_CANDIDATE_H_
#define RIME_QJS_CANDIDATE_H_

#include "qjs_macros.h"
#include "qjs_type_registry.h"
#include <rime/candidate.h>

DECLARE_JS_CLASS_WITH_SHARED_POINTER(Candidate,
  DECLARE_PROPERTIES(text, comment, type, start, end, quality, preedit),
  // no functions to declare
)

#endif // RIME_QJS_CANDIDATE_H_
