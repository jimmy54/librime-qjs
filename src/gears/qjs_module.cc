#include "qjs_module.h"

#include "jsvalue_raii.h"
#include "qjs_environment.h"
#include "qjs_helper.h"

namespace rime {

QjsModule::QjsModule(const std::string& nameSpace, Engine* engine, const char* mainFuncName)
    : namespace_(nameSpace) {
  auto* ctx = QjsHelper::getInstance().getContext();
  std::string fileName = nameSpace + ".js";
  JSValueRAII moduleNamespace(QjsHelper::loadJsModuleToNamespace(ctx, fileName.c_str()));
  if (JS_IsUndefined(moduleNamespace) || JS_IsException(moduleNamespace)) {
    LOG(ERROR) << "[qjs] Failed to load " << fileName;
    return;
  }

  JSValueRAII jsClazz =
      QjsHelper::getExportedClassHavingMethodNameInModule(ctx, moduleNamespace, mainFuncName);
  if (JS_IsUndefined(jsClazz)) {
    LOG(ERROR) << "[qjs] No exported class having `" << mainFuncName << "` function in "
               << fileName;
    return;
  }

  environment_ = QjsEnvironment::create(ctx, engine, nameSpace);

  JSValueRAII proto = JS_GetPropertyStr(ctx, jsClazz, "prototype");
  JSValueRAII constructor = JS_GetPropertyStr(ctx, proto, "constructor");
  if (!JS_IsUndefined(constructor)) {
    instance_ = JS_CallConstructor(ctx, constructor, 1, environment_.getPtr());
    if (JS_IsException(instance_)) {
      LOG(ERROR) << "[qjs] Error creating an instance of the exported class in " << fileName;
      return;
    }
    DLOG(INFO) << "[qjs] constructor function executed successfully in " << fileName;
  }

  mainFunc_ = QjsHelper::getMethodByNameInClass(ctx, jsClazz, mainFuncName);
  finalizer_ = QjsHelper::getMethodByNameInClass(ctx, jsClazz, "finalizer");

  isLoaded_ = true;
}

QjsModule::~QjsModule() {
  if (JS_IsUndefined(finalizer_)) {
    DLOG(INFO) << "[qjs] ~" << namespace_ << " no `finalizer` function exported.";
  } else {
    DLOG(INFO) << "[qjs] running the finalizer function of " << namespace_;
    auto* ctx = QjsHelper::getInstance().getContext();
    JSValueRAII finalizerResult(JS_Call(ctx, finalizer_, instance_, 1, environment_.getPtr()));
    if (JS_IsException(finalizerResult)) {
      LOG(ERROR) << "[qjs] ~" << namespace_ << " Error running the finalizer function.";
    }
  }
}

}  // namespace rime
