#include <gtest/gtest.h>
#include <rime/config/config_component.h>
#include <rime/context.h>
#include <rime/engine.h>
#include <rime/key_event.h>
#include <rime/schema.h>

#include "qjs_processor.h"

using namespace rime;

class QuickJSProcessorTest : public ::testing::Test {
protected:
  void SetUp() override { QjsHelper::basePath = "tests/js"; }

  static void addSegment(Engine* engine, const std::string& prompt) {
    Segment segment(0, static_cast<int>(prompt.length()));
    segment.prompt = prompt;
    engine->context()->composition().AddSegment(segment);
  }
};

// NOLINTNEXTLINE(cppcoreguidelines-avoid-non-const-global-variables,
// readability-function-cognitive-complexity)
TEST_F(QuickJSProcessorTest, ProcessKeyEvent) {
  auto* engine = Engine::Create();
  ASSERT_TRUE(engine->schema() != nullptr);

  auto* config = engine->schema()->config();
  ASSERT_TRUE(config != nullptr);
  config->SetString("greet", "hello from c++");

  addSegment(engine, "prompt");

  Ticket ticket(engine, "processor", "qjs_processor@processor_test");
  auto processor = New<QuickJSProcessor>(ticket);

  // Test key event that should be accepted
  KeyEvent acceptEvent("space");
  EXPECT_EQ(processor->ProcessKeyEvent(acceptEvent), kAccepted);

  // Test key event that should be rejected
  KeyEvent rejectEvent("Return");
  EXPECT_EQ(processor->ProcessKeyEvent(rejectEvent), kRejected);

  // Test key event that should result in noop
  KeyEvent noopEvent("invalid_key");
  EXPECT_EQ(processor->ProcessKeyEvent(noopEvent), kNoop);
}

// NOLINTNEXTLINE(cppcoreguidelines-avoid-non-const-global-variables,
// readability-function-cognitive-complexity)
TEST_F(QuickJSProcessorTest, NonExistentModule) {
  auto* engine = Engine::Create();
  ASSERT_TRUE(engine->schema() != nullptr);

  addSegment(engine, "prompt");

  // Create a ticket with a non-existent module
  Ticket ticket(engine, "processor", "qjs_processor@non_existent");
  auto processor = New<QuickJSProcessor>(ticket);

  // Test key event - should return noop due to unloaded module
  KeyEvent event("space");
  EXPECT_EQ(processor->ProcessKeyEvent(event), kNoop);
}

// NOLINTNEXTLINE(cppcoreguidelines-avoid-non-const-global-variables,
// readability-function-cognitive-complexity)
TEST_F(QuickJSProcessorTest, ModuleInitialization) {
  auto* engine = Engine::Create();
  ASSERT_TRUE(engine->schema() != nullptr);

  auto* config = engine->schema()->config();
  ASSERT_TRUE(config != nullptr);
  config->SetString("init_test", "test_value");

  addSegment(engine, "prompt");

  Ticket ticket(engine, "processor", "qjs_processor@processor_test");
  auto processor = New<QuickJSProcessor>(ticket);

  // Test a key event to verify the processor was properly initialized
  KeyEvent event("space");
  EXPECT_EQ(processor->ProcessKeyEvent(event), kAccepted);
}
