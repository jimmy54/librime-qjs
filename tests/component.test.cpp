#include <gtest/gtest.h>
#include <rime/filter.h>
#include <rime/gear/filter_commons.h>
#include <rime/ticket.h>
#include "qjs_component.h"

using namespace rime;

class MockFilter : public Filter {
public:
  explicit MockFilter(const Ticket& ticket): Filter(ticket) {
    LOG(INFO) << "MockFilter created with ticket: " << ticket.name_space;
  };
  virtual ~MockFilter() {
    LOG(INFO) << "MockFilter destroyed";
  }
  void setEngine(Engine* engine) {
    this->engine_ = engine;
  }
  virtual an<Translation> Apply(an<Translation> translation,
                                CandidateList* candidates) {
    return translation;
  }
};

TEST(QuickJSComponentTest, ShareComponentAcrossRimeSessions) {
  QuickJSComponent<MockFilter, FilterWrapper<MockFilter>> component;

  Ticket ticket(nullptr, "test_namespace", "test");
  FilterWrapper<MockFilter>* instance1 = component.Create(ticket);
  auto actualInstance1 = instance1->actualFilter_;
  delete instance1; // Rime session 1 ends

  FilterWrapper<MockFilter>* instance2 = component.Create(ticket);
  ASSERT_EQ(actualInstance1, instance2->actualFilter_)
    << "delete instance1 should not destroy the actual filter instance";
  delete instance2; // Rime session 2 ends

  Ticket ticket2(nullptr, "test_namespace", "test");
  FilterWrapper<MockFilter>* instance3 = component.Create(ticket2);
  ASSERT_EQ(actualInstance1, instance3->actualFilter_)
    << "delete instance1 should not destroy the actual filter instance";
  delete instance3; // Rime session 3 ends
}

TEST(QuickJSComponentTest, CreateComponent) {
  QuickJSComponent<MockFilter, FilterWrapper<MockFilter>> component;

  Ticket ticket(nullptr, "test_namespace", "test");
  auto* instance1 = component.Create(ticket);
  ASSERT_NE(nullptr, instance1);

  auto* instance2 = component.Create(ticket);
  ASSERT_EQ(instance1->actualFilter_, instance2->actualFilter_)
    << "should return the same actual filter with the same ticket";

  Ticket ticket2(nullptr, "test_namespace", "test");
  auto* instance3 = component.Create(ticket2);
  ASSERT_EQ(instance1->actualFilter_, instance3->actualFilter_)
    << "should return the same actual filter with the same ticket namespace";

  Ticket ticket3(nullptr, "test_namespace2", "test");
  auto* instance4 = component.Create(ticket3);
  ASSERT_NE(nullptr, instance4);
  ASSERT_NE(instance1->actualFilter_, instance4->actualFilter_)
    << "should create a new instance with a different ticket namespace";

  delete instance1;
  delete instance2;
  delete instance3;
  delete instance4;
}
