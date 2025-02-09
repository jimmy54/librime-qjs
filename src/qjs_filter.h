#ifndef RIME_QJS_FILTER_H_
#define RIME_QJS_FILTER_H_

#include <rime/filter.h>
#include <rime/gear/filter_commons.h>
#include "quickjs.h"

namespace rime {

class QuickJSFilter : public Filter, TagMatching {
public:
  explicit QuickJSFilter(const Ticket& ticket, JSRuntime* rt, JSContext* ctx);
  virtual ~QuickJSFilter();

  virtual an<Translation> Apply(an<Translation> translation,
                              CandidateList* candidates);

  virtual bool AppliesToSegment(Segment* segment);

private:
  JSRuntime* rt_;    // QuickJS runtime
  JSContext* ctx_;   // QuickJS context
};

} // namespace rime

#endif  // RIME_QJS_FILTER_H_
