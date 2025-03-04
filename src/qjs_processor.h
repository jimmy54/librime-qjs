#ifndef RIME_QJS_PROCESSOR_H_
#define RIME_QJS_PROCESSOR_H_

#include <rime/engine.h>
#include <rime/processor.h>
#include "quickjs.h"
#include "qjs_engine.h"
#include "jsvalue_raii.h"

namespace rime {

class QuickJSProcessor : public Processor {
public:
  explicit QuickJSProcessor(const Ticket& ticket);
  virtual ~QuickJSProcessor();

  virtual ProcessResult ProcessKeyEvent(const KeyEvent& key_event);

  void setEngine(Engine* engine) {
    this->engine_ = engine;
    auto ctx = QjsHelper::getInstance().getContext();
    JS_SetPropertyStr(ctx, environment_, "engine", QjsEngine::Wrap(ctx, engine));
  }

private:
  JSValueRAII processFunc_{JS_UNDEFINED};
  JSValueRAII finitFunc_{JS_UNDEFINED};
  JSValueRAII environment_{JS_UNDEFINED};

  bool isLoaded_ = false;
};

} // namespace rime

#endif  // RIME_QJS_PROCESSOR_H_
