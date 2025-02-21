#include "qjs_filter.h"
#include "qjs_translation.h"
#include "jsvalue_raii.h"
#include "qjs_helper.h"
#include "qjs_engine.h"

#include <fstream>
#include <filesystem>
#include <rime/translation.h>
#include <rime/gear/filter_commons.h>

namespace rime {

static JSValue loadFile(JSContext* ctx, JSValueConst this_val, int argc, JSValueConst* argv) {
  if (argc < 1) {
    return JS_ThrowSyntaxError(ctx, "The absolutePath argument is required");
  }

  const char* path = JS_ToCString(ctx, argv[0]);
  if (!path) {
    return JS_ThrowSyntaxError(ctx, "The absolutePath argument should be a string");
  }

  std::string content = QjsHelper::loadFile(path);
  JS_FreeCString(ctx, path);

  return JS_NewString(ctx, content.c_str());
}

static JSValue fileExists(JSContext* ctx, JSValueConst this_val, int argc, JSValueConst* argv) {
  if (argc < 1) {
    return JS_ThrowSyntaxError(ctx, "The absolutePath argument is required");
  }

  const char* path = JS_ToCString(ctx, argv[0]);
  if (!path) {
    return JS_ThrowSyntaxError(ctx, "The absolutePath argument should be a string");
  }

  bool exists = std::filesystem::exists(path);
  JS_FreeCString(ctx, path);

  return JS_NewBool(ctx, exists);
}

QuickJSFilter::QuickJSFilter(const Ticket& ticket)
    : Filter(ticket) {
  auto ctx = QjsHelper::getInstance().getContext();

  // filters:
  //   - qjs_filter@abc
  // => klass = qjs_filter, name_space = abc
  std::string fileName = ticket.name_space + ".js";
  JSValue moduleNamespace = QjsHelper::loadJsModuleToNamespace(ctx, fileName.c_str());
  if (JS_IsUndefined(moduleNamespace)) {
    LOG(ERROR) << "[qjs] QuickJSFilter Could not load " << fileName;
    return;
  }

  environment_ = JSValueRAII(JS_NewObject(ctx));
  JSValueRAII jsEngine(QjsEngine::Wrap(ctx, engine_));
  JS_SetPropertyStr(ctx, environment_, "engine", jsEngine);
  JSValueRAII jsNamespace(JS_NewString(ctx, name_space_.c_str()));
  JS_SetPropertyStr(ctx, environment_, "namespace", jsNamespace);
  JSValueRAII jsUserDataDir(JS_NewString(ctx, QjsHelper::basePath.c_str()));
  JS_SetPropertyStr(ctx, environment_, "userDataDir", jsUserDataDir);

  JSValue loadFileFunc = JS_NewCFunction(ctx, loadFile, "loadFile", 1);
  JS_SetPropertyStr(ctx, environment_, "loadFile", loadFileFunc);
  JSValue fileExistsFunc = JS_NewCFunction(ctx, fileExists, "fileExists", 1);
  JS_SetPropertyStr(ctx, environment_, "fileExists", fileExistsFunc);

  JSValueRAII initFunc(JS_GetPropertyStr(ctx, moduleNamespace, "init"));
  if (!JS_IsUndefined(initFunc)) {
    JSValueRAII initResult(JS_Call(ctx, initFunc, JS_UNDEFINED, 1, environment_.getPtr()));
    if (JS_IsException(initResult)) {
      LOG(ERROR) << "[qjs] QuickJSFilter Error running the init function in " << fileName;
      return;
    }
  } else {
    DLOG(INFO) << "[qjs] QuickJSFilter no `init` function exported in " << fileName;
  }

  finitFunc_ = JSValueRAII(JS_GetPropertyStr(ctx, moduleNamespace, "finit"));

  filterFunc_ = JSValueRAII(JS_GetPropertyStr(ctx, moduleNamespace, "filter"));
  if (JS_IsUndefined(filterFunc_)) {
    LOG(ERROR) << "[qjs] QuickJSFilter No `filter` function exported in " << fileName;
    return;
  }

  isLoaded_ = true;
}

QuickJSFilter::~QuickJSFilter() {
  if (!JS_IsUndefined(finitFunc_)) {
    DLOG(INFO) << "[qjs] QuickJSFilter::~QuickJSFilter running the finit function";
    JSValueRAII finitResult(JS_Call(QjsHelper::getInstance().getContext(),
                                    finitFunc_,
                                    JS_UNDEFINED,
                                    1,
                                    environment_.getPtr()));
    if (JS_IsException(finitResult)) {
      LOG(ERROR) << "[qjs] ~QuickJSFilter Error running the finit function.";
      return;
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
