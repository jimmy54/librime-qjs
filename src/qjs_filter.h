#ifndef RIME_QJS_FILTER_H_
#define RIME_QJS_FILTER_H_

#include <rime/filter.h>
#include <rime/gear/filter_commons.h>
#include "quickjs.h"
#include "jsvalue_raii.h"

namespace rime {

class QuickJSFilter : public Filter {
public:
  explicit QuickJSFilter(const Ticket& ticket);
  virtual ~QuickJSFilter();

  virtual an<Translation> Apply(an<Translation> translation,
                                CandidateList* candidates);

private:
  JSValueRAII filterFunc_{JS_UNDEFINED};
  JSValueRAII finitFunc_{JS_UNDEFINED};
  JSValueRAII environment_{JS_UNDEFINED};

  bool isLoaded_ = false;
};

} // namespace rime

#endif  // RIME_QJS_FILTER_H_
