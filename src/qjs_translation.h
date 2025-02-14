#ifndef RIME_QJS_TRANSLATION_H_
#define RIME_QJS_TRANSLATION_H_

#include <rime/translation.h>

#include "jsvalue_raii.h"
#include "qjs_iterator.h"
#include "qjs_candidate.h"
#include "quickjs.h"

namespace rime {

class QuickJSTranslation : public PrefetchTranslation {
 public:
  QuickJSTranslation(an<Translation> translation, JSContext* ctx, const JSValueRAII& filterFunc);
  ~QuickJSTranslation() override;

 protected:
  bool Replenish() override { return replenished_; }

 private:
  bool DoFilter(JSContext* ctx, const JSValueRAII& filterFunc);

  bool replenished_ = false;

};

} // namespace rime

#endif  // RIME_QJS_TRANSLATION_H_
