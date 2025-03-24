#include <gtest/gtest.h>
#include <rime/filter.h>
#include <rime/gear/filter_commons.h>
#include <rime/ticket.h>

#include "qjs_component.h"
#include "qjs_filter.h"
#include "quickjs.h"

using namespace rime;

class MockFilter {
public:
  MockFilter(const MockFilter&) = delete;
  MockFilter(MockFilter&&) = delete;
  MockFilter& operator=(const MockFilter&) = delete;
  MockFilter& operator=(MockFilter&&) = delete;

  explicit MockFilter(const Ticket& ticket, JSValue& environment) {
    LOG(INFO) << "MockFilter created with ticket: " << ticket.name_space;
  };
  ~MockFilter() { LOG(INFO) << "MockFilter destroyed"; }

  // NOLINTNEXTLINE(readability-convert-member-functions-to-static)
  an<Translation> apply(an<Translation> translation, const JSValue& environment) {
    return translation;
  }
};

// NOLINTNEXTLINE(cppcoreguidelines-avoid-non-const-global-variables)
TEST(QuickJSComponentTest, ShareComponentAcrossRimeSessions) {
  QuickJSComponent<MockFilter, Filter> component;

  Ticket ticket(Engine::Create(), "test_namespace", "test");

  auto* instance1 = component.Create(ticket);
  auto actualInstance1 = instance1->actual();
  delete instance1;  // Rime session 1 ends

  auto* instance2 = component.Create(ticket);
  ASSERT_EQ(actualInstance1, instance2->actual())
      << "delete instance1 should not destroy the actual filter instance";
  delete instance2;  // Rime session 2 ends

  Ticket ticket2(Engine::Create(), "test_namespace", "test");
  auto* instance3 = component.Create(ticket2);
  ASSERT_EQ(actualInstance1, instance3->actual())
      << "delete instance1 should not destroy the actual filter instance";
  delete instance3;  // Rime session 3 ends
}

// NOLINTNEXTLINE(cppcoreguidelines-avoid-non-const-global-variables)
TEST(QuickJSComponentTest, CreateComponent) {
  QuickJSComponent<MockFilter, Filter> component;

  Ticket ticket(Engine::Create(), "test_namespace", "test");
  auto* instance1 = component.Create(ticket);
  ASSERT_NE(nullptr, instance1);

  auto* instance2 = component.Create(ticket);
  ASSERT_EQ(instance1->actual(), instance2->actual())
      << "should return the same actual filter with the same ticket";

  Ticket ticket2(Engine::Create(), "test_namespace", "test");
  auto* instance3 = component.Create(ticket2);
  ASSERT_EQ(instance1->actual(), instance3->actual())
      << "should return the same actual filter with the same ticket namespace";

  Ticket ticket3(Engine::Create(), "test_namespace2", "test");
  auto* instance4 = component.Create(ticket3);
  ASSERT_TRUE(instance4 != nullptr);
  ASSERT_NE(instance1->actual(), instance4->actual())
      << "should create a new instance with a different ticket namespace";

  delete instance4;
  delete instance3;
  delete instance2;
  delete instance1;
}
