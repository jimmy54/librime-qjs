#ifndef RIME_QJS_COMPONENT_H_
#define RIME_QJS_COMPONENT_H_

#include <rime/common.h>
#include <rime/component.h>
#include <rime/filter.h>
#include <rime/gear/filter_commons.h>
#include <rime/gear/translator_commons.h>
#include <rime/processor.h>
#include <rime/ticket.h>
#include <rime/translator.h>

#include <map>

namespace rime {

// Primary template declaration
template <typename T_ACTUAL, typename T_BASE>
class ComponentWrapper;

// Base class for all ComponentWrapper specializations
template <typename T_ACTUAL, typename T_BASE>
class ComponentWrapperBase : public T_BASE {
public:
  explicit ComponentWrapperBase(const Ticket& ticket, an<T_ACTUAL>& actual)
      : T_BASE(ticket), actual_(actual) {
    DLOG(INFO) << "[qjs] " << typeid(T_BASE).name()
               << " ComponentWrapper created with ticket: " << ticket.name_space;
  }

  virtual ~ComponentWrapperBase() {
    DLOG(INFO) << "[qjs] " << typeid(T_BASE).name() << " ComponentWrapper destroyed";
  }

  // Delete copy constructor and assignment operator
  ComponentWrapperBase(const ComponentWrapperBase&) = delete;
  ComponentWrapperBase& operator=(const ComponentWrapperBase&) = delete;
  // Delete move constructor and assignment operator
  ComponentWrapperBase(ComponentWrapperBase&&) = delete;
  ComponentWrapperBase& operator=(ComponentWrapperBase&&) = delete;

  an<T_ACTUAL> actual() { return actual_; }

protected:
  an<T_ACTUAL> actual_;
};

// Specialization for Filter
template <typename T_ACTUAL>
class ComponentWrapper<T_ACTUAL, Filter> : public ComponentWrapperBase<T_ACTUAL, Filter> {
public:
  using Base = ComponentWrapperBase<T_ACTUAL, Filter>;
  explicit ComponentWrapper(const Ticket& ticket, an<T_ACTUAL>& actual) : Base(ticket, actual) {}

  virtual an<Translation> Apply(an<Translation> translation, CandidateList* candidates) {
    return this->actual_->Apply(translation, candidates);
  }
};

// Specialization for Translator
template <typename T_ACTUAL>
class ComponentWrapper<T_ACTUAL, Translator> : public ComponentWrapperBase<T_ACTUAL, Translator> {
public:
  using Base = ComponentWrapperBase<T_ACTUAL, Translator>;
  explicit ComponentWrapper(const Ticket& ticket, an<T_ACTUAL>& actual) : Base(ticket, actual) {}

  virtual an<Translation> Query(const string& input, const Segment& segment) {
    return this->actual_->Query(input, segment);
  }
};

// Specialization for Processor
template <typename T_ACTUAL>
class ComponentWrapper<T_ACTUAL, Processor> : public ComponentWrapperBase<T_ACTUAL, Processor> {
public:
  using Base = ComponentWrapperBase<T_ACTUAL, Processor>;
  explicit ComponentWrapper(const Ticket& ticket, an<T_ACTUAL>& actual) : Base(ticket, actual) {}

  ProcessResult ProcessKeyEvent(const KeyEvent& keyEvent) override {
    return this->actual_->ProcessKeyEvent(keyEvent);
  }
};

template <typename T_ACTUAL, typename T_BASE>
class QuickJSComponent : public T_BASE::Component {
public:
  QuickJSComponent() = default;

  // NOLINTNEXTLINE(readability-identifier-naming)
  ComponentWrapper<T_ACTUAL, T_BASE>* Create(const Ticket& a) {
    if (!components_.count(a.name_space)) {
      LOG(INFO) << "[qjs] creating component '" << a.name_space << "'.";
      components_[a.name_space] = New<T_ACTUAL>(a);
    } else {
      // The previous engine might be freed in switching input methods
      // update the new engine to the component
      components_[a.name_space]->setEngine(a.engine);
    }
    return new ComponentWrapper<T_ACTUAL, T_BASE>(a, components_[a.name_space]);
  }

private:
  std::map<string, an<T_ACTUAL>> components_;
};

}  // namespace rime

#endif  // RIME_QJS_COMPONENT_H_
