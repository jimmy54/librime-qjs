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
  QuickJSComponent(JSRuntime* rt, JSContext* ctx) : rt_(rt), ctx_(ctx) {}

  T* Create(const Ticket& a) {
    // Create a new ticket with the same namespace for both schema and config
    Ticket t(a.engine, a.name_space, a.name_space);
    return new T(t, rt_, ctx_);
  }

private:
  JSRuntime* rt_;    // QuickJS runtime
  JSContext* ctx_;   // QuickJS context
};

} // namespace rime

#endif  // RIME_QJS_COMPONENT_H_
