#pragma once

#include <rime/translation.h>

#include "engines/common.h"
#include "environment.h"

using namespace rime;

template <typename T_JS_VALUE>
class QuickJSTranslation : public rime::PrefetchTranslation {
public:
  QuickJSTranslation(const QuickJSTranslation&) = delete;
  QuickJSTranslation(QuickJSTranslation&&) = delete;
  QuickJSTranslation& operator=(const QuickJSTranslation&) = delete;
  QuickJSTranslation& operator=(QuickJSTranslation&&) = delete;

  QuickJSTranslation(rime::an<rime::Translation> translation,
                     const T_JS_VALUE& filterObj,
                     const T_JS_VALUE& filterFunc,
                     Environment* environment)
      : PrefetchTranslation(std::move(translation)), replenished_(true) {
    doFilter(filterObj, filterFunc, environment);
    set_exhausted(cache_.empty());
  }
  ~QuickJSTranslation() override = default;

protected:
  bool Replenish() override { return replenished_; }

private:
  bool doFilter(const T_JS_VALUE& filterObj,
                const T_JS_VALUE& filterFunc,
                Environment* environment) {
    auto& jsEngine = JsEngine<T_JS_VALUE>::instance();
    auto jsArray = jsEngine.newArray();
    size_t idx = 0;
    while (auto candidate = translation_->exhausted() ? nullptr : translation_->Peek()) {
      translation_->Next();
      jsEngine.insertItemToArray(jsArray, idx++, jsEngine.wrap(candidate));
    }
    if (idx == 0) {
      jsEngine.freeValue(jsArray);
      return true;
    }

    auto jsEnvironment = jsEngine.wrap(environment);
    T_JS_VALUE args[] = {jsArray, jsEnvironment};
    T_JS_VALUE resultArray =
        jsEngine.callFunction(jsEngine.toObject(filterFunc), jsEngine.toObject(filterObj), 2, args);
    jsEngine.freeValue(jsArray, jsEnvironment);

    if (!jsEngine.isArray(resultArray)) {
      LOG(ERROR) << "[qjs] Failed to filter candidates";
      jsEngine.freeValue(resultArray);
      return false;
    }

    size_t length = jsEngine.getArrayLength(resultArray);
    for (size_t i = 0; i < length; i++) {
      T_JS_VALUE item = jsEngine.getArrayItem(resultArray, i);
      if (auto candidate = jsEngine.template unwrap<Candidate>(item)) {
        cache_.push_back(candidate);
      } else {
        LOG(ERROR) << "[qjs] Failed to unwrap candidate at index " << i;
      }
      jsEngine.freeValue(item);
    }

    jsEngine.freeValue(resultArray);
    return true;
  }

  bool replenished_ = false;
};
