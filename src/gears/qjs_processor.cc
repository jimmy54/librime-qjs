#include "qjs_processor.h"

#include <rime/gear/translator_commons.h>
#include <rime/translation.h>

#include "jsvalue_raii.hpp"
#include "qjs_key_event.h"
#include "quickjs.h"

namespace rime {

ProcessResult QuickJSProcessor::processKeyEvent(const KeyEvent& keyEvent,
                                                const JSValue& environment) {
  if (!isLoaded()) {
    return kNoop;
  }

  auto* ctx = QjsHelper::getInstance().getContext();
  // NOLINTNEXTLINE(cppcoreguidelines-pro-type-const-cast)
  JSValueRAII jsKeyEvt(QjsKeyEvent::wrap(ctx, const_cast<KeyEvent*>(&keyEvent)));
  JSValue args[] = {jsKeyEvt.get(), environment};
  JSValueRAII jsResult = JS_Call(ctx, getMainFunc(), getInstance(), 2, static_cast<JSValue*>(args));
  if (JS_IsException(jsResult)) {
    return kNoop;
  }

  // do not wrap it in JSStringRAII, because it would be freed by JSValueRAII jsResult
  std::string result = JS_ToCString(ctx, jsResult);
  if (result == "kNoop") {
    return kNoop;
  }
  if (result == "kAccepted") {
    return kAccepted;
  }
  if (result == "kRejected") {
    return kRejected;
  }

  LOG(ERROR) << "[qjs] " << getNamespace() << "::ProcessKeyEvent unknown result: " << result;
  return kNoop;
}

}  // namespace rime
