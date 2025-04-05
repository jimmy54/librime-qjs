#pragma once

#include <rime/gear/translator_commons.h>
#include <rime/translator.h>

#include <rime/composition.h>
#include <rime/context.h>
#include <rime/translation.h>

#include "engines/js_macros.h"
#include "qjs_component.hpp"
#include "qjs_module.h"

using namespace rime;

template <typename T_JS_VALUE>
class QuickJSTranslator : public QjsModule<T_JS_VALUE> {
public:
  explicit QuickJSTranslator(const rime::Ticket& ticket, Environment* environment)
      : QjsModule<T_JS_VALUE>(ticket.name_space, environment, "translate") {}

  rime::an<rime::Translation> query(const std::string& input,
                                    const rime::Segment& segment,
                                    Environment* environment) {
    auto translation = New<FifoTranslation>();
    if (!this->isLoaded()) {
      return translation;
    }

    auto* engine = this->getJsEngine();
    T_JS_VALUE jsInput = engine->toJsString(input.c_str());
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-const-cast)
    T_JS_VALUE jsSegment = engine->wrap(const_cast<Segment*>(&segment));
    auto jsEnvironment = engine->wrap(environment);
    T_JS_VALUE args[] = {jsInput, jsSegment, jsEnvironment};
    T_JS_VALUE resultArray =
        engine->callFunction(this->getMainFunc(), this->getInstance(), countof(args), args);
    engine->freeValue(jsInput, jsSegment, jsEnvironment);
    if (engine->isException(resultArray)) {
      return translation;
    }

    size_t length = engine->getArrayLength(resultArray);

    for (uint32_t i = 0; i < length; i++) {
      T_JS_VALUE item = engine->getArrayItem(resultArray, i);
      if (an<Candidate> candidate = engine->template unwrapShared<Candidate>(item)) {
        translation->Append(candidate);
      } else {
        LOG(ERROR) << "[qjs] Failed to unwrap candidate at index " << i;
      }
      engine->freeValue(item);
    }

    engine->freeValue(resultArray);
    return translation;
  }
};

// Specialization for Translator
template <typename T_ACTUAL, typename T_JS_VALUE>
class rime::ComponentWrapper<T_ACTUAL, rime::Translator, T_JS_VALUE>
    : public ComponentWrapperBase<T_ACTUAL, rime::Translator, T_JS_VALUE> {
public:
  explicit ComponentWrapper(const rime::Ticket& ticket)
      : ComponentWrapperBase<T_ACTUAL, rime::Translator, T_JS_VALUE>(ticket) {}

  // NOLINTNEXTLINE(readability-identifier-naming)
  virtual rime::an<rime::Translation> Query(const std::string& input,
                                            const rime::Segment& segment) {
    return this->actual()->query(input, segment, this->environment());
  }
};
