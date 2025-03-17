#include <gtest/gtest.h>
#include <rime/config/config_component.h>
#include <rime/context.h>
#include <rime/engine.h>
#include <rime/key_event.h>
#include <rime/schema.h>

#include "qjs_environment.h"
#include "qjs_helper.h"
#include "qjs_processor.h"
#include "quickjs.h"
#include "test_helper.h"

using namespace rime;

class QuickJSProcessorTest : public ::testing::Test {
protected:
  void SetUp() override { setJsBasePathForTest(__FILE__, "/js"); }

  static void addSegment(Engine* engine, const std::string& prompt) {
    Segment segment(0, static_cast<int>(prompt.length()));
    segment.prompt = prompt;
    engine->context()->composition().AddSegment(segment);
  }
};

// NOLINTNEXTLINE(cppcoreguidelines-avoid-non-const-global-variables, readability-function-cognitive-complexity)
TEST_F(QuickJSProcessorTest, ProcessKeyEvent) {
  auto* engine = Engine::Create();
  ASSERT_TRUE(engine->schema() != nullptr);

  auto* config = engine->schema()->config();
  ASSERT_TRUE(config != nullptr);
  config->SetString("greet", "hello from c++");

  addSegment(engine, "prompt");

  Ticket ticket(engine, "processor_test", "qjs_processor@processor_test");
  auto* ctx = QjsHelper::getInstance().getContext();
  JSValue environment = QjsEnvironment::create(ctx, engine, "processor_test");
  auto processor = New<QuickJSProcessor>(ticket, environment);

  // Test key event that should be accepted
  KeyEvent acceptEvent("space");
  EXPECT_EQ(processor->processKeyEvent(acceptEvent, environment), kAccepted);

  // Test key event that should be rejected
  KeyEvent rejectEvent("Return");
  EXPECT_EQ(processor->processKeyEvent(rejectEvent, environment), kRejected);

  // Test key event that should result in noop
  KeyEvent noopEvent("invalid_key");
  EXPECT_EQ(processor->processKeyEvent(noopEvent, environment), kNoop);

  JS_FreeValue(ctx, environment);
}

// NOLINTNEXTLINE(cppcoreguidelines-avoid-non-const-global-variables, readability-function-cognitive-complexity)
TEST_F(QuickJSProcessorTest, NonExistentModule) {
  auto* engine = Engine::Create();
  ASSERT_TRUE(engine->schema() != nullptr);

  addSegment(engine, "prompt");

  // Create a ticket with a non-existent module
  Ticket ticket(engine, "non_existent", "qjs_processor@non_existent");
  auto* ctx = QjsHelper::getInstance().getContext();
  JSValue environment = QjsEnvironment::create(ctx, engine, "non_existent");
  auto processor = New<QuickJSProcessor>(ticket, environment);

  // Test key event - should return noop due to unloaded module
  KeyEvent event("space");
  EXPECT_EQ(processor->processKeyEvent(event, environment), kNoop);

  JS_FreeValue(ctx, environment);
}
