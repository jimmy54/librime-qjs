#pragma once

#include <rime/engine.h>
#include "quickjs.h"
#include "jsvalue_raii.h"
#include "qjs_helper.h"
#include "qjs_engine.h"

#include <string>

namespace rime {

// Base class to handle common QuickJS module operations
class QjsModule {
public:

  void setEngine(Engine* engine) {
    auto ctx = QjsHelper::getInstance().getContext();
    JS_SetPropertyStr(ctx, environment_, "engine", QjsEngine::Wrap(ctx, engine));
  }

protected:

  QjsModule(const std::string& name_space,
            Engine* engine,
            const char* main_func_name);

  virtual ~QjsModule();

  bool isLoaded_ = false;

  JSValueRAII mainFunc_{ JS_UNDEFINED };
  JSValueRAII finitFunc_{ JS_UNDEFINED };
  JSValueRAII environment_{ JS_UNDEFINED };

private:
  const std::string namespace_;
};

} // namespace rime
