#ifndef RIME_QJS_TRANSLATION_H_
#define RIME_QJS_TRANSLATION_H_

#include <rime/translation.h>

#include "jsvalue_raii.h"
#include "qjs_candidate.h"
#include "quickjs.h"

namespace rime {

class QuickJSTranslation : public PrefetchTranslation {
 public:
  QuickJSTranslation(an<Translation> translation,
                     const JSValueRAII& filterFunc,
                     const JSValue& environment);
  ~QuickJSTranslation() override;

 protected:
  bool Replenish() override { return replenished_; }

 private:
  bool DoFilter(const JSValueRAII& filterFunc, const JSValue& environment);
  bool replenished_ = false;
};

} // namespace rime

#endif  // RIME_QJS_TRANSLATION_H_
