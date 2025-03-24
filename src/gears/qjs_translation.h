#ifndef RIME_QJS_TRANSLATION_H_
#define RIME_QJS_TRANSLATION_H_

#include <rime/translation.h>

#include "quickjs.h"

namespace rime {

class QuickJSTranslation : public PrefetchTranslation {
public:
  QuickJSTranslation(const QuickJSTranslation&) = delete;
  QuickJSTranslation(QuickJSTranslation&&) = delete;
  QuickJSTranslation& operator=(const QuickJSTranslation&) = delete;
  QuickJSTranslation& operator=(QuickJSTranslation&&) = delete;

  QuickJSTranslation(an<Translation> translation,
                     const JSValue& filterObj,
                     const JSValue& filterFunc,
                     const JSValue& environment);
  ~QuickJSTranslation() override = default;

protected:
  bool Replenish() override { return replenished_; }

private:
  bool doFilter(const JSValue& filterObj, const JSValue& filterFunc, const JSValue& environment);
  bool replenished_ = false;
};

}  // namespace rime

#endif  // RIME_QJS_TRANSLATION_H_
