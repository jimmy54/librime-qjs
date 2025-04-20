#pragma once

#include <rime/context.h>
#include <rime/engine.h>
#include <rime/filter.h>
#include <rime/gear/filter_commons.h>
#include <chrono>
#include <iomanip>
#include <memory>

#include "environment.h"
#include "qjs_component.hpp"
#include "qjs_module.h"
#include "qjs_translation.h"

template <typename T_JS_VALUE>
class QuickJSFilter : public QjsModule<T_JS_VALUE> {
  inline static std::chrono::time_point<std::chrono::steady_clock, std::chrono::nanoseconds>
      beginClock = std::chrono::steady_clock::now();

public:
  explicit QuickJSFilter(const rime::Ticket& ticket, Environment* environment)
      : QjsModule<T_JS_VALUE>(ticket.name_space, environment, "filter") {}

  std::shared_ptr<rime::Translation> apply(std::shared_ptr<rime::Translation> translation,
                                           Environment* environment) {
    if (this->getNamespace().find("benchmark_begin") != std::string::npos) {
      beginClock = std::chrono::steady_clock::now();
    } else if (this->getNamespace().find("benchmark_end") != std::string::npos) {
      auto endClock = std::chrono::steady_clock::now();
      auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(endClock - beginClock);
      std::string engine = "jsc";
      if constexpr (std::is_same_v<T_JS_VALUE, JSValue>) {
        engine = "qjs";
      }
      LOG(INFO) << "[benchmark] all " << engine << " filters run for " << std::setw(3)
                << duration.count()
                << " ms, with input = " << environment->getEngine()->context()->input();
    }

    if (!this->isLoaded()) {
      return translation;
    }

    return std::make_shared<QuickJSTranslation<T_JS_VALUE>>(
        translation, this->getJsEngine(), this->getInstance(), this->getMainFunc(), environment);
  }
};

// Specialization for Filter
template <typename T_ACTUAL, typename T_JS_VALUE>
class rime::ComponentWrapper<T_ACTUAL, rime::Filter, T_JS_VALUE>
    : public ComponentWrapperBase<T_ACTUAL, rime::Filter, T_JS_VALUE> {
public:
  explicit ComponentWrapper(const rime::Ticket& ticket)
      : ComponentWrapperBase<T_ACTUAL, rime::Filter, T_JS_VALUE>(ticket) {}

  // NOLINTNEXTLINE(readability-identifier-naming)
  virtual rime::an<rime::Translation> Apply(rime::an<rime::Translation> translation,
                                            rime::CandidateList* candidates) {
    return this->actual()->apply(translation, this->environment());
  }
};
