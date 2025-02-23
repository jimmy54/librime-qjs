#ifndef RIME_QJS_COMPONENT_H_
#define RIME_QJS_COMPONENT_H_

#include <rime/common.h>
#include <rime/component.h>
#include <rime/filter.h>
#include <rime/ticket.h>
#include <rime/gear/filter_commons.h>
#include <map>

namespace rime {

template<typename T_ACTUAL, typename T_WRAPPER>
class QuickJSComponent : public T_WRAPPER::Component {
public:
  QuickJSComponent() {}

  T_WRAPPER* Create(const Ticket& a) {
    if (!components_.count(a.name_space)) {
      LOG(INFO) << "[qjs] creating component '" << a.name_space << "'.";
      components_[a.name_space] = New<T_ACTUAL>(a);
    } else {
      // The previous engine might be freed in switching input methods
      // update the new engine to the component
      components_[a.name_space]->setEngine(a.engine);
    }
    return new T_WRAPPER(a, components_[a.name_space]);
  }

private:
  std::map<string, an<T_ACTUAL>> components_;
};

template<typename T>
class FilterWrapper : public Filter {
public:
  explicit FilterWrapper(const Ticket& ticket, an<T>& actualFilter_)
      : Filter(ticket), actualFilter_(actualFilter_) {
    DLOG(INFO) << "FilterWrapper created with ticket: " << ticket.name_space;
  };
  virtual ~FilterWrapper() {
    DLOG(INFO) << "FilterWrapper destroyed";
  }
  virtual an<Translation> Apply(an<Translation> translation,
                                CandidateList* candidates) {
    return actualFilter_->Apply(translation, candidates);
  }

  an<T> actualFilter_;
};


} // namespace rime

#endif  // RIME_QJS_COMPONENT_H_
