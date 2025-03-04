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

  int32_t result = 0;
  JS_ToInt32(ctx, &result, jsResult);

  switch (result) {
  case 0: return kRejected;
  case 1: return kAccepted;
  case 2: return kNoop;
  default:
    LOG(ERROR) << "[qjs] " << name_space_ << "::ProcessKeyEvent unknown result: " << result;
    return kNoop;
  }
}

}  // namespace rime
