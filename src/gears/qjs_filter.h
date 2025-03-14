#ifndef RIME_QJS_FILTER_H_
#define RIME_QJS_FILTER_H_

#include <rime/engine.h>
#include <rime/filter.h>
#include <rime/gear/filter_commons.h>

#include "qjs_component.h"
#include "qjs_module.h"
#include "qjs_translation.h"
#include "quickjs.h"

namespace rime {

class QuickJSFilter : public QjsModule {
public:
  explicit QuickJSFilter(const Ticket& ticket, JSValue& environment)
      : QjsModule(ticket.name_space, environment, "filter") {}

  an<Translation> Apply(an<Translation> translation, const JSValue& environment) {
    if (!isLoaded()) {
      return translation;
    }

    return New<QuickJSTranslation>(translation, getInstance(), getMainFunc(), environment);
  }
};

// Specialization for Filter
template <typename T_ACTUAL>
class ComponentWrapper<T_ACTUAL, Filter> : public ComponentWrapperBase<T_ACTUAL, Filter> {
public:
  explicit ComponentWrapper(const Ticket& ticket,
                            const an<T_ACTUAL>& actual,
                            const JSValue& environment)
      : ComponentWrapperBase<T_ACTUAL, Filter>(ticket, actual, environment) {}

  virtual an<Translation> Apply(an<Translation> translation, CandidateList* candidates) {
    return this->actual_->Apply(translation, this->environment_.get());
  }
};

}  // namespace rime

#endif  // RIME_QJS_FILTER_H_
