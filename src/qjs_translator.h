#ifndef RIME_QJS_TRANSLATOR_H_
#define RIME_QJS_TRANSLATOR_H_

#include <rime/engine.h>
#include <rime/translator.h>
#include <rime/gear/translator_commons.h>
#include "quickjs.h"
#include "qjs_engine.h"
#include "jsvalue_raii.h"

namespace rime {

class QuickJSTranslator : public Translator {
public:
  explicit QuickJSTranslator(const Ticket& ticket);
  virtual ~QuickJSTranslator();

  virtual an<Translation> Query(const string& input,
                                const Segment& segment);

  void setEngine(Engine* engine) {
    this->engine_ = engine;
    auto ctx = QjsHelper::getInstance().getContext();
    JS_SetPropertyStr(ctx, environment_, "engine", QjsEngine::Wrap(ctx, engine));
  }

private:
  JSValueRAII translateFunc_{JS_UNDEFINED};
  JSValueRAII finitFunc_{JS_UNDEFINED};
  JSValueRAII environment_{JS_UNDEFINED};

  bool isLoaded_ = false;
};

} // namespace rime

#endif  // RIME_QJS_TRANSLATOR_H_
