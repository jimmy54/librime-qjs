#pragma once

#include <rime/engine.h>

#include <string>

#include "jsvalue_raii.h"
#include "qjs_engine.h"
#include "qjs_helper.h"
#include "quickjs.h"

namespace rime {

// Base class to handle common QuickJS module operations
class QjsModule {
public:
  void setEngine(Engine* engine) {
    auto* ctx = QjsHelper::getInstance().getContext();
    JS_SetPropertyStr(ctx, environment_, "engine", QjsEngine::Wrap(ctx, engine));
  }

protected:
  QjsModule(const std::string& nameSpace, Engine* engine, const char* mainFuncName);

  ~QjsModule();

  [[nodiscard]] bool isLoaded() const { return isLoaded_; }
  [[nodiscard]] JSValue getInstance() const { return instance_.get(); }
  [[nodiscard]] JSValue getMainFunc() const { return mainFunc_.get(); }
  [[nodiscard]] JSValue getEnvironment() const { return environment_.get(); }

private:
  const std::string namespace_;

  bool isLoaded_ = false;

  JSValueRAII instance_{JS_UNDEFINED};
  JSValueRAII mainFunc_{JS_UNDEFINED};
  JSValueRAII finalizer_{JS_UNDEFINED};

  JSValueRAII environment_{JS_UNDEFINED};
};

}  // namespace rime
