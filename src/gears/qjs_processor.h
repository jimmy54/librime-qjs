#pragma once

#include <rime/processor.h>

#include <rime/gear/translator_commons.h>
#include <rime/translation.h>
#include "qjs_component.hpp"
#include "qjs_module.h"

#include "engines/engine_manager.h"

template <typename T_JS_VALUE>
class QuickJSProcessor : public QjsModule<T_JS_VALUE> {
public:
  explicit QuickJSProcessor(const rime::Ticket& ticket, T_JS_VALUE& environment)
      : QjsModule<T_JS_VALUE>(ticket.name_space, environment, "process") {}

  rime::ProcessResult processKeyEvent(const rime::KeyEvent& keyEvent,
                                      const T_JS_VALUE& environment) {
    if (!this->isLoaded()) {
      return rime::kNoop;
    }

    auto& engine = getJsEngine<T_JS_VALUE>();
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-const-cast)
    T_JS_VALUE jsKeyEvt = engine.wrap(const_cast<rime::KeyEvent*>(&keyEvent));
    T_JS_VALUE args[] = {jsKeyEvt, environment};
    T_JS_VALUE jsResult = engine.callFunction(this->getMainFunc(), this->getInstance(), 2, args);
    if (JS_IsException(jsResult)) {
      return rime::kNoop;
    }
    engine.freeValue(jsKeyEvt);

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
  explicit ComponentWrapper(const rime::Ticket& ticket,
                            const rime::an<T_ACTUAL>& actual,
                            const T_JS_VALUE& environment)
      : ComponentWrapperBase<T_ACTUAL, rime::Processor, T_JS_VALUE>(ticket, actual, environment) {}

  // NOLINTNEXTLINE(readability-identifier-naming)
  rime::ProcessResult ProcessKeyEvent(const rime::KeyEvent& keyEvent) override {
    return this->actual()->processKeyEvent(keyEvent, this->environment());
  }
};
