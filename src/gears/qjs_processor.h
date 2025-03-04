#ifndef RIME_QJS_PROCESSOR_H_
#define RIME_QJS_PROCESSOR_H_

#include <rime/processor.h>
#include "qjs_module.h"

namespace rime {

class QuickJSProcessor : public Processor, public QjsModule {
public:
  explicit QuickJSProcessor(const Ticket& ticket)
    : Processor(ticket), QjsModule(name_space_, engine_, "process") {}

  virtual ProcessResult ProcessKeyEvent(const KeyEvent& key_event);
};

} // namespace rime

#endif  // RIME_QJS_PROCESSOR_H_
