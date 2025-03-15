#ifndef RIME_QJS_TRANSLATOR_H_
#define RIME_QJS_TRANSLATOR_H_

#include <rime/gear/translator_commons.h>
#include <rime/translator.h>

#include "qjs_component.h"
#include "qjs_module.h"
#include "quickjs.h"

namespace rime {

class QuickJSTranslator : public QjsModule {
public:
  explicit QuickJSTranslator(const Ticket& ticket, JSValue& environment)
      : QjsModule(ticket.name_space, environment, "translate") {}

  an<Translation> query(const string& input, const Segment& segment, const JSValue& environment);
};

// Specialization for Translator
template <typename T_ACTUAL>
class ComponentWrapper<T_ACTUAL, Translator> : public ComponentWrapperBase<T_ACTUAL, Translator> {
public:
  explicit ComponentWrapper(const Ticket& ticket,
                            const an<T_ACTUAL>& actual,
                            const JSValue& environment)
      : ComponentWrapperBase<T_ACTUAL, Translator>(ticket, actual, environment) {}

  // NOLINTNEXTLINE(readability-identifier-naming)
  virtual an<Translation> Query(const string& input, const Segment& segment) {
    return this->actual()->query(input, segment, this->environment());
  }
};

}  // namespace rime

#endif  // RIME_QJS_TRANSLATOR_H_
