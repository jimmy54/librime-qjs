#pragma once

#include <rime/processor.h>

#include <rime/gear/translator_commons.h>
#include <rime/translation.h>
#include "qjs_component.hpp"
#include "qjs_module.h"

template <typename T_JS_VALUE>
class QuickJSProcessor : public QjsModule<T_JS_VALUE> {
public:
  explicit QuickJSProcessor(const rime::Ticket& ticket, Environment* environment)
      : QjsModule<T_JS_VALUE>(ticket.name_space, environment, "process") {}

  rime::ProcessResult processKeyEvent(const rime::KeyEvent& keyEvent, Environment* environment) {
    if (!this->isLoaded()) {
      return rime::kNoop;
    }

    auto& engine = JsEngine<T_JS_VALUE>::instance();
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-const-cast)
    T_JS_VALUE jsKeyEvt = engine.wrap(const_cast<rime::KeyEvent*>(&keyEvent));
    auto jsEnvironment = engine.wrap(environment);
    T_JS_VALUE args[] = {jsKeyEvt, jsEnvironment};
    T_JS_VALUE jsResult = engine.callFunction(this->getMainFunc(), this->getInstance(), 2, args);
    engine.freeValue(jsKeyEvt, jsEnvironment);

    if (engine.isException(jsResult)) {
      LOG(ERROR) << "[qjs] " << this->getNamespace()
                 << " failed to process keyEvent = " << keyEvent.repr();
      return rime::kNoop;
    }

    std::string result = engine.toStdString(jsResult);
    engine.freeValue(jsResult);

    if (result == "kNoop") {
      return rime::kNoop;
    }
    if (result == "kAccepted") {
      return rime::kAccepted;
    }
    if (result == "kRejected") {
      return rime::kRejected;
    }

    LOG(ERROR) << "[qjs] " << this->getNamespace()
               << "::ProcessKeyEvent unknown result: " << result;
    return rime::kNoop;
  }
};

// Specialization for Processor
template <typename T_ACTUAL, typename T_JS_VALUE>
class rime::ComponentWrapper<T_ACTUAL, rime::Processor, T_JS_VALUE>
    : public ComponentWrapperBase<T_ACTUAL, rime::Processor, T_JS_VALUE> {
public:
  explicit ComponentWrapper(const rime::Ticket& ticket)
      : ComponentWrapperBase<T_ACTUAL, rime::Processor, T_JS_VALUE>(ticket) {}

  // NOLINTNEXTLINE(readability-identifier-naming)
  rime::ProcessResult ProcessKeyEvent(const rime::KeyEvent& keyEvent) override {
    return this->actual()->processKeyEvent(keyEvent, this->environment());
  }
};
