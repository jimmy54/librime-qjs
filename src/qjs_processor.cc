#include "qjs_processor.h"
#include "qjs_key_event.h"
#include "jsvalue_raii.h"
#include "qjs_helper.h"
#include "helpers/qjs_environment.h"

#include <fstream>
#include <rime/translation.h>
#include <rime/gear/translator_commons.h>

namespace rime {

QuickJSProcessor::QuickJSProcessor(const Ticket& ticket)
  : Processor(ticket) {
  auto ctx = QjsHelper::getInstance().getContext();
  std::string fileName = ticket.name_space + ".js";
  JSValueRAII moduleNamespace(QjsHelper::loadJsModuleToNamespace(ctx, fileName.c_str()));
  if (JS_IsUndefined(moduleNamespace) || JS_IsException(moduleNamespace)) {
    LOG(ERROR) << "[qjs] QuickJSProcessor Could not load " << fileName;
    return;
  }

  environment_ = QjsEnvironment::Create(ctx, engine_, name_space_);

  if (!QjsEnvironment::CallInitFunction(ctx, moduleNamespace, environment_)) {
    LOG(ERROR) << "[qjs] QuickJSProcessor Error running the init function in " << fileName;
    return;
  }
  else {
    DLOG(INFO) << "[qjs] QuickJSProcessor init function executed successfully in " << fileName;
  }

  finitFunc_ = JSValueRAII(JS_GetPropertyStr(ctx, moduleNamespace, "finit"));
  processFunc_ = JSValueRAII(JS_GetPropertyStr(ctx, moduleNamespace, "process"));
  if (JS_IsUndefined(processFunc_)) {
    LOG(ERROR) << "[qjs] QuickJSProcessor No `translate` function exported in " << fileName;
  }
  else {
    isLoaded_ = true;
  }
}

QuickJSProcessor::~QuickJSProcessor() {
  if (!JS_IsUndefined(finitFunc_)) {
    DLOG(INFO) << "[qjs] QuickJSProcessor::~QuickJSProcessor running the finit function";
    auto ctx = QjsHelper::getInstance().getContext();
    if (!QjsEnvironment::CallFinitFunction(ctx, finitFunc_, environment_)) {
      LOG(ERROR) << "[qjs] ~QuickJSProcessor Error running the finit function.";
    }
  }
  else {
    DLOG(INFO) << "[qjs] ~QuickJSProcessor no `finit` function exported.";
  }
}

ProcessResult QuickJSProcessor::ProcessKeyEvent(const KeyEvent& key_event) {
  if (!isLoaded_) {
    return kNoop;
  }

  auto ctx = QjsHelper::getInstance().getContext();
  JSValue args[] = { QjsKeyEvent::Wrap(ctx, const_cast<KeyEvent*>(&key_event)), environment_.get() };
  JSValueRAII jsResult(JS_Call(ctx, processFunc_, JS_UNDEFINED, 2, args));
  if (JS_IsException(jsResult)) {
    return kNoop;
  }

  int32_t result = 0;
  JS_ToInt32(ctx, &result, jsResult);

  switch (result) {
  case 0: return kRejected;
  case 1: return kAccepted;
  default: return kNoop;
  }
}

}  // namespace rime
