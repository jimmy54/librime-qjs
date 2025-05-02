#pragma once

#include <rime/candidate.h>
#include <rime/gear/translator_commons.h>

#include "engines/js_macros.h"
#include "js_wrapper.h"

using namespace rime;

using CandidateIterator = rime::Translation;

template <>
class JsWrapper<rime::Translation> {
  DEFINE_GETTER(CandidateIterator, next, peekAndNext(obj))

  static an<Candidate> peekAndNext(const an<CandidateIterator>& obj) {
    if (obj->exhausted()) {
      return nullptr;
    }
    auto candidate = obj->Peek();
    obj->Next();
    return candidate;
  }

public:
  EXPORT_CLASS_WITH_SHARED_POINTER(CandidateIterator,
                                   WITHOUT_CONSTRUCTOR,
                                   WITHOUT_PROPERTIES,
                                   WITH_GETTERS(next),
                                   WITHOUT_FUNCTIONS);
};
