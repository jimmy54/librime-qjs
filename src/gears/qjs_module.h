#pragma once

#include <glog/logging.h>
#include <string>

#include "engines/engine_manager.h"
#include "qjs_types.h"

template <typename T_JS_VALUE>
class QjsModule {
  using T_JS_OBJECT = typename TypeMap<T_JS_VALUE>::ObjectType;

protected:
  [[nodiscard]] JsEngine<T_JS_VALUE>* getJsEngine() { return &jsEngine_; }

  QjsModule(const std::string& nameSpace, Environment* environment, const char* mainFuncName)
      : namespace_(nameSpace), jsEngine_(newOrShareEngine<T_JS_VALUE>()) {
    // Each module requires its own JavaScriptCore context to prevent
    // naming conflicts when loading multiple JavaScript files.
    // As a result, we must register types for each new engine instance.
    registerTypesToJsEngine(jsEngine_);

    // gets a module for quickjs, exception if failed
    // gets globalThis for javascriptcore, nullptr if failed
    T_JS_VALUE container = jsEngine_.loadJsFile(nameSpace.c_str());
    if (!jsEngine_.isObject(container)) {
      jsEngine_.freeValue(container);
      LOG(ERROR) << "[qjs] Failed to load plugin: " << mainFuncName << '@' << nameSpace;
      return;
    }

    T_JS_VALUE jsClass = jsEngine_.getJsClassHavingMethod(container, mainFuncName);
    if (!jsEngine_.isObject(jsClass)) {
      jsEngine_.freeValue(jsClass);
      LOG(ERROR) << "[qjs] No exported class having `" << mainFuncName << "` function in "
                 << nameSpace;
      return;
    }

    auto objClass = jsEngine_.toObject(jsClass);
    auto jsEnvironment = jsEngine_.wrap(environment);
    T_JS_VALUE args[] = {jsEnvironment};
    instance_ = jsEngine_.newClassInstance(objClass, 1, static_cast<T_JS_VALUE*>(args));
    if (jsEngine_.isException(instance_)) {
      LOG(ERROR) << "[qjs] Error creating an instance of the exported class in " << nameSpace;
      jsEngine_.logErrorStackTrace(instance_, __FILE_NAME__, __LINE__);
      jsEngine_.freeValue(jsClass, jsEnvironment);
      return;
    }
    DLOG(INFO) << "[qjs] constructor function executed successfully in " << nameSpace;

    mainFunc_ =
        jsEngine_.toObject(jsEngine_.getMethodOfClassOrInstance(objClass, instance_, mainFuncName));
    finalizer_ =
        jsEngine_.toObject(jsEngine_.getMethodOfClassOrInstance(objClass, instance_, "finalizer"));

    jsEngine_.freeValue(container, jsClass, jsEnvironment);
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

  JsEngine<T_JS_VALUE> jsEngine_;

  T_JS_OBJECT instance_;
  T_JS_OBJECT mainFunc_;
  T_JS_OBJECT finalizer_;

public:
  QjsModule(const QjsModule&) = delete;
  QjsModule(QjsModule&&) = delete;
  QjsModule& operator=(const QjsModule&) = delete;
  QjsModule& operator=(QjsModule&&) = delete;
};
