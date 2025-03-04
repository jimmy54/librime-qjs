#include "qjs_translator.h"
#include "jsvalue_raii.h"
#include "qjs_helper.h"
#include "qjs_engine.h"
#include "qjs_segment.h"
#include "qjs_candidate.h"
#include "helpers/qjs_environment.h"

#include <fstream>
#include <rime/context.h>
#include <rime/composition.h>
#include <rime/translation.h>
#include <rime/gear/translator_commons.h>

namespace rime {

QuickJSTranslator::QuickJSTranslator(const Ticket& ticket)
    : Translator(ticket) {
  auto ctx = QjsHelper::getInstance().getContext();
  std::string fileName = ticket.name_space + ".js";
  JSValueRAII moduleNamespace(QjsHelper::loadJsModuleToNamespace(ctx, fileName.c_str()));
  if (JS_IsUndefined(moduleNamespace) || JS_IsException(moduleNamespace)) {
    LOG(ERROR) << "[qjs] QuickJSTranslator Could not load " << fileName;
    return;
  }

  environment_ = QjsEnvironment::Create(ctx, engine_, name_space_);

  if (!QjsEnvironment::CallInitFunction(ctx, moduleNamespace, environment_)) {
    LOG(ERROR) << "[qjs] QuickJSTranslator Error running the init function in " << fileName;
    return;
  } else {
    DLOG(INFO) << "[qjs] QuickJSTranslator init function executed successfully in " << fileName;
  }

  finitFunc_ = JSValueRAII(JS_GetPropertyStr(ctx, moduleNamespace, "finit"));
  translateFunc_ = JSValueRAII(JS_GetPropertyStr(ctx, moduleNamespace, "translate"));
  if (JS_IsUndefined(translateFunc_)) {
    LOG(ERROR) << "[qjs] QuickJSTranslator No `translate` function exported in " << fileName;
  } else {
    isLoaded_ = true;
  }
}

QuickJSTranslator::~QuickJSTranslator() {
  if (!JS_IsUndefined(finitFunc_)) {
    DLOG(INFO) << "[qjs] QuickJSTranslator::~QuickJSTranslator running the finit function";
    auto ctx = QjsHelper::getInstance().getContext();
    if (!QjsEnvironment::CallFinitFunction(ctx, finitFunc_, environment_)) {
      LOG(ERROR) << "[qjs] ~QuickJSTranslator Error running the finit function.";
    }
  } else {
    DLOG(INFO) << "[qjs] ~QuickJSTranslator no `finit` function exported.";
  }
}

an<Translation> QuickJSTranslator::Query(const string& input,
                                         const Segment& segment) {
  auto translation = New<FifoTranslation>();
  if (!isLoaded_) {
    return translation;
  }

  // TODO: the composition is always empty in the qjs processors, find the root cause and fix it.
  // Temporary workaround: add the segment to the context and use it in the qjs processors.
  if (engine_->context()->composition().empty()) {
    DLOG(INFO) << "[qjs] QuickJSTranslator::Query adding segment to context for later usage in the qjs processors.";
    engine_->context()->composition().AddSegment(segment);
  }

  auto ctx = QjsHelper::getInstance().getContext();
  JSValueRAII jsInput(JS_NewString(ctx, input.c_str()));
  JSValue args[] = { jsInput.get(), QjsSegment::Wrap(ctx, const_cast<Segment*>(&segment)), environment_.get() };
  JSValueRAII resultArray(JS_Call(ctx, translateFunc_, JS_UNDEFINED, 3, args));
  if (JS_IsException(resultArray)) {
    return translation;
  }

  JSValueRAII lengthVal(JS_GetPropertyStr(ctx, resultArray, "length"));
  uint32_t length;
  JS_ToUint32(ctx, &length, lengthVal);

  for (uint32_t i = 0; i < length; i++) {
    JSValueRAII item(JS_GetPropertyUint32(ctx, resultArray, i));
    if (an<Candidate> candidate = QjsCandidate::Unwrap(ctx, item)) {
      translation->Append(candidate);
    } else {
      LOG(ERROR) << "[qjs] Failed to unwrap candidate at index " << i;
    }
  }

  return translation;
}

}  // namespace rime
