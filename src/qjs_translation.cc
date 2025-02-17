#include "qjs_translation.h"
#include "qjs_candidate.h"

#include <rime/translation.h>

namespace rime {

QuickJSTranslation::QuickJSTranslation(an<Translation> translation,
                                       const JSValueRAII& filterFunc,
                                       const JSValue& environment)
    : PrefetchTranslation(translation) {

  DoFilter(filterFunc, environment);
  replenished_ = true;
  set_exhausted(cache_.empty());
}

bool QuickJSTranslation::DoFilter(const JSValueRAII& filterFunc, const JSValue& environment) {
  auto ctx = QjsHelper::getInstance().getContext();
  JSValueRAII jsArray(JS_NewArray(ctx));
  size_t idx = 0;
  while (!translation_->exhausted()) {
    an<Candidate> candidate = translation_->Peek();
    if (!candidate) {
      break;
    }

    translation_->Next();

    JSValueRAII jsObjectRAII(QjsCandidate::Wrap(ctx, candidate));
    // Use dup() to create a new reference for the array
    JS_SetPropertyUint32(ctx, jsArray, idx++, jsObjectRAII.dup());
  }
  if (idx == 0) {
    return true;
  }

  JSValueRAII resultArray(JS_Call(ctx, filterFunc, JS_UNDEFINED, 2, (JSValueConst[]){jsArray, environment}));
  if (JS_IsException(resultArray)) {
    return false;
  }

  JSValueRAII lengthVal(JS_GetPropertyStr(ctx, resultArray, "length"));
  uint32_t length;
  JS_ToUint32(ctx, &length, lengthVal);

  for (uint32_t i = 0; i < length; i++) {
    JSValueRAII item(JS_GetPropertyUint32(ctx, resultArray, i));
    if (an<Candidate> candidate = QjsCandidate::Unwrap(ctx, item)) {
      cache_.push_back(candidate);
    } else {
      LOG(ERROR) << "[qjs] Failed to unwrap candidate at index " << i;
    }
  }

  return true;
}

QuickJSTranslation::~QuickJSTranslation() {
}

}  // namespace rime
