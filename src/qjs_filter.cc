#include "qjs_filter.h"
#include "qjs_translation.h"

#include <rime/translation.h>
#include <rime/gear/filter_commons.h>

namespace rime {

QuickJSFilter::QuickJSFilter(const Ticket& ticket,
                             JSRuntime* rt,
                             JSContext* ctx)
    : Filter(ticket), TagMatching(ticket), rt_(rt), ctx_(ctx) {
  // LOG(INFO) << "[qjs] QuickJSFilter::QuickJSFilter"
  //           << " name_space=" << ticket.name_space
  //           << " klass=" << ticket.klass;
  // qjs_filter@abc => klass = qjs_filter, name_space = abc
  if (ticket.name_space == "filter") {
    name_space_ = "filter";
  }
}

QuickJSFilter::~QuickJSFilter() {
  LOG(INFO) << "[qjs] QuickJSFilter::~QuickJSFilter";
}

bool QuickJSFilter::AppliesToSegment(Segment* segment) {
  return TagsMatch(segment);
}

an<Translation> QuickJSFilter::Apply(an<Translation> translation,
                                     CandidateList* candidates) {
  LOG(INFO) << "[qjs] QuickJSFilter::Apply";
  return New<QuickJSTranslation>(translation, ctx_, "", "");
}

}  // namespace rime
