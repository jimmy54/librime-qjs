#include "qjs_filter.h"
#include "qjs_translation.h"
#include "jsvalue_raii.h"
#include "qjs_helper.h"

#include <fstream>
#include <rime/translation.h>
#include <rime/gear/filter_commons.h>

namespace rime {

QuickJSFilter::QuickJSFilter(const Ticket& ticket,
                             JSContext* ctx,
                             const std::string& jsDirectory)
    : Filter(ticket), TagMatching(ticket), ctx_(ctx) {
  // filters:
  //   - qjs_filter@abc
  // => klass = qjs_filter, name_space = abc
  std::string fileName = ticket.name_space + ".js";
  JSValue moduleNamespace = QjsHelper::loadJsModuleToNamespace(ctx_, fileName.c_str());
  if (JS_IsUndefined(moduleNamespace)) {
    LOG(ERROR) << "[qjs] QuickJSFilter::QuickJSFilter Could not load " << fileName;
    return;
  }

  JSValueRAII initFunc(JS_GetPropertyStr(ctx_, moduleNamespace, "init"));
  if (!JS_IsUndefined(initFunc)) {
    LOG(INFO) << "[qjs] QuickJSFilter::QuickJSFilter running the init function";
    JS_Call(ctx_, initFunc, JS_UNDEFINED, 0, nullptr);
  } else {
    LOG(INFO) << "[qjs] QuickJSFilter::QuickJSFilter no `init` function exported in " << fileName;
  }

  finitFunc_ = JSValueRAII(JS_GetPropertyStr(ctx_, moduleNamespace, "finit"));

  filterFunc_ = JSValueRAII(JS_GetPropertyStr(ctx_, moduleNamespace, "filter"));
  if (JS_IsUndefined(filterFunc_)) {
    LOG(ERROR) << "[qjs] QuickJSFilter::QuickJSFilter No `filter` function exported in " << fileName;
    return;
  }

  isLoaded_ = true;
}

QuickJSFilter::~QuickJSFilter() {
  if (!JS_IsUndefined(finitFunc_)) {
    LOG(INFO) << "[qjs] QuickJSFilter::~QuickJSFilter running the finit function";
    JS_Call(ctx_, finitFunc_, JS_UNDEFINED, 0, nullptr);
  } else {
    LOG(INFO) << "[qjs] QuickJSFilter::~QuickJSFilter no `finit` function exported.";
  }
}

bool QuickJSFilter::AppliesToSegment(Segment* segment) {
  return TagsMatch(segment);
}

an<Translation> QuickJSFilter::Apply(an<Translation> translation,
                                     CandidateList* candidates) {
  if (!isLoaded_) {
    return translation;
  }

  return New<QuickJSTranslation>(translation, ctx_, filterFunc_);
}

}  // namespace rime
