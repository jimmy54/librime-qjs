#include "qjs_filter.h"
#include "qjs_translation.h"
#include "jsvalue_raii.h"
#include "qjs_helper.h"
#include "qjs_engine.h"
#include "helpers/qjs_environment.h"

#include <fstream>
#include <filesystem>
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
  JSValueRAII moduleNamespace(QjsHelper::loadJsModuleToNamespace(ctx, fileName.c_str()));
  if (JS_IsUndefined(moduleNamespace) || JS_IsException(moduleNamespace)) {
    LOG(ERROR) << "[qjs] QuickJSFilter Could not load " << fileName;
    return;
  }

  // Create and initialize the environment
  environment_ = QjsEnvironment::Create(ctx, engine_, name_space_);

  // Call the init function if it exists
  if (!QjsEnvironment::CallInitFunction(ctx, moduleNamespace, environment_)) {
    LOG(ERROR) << "[qjs] QuickJSFilter Error running the init function in " << fileName;
    return;
  }

  finitFunc_ = JSValueRAII(JS_GetPropertyStr(ctx, moduleNamespace, "finit"));
  filterFunc_ = JSValueRAII(JS_GetPropertyStr(ctx, moduleNamespace, "filter"));
  if (JS_IsUndefined(filterFunc_)) {
    LOG(ERROR) << "[qjs] QuickJSFilter No `filter` function exported in " << fileName;
  } else {
    isLoaded_ = true;
  }
}

QuickJSFilter::~QuickJSFilter() {
  if (!JS_IsUndefined(finitFunc_)) {
    DLOG(INFO) << "[qjs] QuickJSFilter::~QuickJSFilter running the finit function";
    auto ctx = QjsHelper::getInstance().getContext();
    if (!QjsEnvironment::CallFinitFunction(ctx, finitFunc_, environment_)) {
      LOG(ERROR) << "[qjs] ~QuickJSFilter Error running the finit function.";
    }
  } else {
    DLOG(INFO) << "[qjs] ~QuickJSFilter no `finit` function exported.";
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
