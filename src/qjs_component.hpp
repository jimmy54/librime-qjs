#pragma once

#include <rime/common.h>
#include <rime/component.h>
#include <rime/engine.h>
#include <rime/schema.h>
#include <rime/ticket.h>
#include <rime/translator.h>

#include <map>
#include <memory>
#include <utility>
#include "environment.h"

#include "engines/engine_manager.h"

namespace rime {

// Primary template declaration
template <typename T_ACTUAL, typename T_BASE, typename T_JS_VALUE>
class ComponentWrapper;

// Base class for all ComponentWrapper specializations
template <typename T_ACTUAL, typename T_BASE, typename T_JS_VALUE>
class ComponentWrapperBase : public T_BASE {
public:
  std::shared_ptr<T_ACTUAL> actual() { return actual_; }
  T_JS_VALUE environment() { return environment_; }

  ComponentWrapperBase(const ComponentWrapperBase&) = delete;
  ComponentWrapperBase& operator=(const ComponentWrapperBase&) = delete;
  ComponentWrapperBase(ComponentWrapperBase&&) = delete;
  ComponentWrapperBase& operator=(ComponentWrapperBase&&) = delete;

protected:
  explicit ComponentWrapperBase(const rime::Ticket& ticket,
                                const std::shared_ptr<T_ACTUAL>& actual,
                                const T_JS_VALUE& environment)
      : T_BASE(ticket), actual_(actual), environment_(environment) {
    DLOG(INFO) << "[qjs] " << typeid(T_ACTUAL).name()
               << " ComponentWrapper created with ticket: " << ticket.name_space;
  }

  virtual ~ComponentWrapperBase() {
    DLOG(INFO) << "[qjs] " << typeid(T_ACTUAL).name() << " ComponentWrapper destroyed";

    auto& engine = getJsEngine<T_JS_VALUE>();
    engine.freeValue(environment_);
  }

private:
  const T_JS_VALUE environment_;
  const std::shared_ptr<T_ACTUAL>& actual_;
};

template <typename T_ACTUAL, typename T_BASE, typename T_JS_VALUE>
class QuickJSComponent : public T_BASE::Component {
  using KeyType = std::pair<std::string, std::string>;
  // using T_JS_OBJECT = typename TypeMap<T_JS_VALUE>::ObjectType;

public:
  // NOLINTNEXTLINE(readability-identifier-naming)
  ComponentWrapper<T_ACTUAL, T_BASE, T_JS_VALUE>* Create(const rime::Ticket& a) {
    auto environment = std::make_shared<Environment>(a.engine, a.name_space);
    auto& engine = getJsEngine<T_JS_VALUE>();
    T_JS_VALUE jsEnvironment = engine.wrapShared(environment);

    // The same plugin could have difference configurations for different schemas, and then behave differently.
    // So we need to create a new component for each schema.
    const std::string& schemaId = a.engine->schema()->schema_id();
    KeyType key = std::make_pair(schemaId, a.name_space);
    if (!components_.count(key)) {
      LOG(INFO) << "[qjs] creating component '" << a.name_space << "' for schema " << schemaId;
      components_[key] = std::make_shared<T_ACTUAL>(a, jsEnvironment);
    }

    return new ComponentWrapper<T_ACTUAL, T_BASE, T_JS_VALUE>(a, components_[key], jsEnvironment);
  }

private:
  std::map<KeyType, std::shared_ptr<T_ACTUAL>> components_;
};

}  // namespace rime
