#pragma once

#include <rime/translation.h>

#include "engines/engine_manager.h"

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
                     const T_JS_VALUE& environment)
      : PrefetchTranslation(std::move(translation)), replenished_(true) {
    doFilter(filterObj, filterFunc, environment);
    set_exhausted(cache_.empty());
  }
  ~QuickJSTranslation() override = default;

  // an<Candidate> Peek() override { return cache_.empty() ? nullptr : cache_.front(); }

  // bool Next() override {
  //   if (!cache_.empty()) {
  //     cache_.pop_front();
  //     return true;
  //   }
  //   set_exhausted(true);
  //   return false;
  // }

protected:
  bool Replenish() override { return replenished_; }

private:
  bool doFilter(const T_JS_VALUE& filterObj,
                const T_JS_VALUE& filterFunc,
                const T_JS_VALUE& environment) {
    auto& engine = getJsEngine<T_JS_VALUE>();
    auto jsArray = engine.newArray();
    size_t idx = 0;
    while (auto candidate = translation_->exhausted() ? nullptr : translation_->Peek()) {
      translation_->Next();
      engine.insertItemToArray(jsArray, idx++, engine.wrapShared(candidate));
    }
    if (idx == 0) {
      engine.freeValue(jsArray);
      return true;
    }

    T_JS_VALUE args[] = {jsArray, environment};
    T_JS_VALUE resultArray = engine.callFunction(filterFunc, filterObj, 2, args);
    engine.freeValue(jsArray);
    if (engine.isException(resultArray)) {
      return false;
    }

    size_t length = engine.getArrayLength(resultArray);
    for (size_t i = 0; i < length; i++) {
      T_JS_VALUE item = engine.getArrayItem(resultArray, i);
      if (auto candidate = engine.template unwrapShared<Candidate>(item)) {
        cache_.push_back(candidate);
      } else {
        LOG(ERROR) << "[qjs] Failed to unwrap candidate at index " << i;
      }
      engine.freeValue(item);
    }

    engine.freeValue(resultArray);
    return true;
  }
  bool replenished_ = false;
};
