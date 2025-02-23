#ifndef RIME_QJS_FILTER_H_
#define RIME_QJS_FILTER_H_

#include <rime/engine.h>
#include <rime/filter.h>
#include <rime/gear/filter_commons.h>
#include "quickjs.h"
#include "qjs_engine.h"
#include "jsvalue_raii.h"

namespace rime {

class QuickJSFilter : public Filter {
public:
  explicit QuickJSFilter(const Ticket& ticket);
  virtual ~QuickJSFilter();

  virtual an<Translation> Apply(an<Translation> translation,
                                CandidateList* candidates);

  void setEngine(Engine* engine) {
    this->engine_ = engine;
    auto ctx = QjsHelper::getInstance().getContext();
    JS_SetPropertyStr(ctx, environment_, "engine", QjsEngine::Wrap(ctx, engine));
  }

private:
  JSValueRAII filterFunc_{JS_UNDEFINED};
  JSValueRAII finitFunc_{JS_UNDEFINED};
  JSValueRAII environment_{JS_UNDEFINED};

  bool isLoaded_ = false;
};

} // namespace rime

#endif  // RIME_QJS_FILTER_H_
