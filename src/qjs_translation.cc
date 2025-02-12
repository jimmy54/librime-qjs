#include "qjs_translation.h"
#include "qjs_candidate.h"

#include <rime/translation.h>


namespace rime {

QuickJSTranslation::QuickJSTranslation(an<Translation> translation,
                                     JSContext* ctx)
    : PrefetchTranslation(translation), ctx_(ctx) {
  JSValueRAII::context_ = ctx_;  // Set the static context
  FilterByJS();
  replenished_ = true;
  set_exhausted(cache_.empty());
}

bool QuickJSTranslation::FilterByJS() {
  std::map<Candidate*, an<Candidate>> mapCandidates;
  JSValueRAII jsArray(JS_NewArray(ctx_));

  size_t idx = 0;
  while (!translation_->exhausted()) {
    an<Candidate> candidate = translation_->Peek();
    if (!candidate) {
      break;
    }

    translation_->Next();
    mapCandidates.insert({candidate.get(), candidate});

    JSValueRAII jsObjectRAII(QjsCandidate::Wrap(ctx_, candidate));
    // Use dup() to create a new reference for the array
    JS_SetPropertyUint32(ctx_, jsArray, idx, jsObjectRAII.dup());

    idx++;
  }
  if (idx == 0) {
    return true;
  }

  constexpr std::string_view script = R"(
        function filterCandidates(candidates) {
            return candidates
              .filter((it, idx) => idx % 2 === 1)
        }
    )";

  JSValueRAII result(JS_Eval(ctx_, script.data(), script.size(), "<input>", JS_EVAL_TYPE_GLOBAL));
  if (JS_IsException(result)) {
    LOG(ERROR) << "[qjs] Exception during script evaluation";
    return false;
  }

  JSValueRAII global(JS_GetGlobalObject(ctx_));
  JSValueRAII filter_func(JS_GetPropertyStr(ctx_, global, "filterCandidates"));
  JSValue resultArray = JS_Call(ctx_, filter_func, global, 1, (JSValueConst[]){jsArray});

  if (JS_IsException(resultArray)) {
    LOG(ERROR) << "[qjs] Exception during filter function call";
    return false;
  }

  JSValueRAII lengthVal(JS_GetPropertyStr(ctx_, resultArray, "length"));
  uint32_t length;
  JS_ToUint32(ctx_, &length, lengthVal);

  for (uint32_t i = 0; i < length; i++) {
    JSValueRAII item(JS_GetPropertyUint32(ctx_, resultArray, i));
    if (!JS_IsException(item)) {
      if (Candidate* candidate = QjsCandidate::Unwrap(ctx_, item)) {
        if (auto it = mapCandidates.find(candidate); it != mapCandidates.end()) {
          cache_.push_back(it->second);
        } else {
          LOG(ERROR) << "[qjs] Candidate not found in map, ptr=" << candidate;
        }
      } else {
        LOG(ERROR) << "[qjs] Failed to unwrap candidate";
      }
    } else {
      LOG(ERROR) << "[qjs] Exception while getting array item " << i;
    }
  }

  return true;
}

QuickJSTranslation::~QuickJSTranslation() {
}

}  // namespace rime
