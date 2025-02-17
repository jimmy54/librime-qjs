#ifndef RIME_QJS_COMPONENT_H_
#define RIME_QJS_COMPONENT_H_

#include <rime/common.h>
#include <rime/component.h>
#include <rime/ticket.h>

namespace rime {

template<typename T>
class QuickJSComponent : public T::Component {
public:
  QuickJSComponent(const string& jsDirectory) : jsDirectory_(jsDirectory) {}

  T* Create(const Ticket& a) {
    // Create a new ticket with the same namespace for both schema and config
    Ticket t(a.engine, a.name_space, a.name_space);
    return new T(t, jsDirectory_);
  }

private:
  const string jsDirectory_;
};

} // namespace rime

#endif  // RIME_QJS_COMPONENT_H_
