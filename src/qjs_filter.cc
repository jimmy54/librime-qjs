#include "qjs_filter.h"
#include "qjs_translation.h"
#include "jsvalue_raii.h"
#include "qjs_helper.h"
#include "qjs_engine.h"

#include <fstream>
#include <rime/translation.h>
#include <rime/gear/filter_commons.h>

namespace rime {

QuickJSFilter::QuickJSFilter(const Ticket& ticket)
    : Filter(ticket) {
  auto ctx = QjsHelper::getInstance().getContext();

  // filters:
  //   - qjs_filter@abc
  // => klass = qjs_filter, name_space = abc
  std::string fileName = ticket.name_space + ".js";
  JSValue moduleNamespace = QjsHelper::loadJsModuleToNamespace(ctx, fileName.c_str());
  if (JS_IsUndefined(moduleNamespace)) {
    LOG(ERROR) << "[qjs] QuickJSFilter::QuickJSFilter Could not load " << fileName;
    return;
  }

  environment_ = JSValueRAII(JS_NewObject(ctx));
  JSValueRAII jsEngine(QjsEngine::Wrap(ctx, engine_));
  JS_SetPropertyStr(ctx, environment_, "engine", jsEngine);
  JSValueRAII jsNamespace(JS_NewString(ctx, name_space_.c_str()));
  JS_SetPropertyStr(ctx, environment_, "namespace", jsNamespace);
  JSValueRAII jsUserDataDir(JS_NewString(ctx, QjsHelper::basePath.c_str()));
  JS_SetPropertyStr(ctx, environment_, "userDataDir", jsUserDataDir);

  JSValueRAII initFunc(JS_GetPropertyStr(ctx, moduleNamespace, "init"));
  if (!JS_IsUndefined(initFunc)) {
    JS_Call(ctx, initFunc, JS_UNDEFINED, 1, environment_.getPtr());
  } else {
    DLOG(INFO) << "[qjs] QuickJSFilter::QuickJSFilter no `init` function exported in " << fileName;
  }

  finitFunc_ = JSValueRAII(JS_GetPropertyStr(ctx, moduleNamespace, "finit"));

  filterFunc_ = JSValueRAII(JS_GetPropertyStr(ctx, moduleNamespace, "filter"));
  if (JS_IsUndefined(filterFunc_)) {
    LOG(ERROR) << "[qjs] QuickJSFilter::QuickJSFilter No `filter` function exported in " << fileName;
    return;
  }

  isLoaded_ = true;
}

QuickJSFilter::~QuickJSFilter() {
  if (!JS_IsUndefined(finitFunc_)) {
    DLOG(INFO) << "[qjs] QuickJSFilter::~QuickJSFilter running the finit function";
    JS_Call(QjsHelper::getInstance().getContext(),
            finitFunc_,
            JS_UNDEFINED,
            1,
            environment_.getPtr());
  } else {
    DLOG(INFO) << "[qjs] QuickJSFilter::~QuickJSFilter no `finit` function exported.";
  }
}

an<Translation> QuickJSFilter::Apply(an<Translation> translation,
                                     CandidateList* candidates) {
  if (!isLoaded_) {
    return translation;
  }

  return New<QuickJSTranslation>(translation, filterFunc_, environment_);
}

}  // namespace rime
