#pragma once

#include <glog/logging.h>
#include <string>

#include "engines/engine_manager.h"

template <typename T_JS_VALUE>
class QjsModule {
protected:
  QjsModule(const std::string& nameSpace, T_JS_VALUE& environment, const char* mainFuncName)
      : namespace_(nameSpace) {
    auto& engine = getJsEngine<T_JS_VALUE>();
    std::string fileName = nameSpace + ".js";
    T_JS_VALUE container = engine.loadJsFile(fileName.c_str());  // module or globalThis
    if (!engine.isObject(container)) {
      LOG(ERROR) << "[qjs] Failed to load " << fileName;
      return;
    }

    T_JS_VALUE jsClass = engine.getJsClassHavingMethod(container, mainFuncName);
    if (!engine.isObject(jsClass)) {
      LOG(ERROR) << "[qjs] No exported class having `" << mainFuncName << "` function in "
                 << fileName;
      return;
    }

    T_JS_VALUE constructor = engine.getMethodOfClass(jsClass, "constructor");
    if (engine.isObject(constructor)) {
      instance_ = engine.callConstructor(constructor, 1, &environment);
      engine.freeValue(constructor);

      if (engine.isException(instance_)) {
        LOG(ERROR) << "[qjs] Error creating an instance of the exported class in " << fileName;
        engine.logErrorStackTrace(instance_);
        engine.freeValue(jsClass);
        return;
      }
      DLOG(INFO) << "[qjs] constructor function executed successfully in " << fileName;
    }

    mainFunc_ = engine.getMethodOfClass(jsClass, mainFuncName);
    finalizer_ = engine.getMethodOfClass(jsClass, "finalizer");

    engine.freeValue(jsClass);

    isLoaded_ = true;
  }

  ~QjsModule() {
    auto& engine = getJsEngine<T_JS_VALUE>();

    if (engine.isUndefined(finalizer_)) {
      DLOG(INFO) << "[qjs] ~" << namespace_ << " no `finalizer` function exported.";
    } else {
      DLOG(INFO) << "[qjs] running the finalizer function of " << namespace_;
      T_JS_VALUE finalizerResult = engine.callFunction(finalizer_, instance_, 0, nullptr);
      if (JS_IsException(finalizerResult)) {
        LOG(ERROR) << "[qjs] ~" << namespace_ << " Error running the finalizer function.";
      }
      engine.freeValue(finalizerResult);
    }

    engine.freeValue(instance_);
    engine.freeValue(mainFunc_);
    engine.freeValue(finalizer_);
  }

  [[nodiscard]] bool isLoaded() const { return isLoaded_; }
  [[nodiscard]] T_JS_VALUE getInstance() const { return instance_; }
  [[nodiscard]] T_JS_VALUE getMainFunc() const { return mainFunc_; }
  [[nodiscard]] std::string getNamespace() const { return namespace_; }

private:
  const std::string namespace_;

  bool isLoaded_ = false;

  T_JS_VALUE instance_;
  T_JS_VALUE mainFunc_;
  T_JS_VALUE finalizer_;

public:
  QjsModule(const QjsModule&) = delete;
  QjsModule(QjsModule&&) = delete;
  QjsModule& operator=(const QjsModule&) = delete;
  QjsModule& operator=(QjsModule&&) = delete;
};
