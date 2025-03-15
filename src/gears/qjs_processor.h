#ifndef RIME_QJS_PROCESSOR_H_
#define RIME_QJS_PROCESSOR_H_

#include <rime/processor.h>

#include "qjs_component.h"
#include "qjs_module.h"
#include "quickjs.h"

namespace rime {

class QuickJSProcessor : public QjsModule {
public:
  explicit QuickJSProcessor(const Ticket& ticket, JSValue& environment)
      : QjsModule(ticket.name_space, environment, "process") {}

  ProcessResult processKeyEvent(const KeyEvent& keyEvent, const JSValue& environment);
};

// Specialization for Processor
template <typename T_ACTUAL>
class ComponentWrapper<T_ACTUAL, Processor> : public ComponentWrapperBase<T_ACTUAL, Processor> {
public:
  explicit ComponentWrapper(const Ticket& ticket,
                            const an<T_ACTUAL>& actual,
                            const JSValue& environment)
      : ComponentWrapperBase<T_ACTUAL, Processor>(ticket, actual, environment) {}

  // NOLINTNEXTLINE(readability-identifier-naming)
  ProcessResult ProcessKeyEvent(const KeyEvent& keyEvent) override {
    return this->actual()->processKeyEvent(keyEvent, this->environment());
  }
};

}  // namespace rime

#endif  // RIME_QJS_PROCESSOR_H_
