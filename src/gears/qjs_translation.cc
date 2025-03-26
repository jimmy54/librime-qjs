#include "qjs_translation.h"

#include <rime/translation.h>

#include <utility>

#include "jsvalue_raii.hpp"
#include "qjs_candidate.h"
#include "quickjs.h"

namespace rime {

QuickJSTranslation::QuickJSTranslation(an<Translation> translation,
                                       const JSValue& filterObj,
                                       const JSValue& filterFunc,
                                       const JSValue& environment)
    : PrefetchTranslation(std::move(translation)), replenished_(true) {
  doFilter(filterObj, filterFunc, environment);
  set_exhausted(cache_.empty());
}

bool QuickJSTranslation::doFilter(const JSValue& filterObj,
                                  const JSValue& filterFunc,
                                  const JSValue& environment) {
  auto* ctx = QjsHelper::getInstance().getContext();
  JSValueRAII jsArray(JS_NewArray(ctx));
  size_t idx = 0;
  while (auto candidate = translation_->exhausted() ? nullptr : translation_->Peek()) {
    translation_->Next();
    JS_SetPropertyUint32(ctx, jsArray, idx++, QjsCandidate::wrap(ctx, candidate));
  }
  if (idx == 0) {
    return true;
  }

  JSValueConst args[] = {jsArray, environment};
  JSValueRAII resultArray = JS_Call(ctx, filterFunc, filterObj, 2, static_cast<JSValue*>(args));
  if (JS_IsException(resultArray)) {
    return false;
  }

  JSValueRAII lengthVal = JS_GetPropertyStr(ctx, resultArray, "length");
  if (JS_IsException(lengthVal)) {
    return false;
  }
  uint32_t length = 0;
  JS_ToUint32(ctx, &length, lengthVal);

  for (uint32_t i = 0; i < length; i++) {
    JSValueRAII item(JS_GetPropertyUint32(ctx, resultArray, i));
    if (an<Candidate> candidate = QjsCandidate::unwrap(ctx, item)) {
      cache_.push_back(candidate);
    } else {
      LOG(ERROR) << "[qjs] Failed to unwrap candidate at index " << i;
    }
  }

  return true;
}

}  // namespace rime
