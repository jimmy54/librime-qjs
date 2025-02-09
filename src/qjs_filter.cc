#include "qjs_filter.h"
// #include "../types/qjs_translation.h"
// #include "qjs_candidate.h"
#include <rime/translation.h>
#include <rime/gear/filter_commons.h>
// #include <vector>
// #include <sstream>

namespace rime {

class QuickJSTranslation : public Translation {
 public:
  QuickJSTranslation(an<Translation> translation, JSContext* ctx);
  ~QuickJSTranslation();

  bool Next();
  an<Candidate> Peek();

 private:
  JSContext* ctx_;
  an<Translation> translation_;
};

QuickJSTranslation::QuickJSTranslation(an<Translation> translation,
                                                   JSContext* ctx)
    : translation_(translation), ctx_(ctx) {
  LOG(INFO) << "[qjs] QuickJSTranslation::QuickJSTranslation";
}

QuickJSTranslation::~QuickJSTranslation() {
  LOG(INFO) << "[qjs] QuickJSTranslation::~QuickJSTranslation";
}
bool QuickJSTranslation::Next() {
  LOG(INFO) << "[qjs] QuickJSTranslation::Next";
  return translation_->Next();
}
an<Candidate> QuickJSTranslation::Peek() {
  LOG(INFO) << "[qjs] QuickJSTranslation::Peek";
  return translation_->Peek();
}

QuickJSFilter::QuickJSFilter(const Ticket& ticket,
                             JSRuntime* rt,
                             JSContext* ctx)
    : Filter(ticket), TagMatching(ticket), rt_(rt), ctx_(ctx) {
  LOG(INFO) << "[qjs] QuickJSFilter::QuickJSFilter";
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
  return New<QuickJSTranslation>(translation, ctx_);
}

}  // namespace rime
