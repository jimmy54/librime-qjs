#ifndef RIME_QJS_FILTER_H_
#define RIME_QJS_FILTER_H_

#include <rime/filter.h>
#include <rime/gear/filter_commons.h>
#include "quickjs.h"

namespace rime {

class QuickJSFilter : public Filter, TagMatching {
public:
  explicit QuickJSFilter(const Ticket& ticket, JSContext* ctx, const string& jsDirectory);
  virtual ~QuickJSFilter();

  virtual an<Translation> Apply(an<Translation> translation,
                              CandidateList* candidates);

  virtual bool AppliesToSegment(Segment* segment);

private:
  JSContext* ctx_;   // QuickJS context
  string jsFunctionName_;
  bool isLoaded_ = false;

  bool LoadScript(const string& scriptPath);
};

} // namespace rime

#endif  // RIME_QJS_FILTER_H_
