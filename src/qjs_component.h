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

// Specialization for Filter
template <typename T_ACTUAL>
class ComponentWrapper<T_ACTUAL, Filter> : public Filter {
public:
  explicit ComponentWrapper(const Ticket& ticket, an<T_ACTUAL>& actual)
      : Filter(ticket), actual_(actual) {
    DLOG(INFO) << "Filter ComponentWrapper created with ticket: " << ticket.name_space;
  }

  virtual ~ComponentWrapper() { DLOG(INFO) << "Filter ComponentWrapper destroyed"; }

  // Delete copy constructor and assignment operator
  ComponentWrapper(const ComponentWrapper&) = delete;
  ComponentWrapper& operator=(const ComponentWrapper&) = delete;
  // Delete move constructor and assignment operator
  ComponentWrapper(ComponentWrapper&&) = delete;
  ComponentWrapper& operator=(ComponentWrapper&&) = delete;

  virtual an<Translation> Apply(an<Translation> translation, CandidateList* candidates) {
    return actual_->Apply(translation, candidates);
  }

  an<T_ACTUAL> actual_;
};

// Specialization for Translator
template <typename T_ACTUAL>
class ComponentWrapper<T_ACTUAL, Translator> : public Translator {
public:
  explicit ComponentWrapper(const Ticket& ticket, an<T_ACTUAL>& actual)
      : Translator(ticket), actual_(actual) {
    DLOG(INFO) << "Translator ComponentWrapper created with ticket: " << ticket.name_space;
  }

  virtual ~ComponentWrapper() { DLOG(INFO) << "Translator ComponentWrapper destroyed"; }

  // Delete copy constructor and assignment operator
  ComponentWrapper(const ComponentWrapper&) = delete;
  ComponentWrapper& operator=(const ComponentWrapper&) = delete;
  // Delete move constructor and assignment operator
  ComponentWrapper(ComponentWrapper&&) = delete;
  ComponentWrapper& operator=(ComponentWrapper&&) = delete;

  virtual an<Translation> Query(const string& input, const Segment& segment) {
    return actual_->Query(input, segment);
  }

  an<T_ACTUAL> actual_;
};

// Specialization for Processor
template <typename T_ACTUAL>
class ComponentWrapper<T_ACTUAL, Processor> : public Processor {
public:
  explicit ComponentWrapper(const Ticket& ticket, an<T_ACTUAL>& actual)
      : Processor(ticket), actual_(actual) {
    DLOG(INFO) << "Processor ComponentWrapper created with ticket: " << ticket.name_space;
  }

  virtual ~ComponentWrapper() { DLOG(INFO) << "Processor ComponentWrapper destroyed"; }

  // Delete copy constructor and assignment operator
  ComponentWrapper(const ComponentWrapper&) = delete;
  ComponentWrapper& operator=(const ComponentWrapper&) = delete;
  // Delete move constructor and assignment operator
  ComponentWrapper(ComponentWrapper&&) = delete;
  ComponentWrapper& operator=(ComponentWrapper&&) = delete;

  ProcessResult ProcessKeyEvent(const KeyEvent& keyEvent) override {
    return actual_->ProcessKeyEvent(keyEvent);
  }

  an<T_ACTUAL> actual_;
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
