#include "qjs_translator.h"
#include "jsvalue_raii.h"
#include "qjs_helper.h"
#include "qjs_engine.h"
#include "qjs_segment.h"
#include "qjs_candidate.h"

#include <fstream>
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

  environment_ = JSValueRAII(JS_NewObject(ctx)); // do not free its properties/methods manually
  JS_SetPropertyStr(ctx, environment_, "engine", QjsEngine::Wrap(ctx, engine_));
  JS_SetPropertyStr(ctx, environment_, "namespace", JS_NewString(ctx, name_space_.c_str()));
  JS_SetPropertyStr(ctx, environment_, "userDataDir", JS_NewString(ctx, QjsHelper::basePath.c_str()));
  // JS_SetPropertyStr(ctx, environment_, "loadFile", JS_NewCFunction(ctx, loadFile, "loadFile", 1));
  // JS_SetPropertyStr(ctx, environment_, "fileExists", JS_NewCFunction(ctx, fileExists, "fileExists", 1));

  JSValueRAII initFunc(JS_GetPropertyStr(ctx, moduleNamespace, "init"));
  if (!JS_IsUndefined(initFunc)) {
    JSValueRAII initResult(JS_Call(ctx, initFunc, JS_UNDEFINED, 1, environment_.getPtr()));
    if (JS_IsException(initResult)) {
      LOG(ERROR) << "[qjs] QuickJSTranslator Error running the init function in " << fileName;
      return;
    }
  } else {
    DLOG(INFO) << "[qjs] QuickJSTranslator no `init` function exported in " << fileName;
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
    JSValueRAII finitResult(JS_Call(QjsHelper::getInstance().getContext(),
                                    finitFunc_,
                                    JS_UNDEFINED,
                                    1,
                                    environment_.getPtr()));
    if (JS_IsException(finitResult)) {
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
