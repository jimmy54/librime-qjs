#ifndef RIME_QJS_COMPONENT_H_
#define RIME_QJS_COMPONENT_H_

#include <rime/common.h>
#include <rime/component.h>
#include <rime/ticket.h>
#include "quickjs.h"

namespace rime {

template<typename T>
class QuickJSComponent : public T::Component {
public:
  QuickJSComponent(JSContext* ctx, const string& jsDirectory) : ctx_(ctx), jsDirectory_(jsDirectory) {}

  T* Create(const Ticket& a) {
    // Create a new ticket with the same namespace for both schema and config
    Ticket t(a.engine, a.name_space, a.name_space);
    return new T(t, ctx_, jsDirectory_);
  }

private:
  JSContext* ctx_;   // QuickJS context
  const string jsDirectory_;
};

} // namespace rime

#endif  // RIME_QJS_COMPONENT_H_
