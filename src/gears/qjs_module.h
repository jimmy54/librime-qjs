#pragma once

#include <glog/logging.h>
#include <string>
#include <vector>

#include "engines/common.h"
#include "types/environment.h"

template <typename T_JS_VALUE>
class QjsModule {
  using T_JS_OBJECT = typename TypeMap<T_JS_VALUE>::ObjectType;

protected:
  [[nodiscard]] JsEngine<T_JS_VALUE>* getJsEngine() { return &jsEngine_; }

  QjsModule(const std::string& nameSpace, Environment* environment, const char* mainFuncName)
      : namespace_(nameSpace), jsEngine_(JsEngine<T_JS_VALUE>::instance()) {
    auto jsEnvironment = jsEngine_.wrap(environment);
    std::vector<T_JS_VALUE> args = {jsEnvironment};
    instance_ = jsEngine_.createInstanceOfModule(namespace_.c_str(), args, mainFuncName);
    jsEngine_.freeValue(jsEnvironment);

    if (!jsEngine_.isObject(instance_)) {
      jsEngine_.freeValue(instance_);
      LOG(ERROR) << "[qjs] Error creating an instance of the exported class in " << nameSpace;
      return;
    }

    mainFunc_ = jsEngine_.toObject(jsEngine_.getObjectProperty(instance_, mainFuncName));
    finalizer_ = jsEngine_.toObject(jsEngine_.getObjectProperty(instance_, "finalizer"));

    jsEngine_.protectFromGC(instance_, mainFunc_, finalizer_);

    isLoaded_ = true;
  }

  ~QjsModule() {
    if (jsEngine_.isUndefined(finalizer_)) {
      DLOG(INFO) << "[qjs] ~" << namespace_ << " no `finalizer` function exported.";
    } else if (isLoaded_) {
      DLOG(INFO) << "[qjs] running the finalizer function of " << namespace_;
      T_JS_VALUE finalizerResult = jsEngine_.callFunction(finalizer_, instance_, 0, nullptr);
      if (jsEngine_.isException(finalizerResult)) {
        LOG(ERROR) << "[qjs] ~" << namespace_ << " Error running the finalizer function.";
      }
      jsEngine_.freeValue(finalizerResult);
    }

    if (isLoaded_) {
      jsEngine_.unprotectFromGC(instance_, mainFunc_, finalizer_);
    }
    jsEngine_.freeValue(instance_, mainFunc_, finalizer_);
  }

  [[nodiscard]] bool isLoaded() const { return isLoaded_; }
  [[nodiscard]] T_JS_OBJECT getInstance() const { return instance_; }
  [[nodiscard]] T_JS_OBJECT getMainFunc() const { return mainFunc_; }
  [[nodiscard]] std::string getNamespace() const { return namespace_; }

private:
  const std::string namespace_;

  bool isLoaded_ = false;

  JsEngine<T_JS_VALUE>& jsEngine_;

  T_JS_OBJECT instance_;
  T_JS_OBJECT mainFunc_;
  T_JS_OBJECT finalizer_;

public:
  QjsModule(const QjsModule&) = delete;
  QjsModule(QjsModule&&) = delete;
  QjsModule& operator=(const QjsModule&) = delete;
  QjsModule& operator=(QjsModule&&) = delete;
};
