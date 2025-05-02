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

template <typename T_JS_VALUE>
class QuickJSLazyTranslation : public rime::Translation {
  using T_JS_OBJECT = typename JsEngine<T_JS_VALUE>::T_JS_OBJECT;

  bool isNextFunctionCalled_ = false;
  T_JS_OBJECT generator_;
  T_JS_OBJECT nextFunction_;
  T_JS_OBJECT nextResult_;

public:
  QuickJSLazyTranslation(const QuickJSLazyTranslation&) = delete;
  QuickJSLazyTranslation(QuickJSLazyTranslation&&) = delete;
  QuickJSLazyTranslation& operator=(const QuickJSLazyTranslation&) = delete;
  QuickJSLazyTranslation& operator=(QuickJSLazyTranslation&&) = delete;

  QuickJSLazyTranslation(const rime::an<rime::Translation>& translation,
                         const T_JS_OBJECT& filterObj,
                         const T_JS_OBJECT& filterFunc,
                         Environment* environment) {
    auto& jsEngine = JsEngine<T_JS_VALUE>::instance();
    auto iterator = jsEngine.wrap(translation);
    auto jsEnv = jsEngine.wrap(environment);
    T_JS_VALUE args[2] = {iterator, jsEnv};
    generator_ = jsEngine.toObject(jsEngine.callFunction(filterFunc, filterObj, 2, args));
    jsEngine.freeValue(jsEnv, iterator);
    nextFunction_ = jsEngine.toObject(jsEngine.getObjectProperty(generator_, "next"));
    jsEngine.protectFromGC(generator_, nextFunction_);
  }

  ~QuickJSLazyTranslation() override {
    auto& jsEngine = JsEngine<T_JS_VALUE>::instance();
    jsEngine.unprotectFromGC(generator_, nextFunction_);
    jsEngine.freeValue(generator_, nextFunction_);

    if (isNextFunctionCalled_) {
      jsEngine.unprotectFromGC(nextResult_);
      jsEngine.freeValue(nextResult_);
    }
  }

  bool Next() override {
    if (this->exhausted()) {
      return false;
    }

    auto& jsEngine = JsEngine<T_JS_VALUE>::instance();
    if (isNextFunctionCalled_) {
      jsEngine.unprotectFromGC(nextResult_);
      jsEngine.freeValue(nextResult_);
    }

    nextResult_ = jsEngine.toObject(jsEngine.callFunction(nextFunction_, generator_, 0, nullptr));
    jsEngine.protectFromGC(nextResult_);
    isNextFunctionCalled_ = true;

    auto jsDone = jsEngine.getObjectProperty(nextResult_, "done");
    bool isDone = jsEngine.isBool(jsDone) && jsEngine.toBool(jsDone);
    jsEngine.freeValue(jsDone);
    if (isDone) {
      set_exhausted(true);
    }
    return !isDone;
  }

  an<rime::Candidate> Peek() override {
    auto& jsEngine = JsEngine<T_JS_VALUE>::instance();
    if (!isNextFunctionCalled_) {
      this->Next();
    }
    if (this->exhausted()) {
      return nullptr;
    }

    auto jsValue = jsEngine.getObjectProperty(nextResult_, "value");
    auto ret = jsEngine.template unwrap<rime::Candidate>(jsValue);
    jsEngine.freeValue(jsValue);
    return ret;
  }
};
