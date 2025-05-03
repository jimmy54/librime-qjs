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
class QuickJSFastTranslation : public rime::Translation {
  using T_JS_OBJECT = typename JsEngine<T_JS_VALUE>::T_JS_OBJECT;

  bool isGeneratorEverInvoked_ = false;
  T_JS_OBJECT generator_;
  T_JS_OBJECT nextFunction_;
  T_JS_OBJECT nextResult_;

  an<Translation> upstream_{nullptr};

public:
  QuickJSFastTranslation(const QuickJSFastTranslation&) = delete;
  QuickJSFastTranslation(QuickJSFastTranslation&&) = delete;
  QuickJSFastTranslation& operator=(const QuickJSFastTranslation&) = delete;
  QuickJSFastTranslation& operator=(QuickJSFastTranslation&&) = delete;

  QuickJSFastTranslation(const rime::an<rime::Translation>& translation,
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

  ~QuickJSFastTranslation() override {
    auto& jsEngine = JsEngine<T_JS_VALUE>::instance();
    jsEngine.unprotectFromGC(generator_, nextFunction_);
    jsEngine.freeValue(generator_, nextFunction_);

    if (isGeneratorEverInvoked_ && jsEngine.isObject(nextResult_)) {
      jsEngine.unprotectFromGC(nextResult_);
      jsEngine.freeValue(nextResult_);
    }
  }

  bool Next() override {
    if (this->exhausted()) {
      return false;
    }

    invokeGenerator();

    if (upstream_ != nullptr) {
      // `return iter;` was called in js side, return the upstream data
      auto ret = upstream_->Next();
      this->set_exhausted(upstream_->exhausted());
      return ret;
    }

    return !this->exhausted();
  }

  an<rime::Candidate> Peek() override {
    if (!isGeneratorEverInvoked_) {
      invokeGenerator();
    }
    if (this->exhausted()) {
      return nullptr;
    }
    if (upstream_ != nullptr) {
      // `return iter;` was called in js side, return the upstream data
      return upstream_->exhausted() ? nullptr : upstream_->Peek();
    }

    auto& jsEngine = JsEngine<T_JS_VALUE>::instance();
    auto jsValue = jsEngine.getObjectProperty(nextResult_, "value");
    auto ret = jsEngine.template unwrap<rime::Candidate>(jsValue);
    jsEngine.freeValue(jsValue);
    return ret;
  }

private:
  void invokeGenerator() {
    if (upstream_ != nullptr) {
      return;
    }

    auto& jsEngine = JsEngine<T_JS_VALUE>::instance();
    if (isGeneratorEverInvoked_) {
      jsEngine.unprotectFromGC(nextResult_);
      jsEngine.freeValue(nextResult_);
    }

    nextResult_ = jsEngine.toObject(jsEngine.callFunction(nextFunction_, generator_, 0, nullptr));
    isGeneratorEverInvoked_ = true;
    if (jsEngine.isException(nextResult_)) {
      LOG(ERROR) << "[qjs] Exception thrown while filtering candidates with iterator";
      set_exhausted(true);
      return;
    }
    jsEngine.protectFromGC(nextResult_);

    auto jsDone = jsEngine.getObjectProperty(nextResult_, "done");
    bool isDone = jsEngine.isBool(jsDone) && jsEngine.toBool(jsDone);
    jsEngine.freeValue(jsDone);
    if (isDone) {
      // check the return value of the generator,
      auto jsValue = jsEngine.getObjectProperty(nextResult_, "value");
      if (auto upstream = jsEngine.template unwrap<Translation>(jsValue)) {
        // it returned the upstream translation with `return iter;` in js side
        upstream_ = upstream;
        set_exhausted(upstream->exhausted());
      } else {
        // it returned undefined with `return;` in js side
        set_exhausted(true);
      }
      jsEngine.freeValue(jsValue);
    }
  }
};
