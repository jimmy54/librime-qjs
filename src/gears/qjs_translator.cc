#include "qjs_translator.h"
#include "jsvalue_raii.h"
#include "qjs_helper.h"
#include "qjs_segment.h"
#include "qjs_candidate.h"

#include <rime/context.h>
#include <rime/composition.h>
#include <rime/translation.h>
#include <rime/gear/translator_commons.h>

namespace rime {

an<Translation> QuickJSTranslator::Query(const string& input, const Segment& segment) {
  auto translation = New<FifoTranslation>();
  if (!isLoaded()) {
    return translation;
  }

  // TODO: the composition is always empty in the qjs processors, find the root cause and fix it.
  // Temporary workaround: add the segment to the context and use it in the qjs processors.
  if (engine_->context()->composition().empty()) {
    DLOG(INFO) << "[qjs] QuickJSTranslator::Query adding segment to context for later usage in the qjs processors.";
    engine_->context()->composition().AddSegment(segment);
  }

  auto *ctx = QjsHelper::getInstance().getContext();
  JSValueRAII jsInput(JS_NewString(ctx, input.c_str()));
  // NOLINTNEXTLINE(cppcoreguidelines-pro-type-const-cast)
  JSValueRAII jsSegment(QjsSegment::Wrap(ctx, const_cast<Segment*>(&segment)));
  JSValue args[] = { jsInput.get(), jsSegment.get(), getEnvironment() };
  JSValueRAII resultArray(JS_Call(ctx, getMainFunc(), getInstance(), countof(args), static_cast<JSValue*>(args)));
  if (JS_IsException(resultArray)) {
    return translation;
  }

  JSValueRAII lengthVal(JS_GetPropertyStr(ctx, resultArray, "length"));
  uint32_t length = 0;
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
