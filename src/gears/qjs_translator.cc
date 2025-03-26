#include "qjs_translator.h"

#include <rime/composition.h>
#include <rime/context.h>
#include <rime/gear/translator_commons.h>
#include <rime/translation.h>

#include "jsvalue_raii.hpp"
#include "qjs_candidate.h"
#include "qjs_helper.h"
#include "qjs_segment.h"
#include "quickjs.h"

namespace rime {

an<Translation> QuickJSTranslator::query(const string& input,
                                         const Segment& segment,
                                         const JSValue& environment) {
  auto translation = New<FifoTranslation>();
  if (!isLoaded()) {
    return translation;
  }

  auto* ctx = QjsHelper::getInstance().getContext();
  JSValueRAII jsInput(JS_NewString(ctx, input.c_str()));
  // NOLINTNEXTLINE(cppcoreguidelines-pro-type-const-cast)
  JSValueRAII jsSegment(QjsSegment::wrap(ctx, const_cast<Segment*>(&segment)));
  JSValue args[] = {jsInput.get(), jsSegment.get(), environment};
  JSValueRAII resultArray =
      JS_Call(ctx, getMainFunc(), getInstance(), countof(args), static_cast<JSValue*>(args));
  if (JS_IsException(resultArray)) {
    return translation;
  }

  JSValueRAII lengthVal = JS_GetPropertyStr(ctx, resultArray, "length");
  if (JS_IsException(lengthVal)) {
    return translation;
  }
  uint32_t length = 0;
  JS_ToUint32(ctx, &length, lengthVal);

  for (uint32_t i = 0; i < length; i++) {
    JSValueRAII item(JS_GetPropertyUint32(ctx, resultArray, i));
    if (an<Candidate> candidate = QjsCandidate::unwrap(ctx, item)) {
      translation->Append(candidate);
    } else {
      LOG(ERROR) << "[qjs] Failed to unwrap candidate at index " << i;
    }
  }

  return translation;
}

}  // namespace rime
