#ifndef RIME_QJS_COMPONENT_H_
#define RIME_QJS_COMPONENT_H_

#include <rime/common.h>
#include <rime/component.h>
#include <rime/engine.h>
#include <rime/schema.h>
#include <rime/ticket.h>
#include <rime/translator.h>

#include <map>
#include <utility>
#include "jsvalue_raii.h"
#include "qjs_environment.h"
#include "qjs_helper.h"

namespace rime {

// Primary template declaration
template <typename T_ACTUAL, typename T_BASE>
class ComponentWrapper;

// Base class for all ComponentWrapper specializations
template <typename T_ACTUAL, typename T_BASE>
class ComponentWrapperBase : public T_BASE {
public:
  an<T_ACTUAL> actual() { return actual_; }

protected:
  explicit ComponentWrapperBase(const Ticket& ticket,
                                const an<T_ACTUAL>& actual,
                                const JSValue& environment)
      : T_BASE(ticket), actual_(actual), environment_(environment) {
    DLOG(INFO) << "[qjs] " << typeid(T_ACTUAL).name()
               << " ComponentWrapper created with ticket: " << ticket.name_space;
  }

  virtual ~ComponentWrapperBase() {
    DLOG(INFO) << "[qjs] " << typeid(T_ACTUAL).name() << " ComponentWrapper destroyed";
  }

  // Delete copy constructor and assignment operator
  ComponentWrapperBase(const ComponentWrapperBase&) = delete;
  ComponentWrapperBase& operator=(const ComponentWrapperBase&) = delete;
  // Delete move constructor and assignment operator
  ComponentWrapperBase(ComponentWrapperBase&&) = delete;
  ComponentWrapperBase& operator=(ComponentWrapperBase&&) = delete;

  const JSValueRAII environment_;
  const an<T_ACTUAL>& actual_;
};

template <typename T_ACTUAL, typename T_BASE>
class QuickJSComponent : public T_BASE::Component {
  using KeyType = std::pair<std::string, std::string>;

public:
  QuickJSComponent() = default;

  // NOLINTNEXTLINE(readability-identifier-naming)
  ComponentWrapper<T_ACTUAL, T_BASE>* Create(const Ticket& a) {
    auto* ctx = QjsHelper::getInstance().getContext();
    JSValue environment = QjsEnvironment::create(ctx, a.engine, a.name_space);

    // The same plugin could have difference configurations for different schemas, and then behave differently.
    // So we need to create a new component for each schema.
    auto& schemaId = a.engine->schema()->schema_id();
    KeyType key = std::make_pair(schemaId, a.name_space);
    if (!components_.count(key)) {
      LOG(INFO) << "[qjs] creating component '" << a.name_space << "' for schema " << schemaId;
      components_[key] = New<T_ACTUAL>(a, environment);
    }

    return new ComponentWrapper<T_ACTUAL, T_BASE>(a, components_[key], environment);
  }

private:
  std::map<KeyType, an<T_ACTUAL>> components_;
};

}  // namespace rime

#endif  // RIME_QJS_COMPONENT_H_
