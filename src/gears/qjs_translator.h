#ifndef RIME_QJS_TRANSLATOR_H_
#define RIME_QJS_TRANSLATOR_H_

#include <rime/translator.h>
#include <rime/gear/translator_commons.h>
#include "qjs_module.h"

namespace rime {

class QuickJSTranslator : public Translator, public QjsModule {
public:
  explicit QuickJSTranslator(const Ticket& ticket)
    : Translator(ticket) , QjsModule(name_space_, engine_, "translate") {}

   an<Translation> Query(const string& input, const Segment& segment) override;
};

} // namespace rime

#endif  // RIME_QJS_TRANSLATOR_H_
