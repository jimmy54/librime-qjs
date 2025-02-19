#ifndef RIME_QJS_COMPONENT_H_
#define RIME_QJS_COMPONENT_H_

#include <rime/common.h>
#include <rime/component.h>
#include <rime/ticket.h>
#include <map>

namespace rime {

template<typename T>
class QuickJSComponent : public T::Component {
public:
  QuickJSComponent() {}

  T* Create(const Ticket& a) {
    if (!components_.count(a.name_space)) {
      LOG(INFO) << "[qjs] creating component '" << a.name_space << "'.";
      Ticket t(a.engine, a.name_space, a.name_space);
      components_[t.name_space] = new T(t);
    }
    return components_[a.name_space];
  }

private:
  std::map<string, T*> components_;
};

} // namespace rime

#endif  // RIME_QJS_COMPONENT_H_
