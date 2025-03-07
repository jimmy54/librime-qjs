#include "qjs_processor.h"
#include "qjs_key_event.h"
#include "jsvalue_raii.h"

#include <rime/translation.h>
#include <rime/gear/translator_commons.h>

namespace rime {

ProcessResult QuickJSProcessor::ProcessKeyEvent(const KeyEvent& key_event) {
  if (!isLoaded_) {
    return kNoop;
  }

  auto ctx = QjsHelper::getInstance().getContext();
  JSValue args[] = { QjsKeyEvent::Wrap(ctx, const_cast<KeyEvent*>(&key_event)), environment_.get() };
  JSValueRAII jsResult(JS_Call(ctx, mainFunc_, JS_UNDEFINED, 2, args));
  if (JS_IsException(jsResult)) {
    return kNoop;
  }

  // do not wrap it in JSStringRAII, because it would be freed by JSValueRAII jsResult
  const char* result = JS_ToCString(ctx, jsResult);
  if (strncmp(result, "kNoop", 5) == 0) {
    return kNoop;
  } else if (strncmp(result, "kAccepted", 9) == 0) {
    return kAccepted;
  } else if (strncmp(result, "kRejected", 9) == 0) {
    return kRejected;
  } else {
    LOG(ERROR) << "[qjs] " << name_space_ << "::ProcessKeyEvent unknown result: " << result;
    return kNoop;
  }
}

}  // namespace rime
