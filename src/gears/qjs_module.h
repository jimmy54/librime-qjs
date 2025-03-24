#pragma once

#include <rime/engine.h>

#include <string>

#include "jsvalue_raii.hpp"
#include "quickjs.h"

namespace rime {

// Base class to handle common QuickJS module operations
class QjsModule {
protected:
  QjsModule(const std::string& nameSpace, JSValue& environment, const char* mainFuncName);

  ~QjsModule();

  [[nodiscard]] bool isLoaded() const { return isLoaded_; }
  [[nodiscard]] JSValue getInstance() const { return instance_.get(); }
  [[nodiscard]] JSValue getMainFunc() const { return mainFunc_.get(); }
  [[nodiscard]] std::string getNamespace() const { return namespace_; }

private:
  const std::string namespace_;

  bool isLoaded_ = false;

  JSValueRAII instance_{JS_UNDEFINED};
  JSValueRAII mainFunc_{JS_UNDEFINED};
  JSValueRAII finalizer_{JS_UNDEFINED};
};

}  // namespace rime
