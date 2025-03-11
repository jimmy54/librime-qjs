#ifndef RIME_QJS_FILTER_H_
#define RIME_QJS_FILTER_H_

#include <rime/engine.h>
#include <rime/filter.h>
#include <rime/gear/filter_commons.h>
#include "qjs_module.h"
#include "qjs_translation.h"

namespace rime {

class QuickJSFilter : public Filter, public QjsModule {
public:
  explicit QuickJSFilter(const Ticket& ticket)
    : Filter(ticket), QjsModule(name_space_, engine_, "filter") {}

   an<Translation> Apply(an<Translation> translation, CandidateList* candidates) override {
    if (!isLoaded()) {
      return translation;
    }

    return New<QuickJSTranslation>(translation, getInstance(), getMainFunc(), getEnvironment());
  }
};

} // namespace rime

#endif  // RIME_QJS_FILTER_H_
