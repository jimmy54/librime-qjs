#include <gtest/gtest.h>
#include <rime/config/config_component.h>
#include <rime/context.h>
#include <rime/engine.h>
#include <rime/key_event.h>
#include <rime/schema.h>

#include <quickjs.h>
#include "qjs_environment.h"
#include "qjs_processor.h"

using namespace rime;

class QuickJSProcessorTest : public ::testing::Test {
protected:
  static void addSegment(Engine* engine, const std::string& prompt) {
    Segment segment(0, static_cast<int>(prompt.length()));
    segment.prompt = prompt;
    engine->context()->composition().AddSegment(segment);
  }
};

// NOLINTNEXTLINE(cppcoreguidelines-avoid-non-const-global-variables, readability-function-cognitive-complexity)
TEST_F(QuickJSProcessorTest, ProcessKeyEvent) {
  the<Engine> engine(Engine::Create());
  ASSERT_TRUE(engine->schema() != nullptr);

  auto* config = engine->schema()->config();
  ASSERT_TRUE(config != nullptr);
  config->SetString("greet", "hello from c++");

  addSegment(engine.get(), "prompt");

  Ticket ticket(engine.get(), "processor_test", "qjs_processor@processor_test");
  JSValue environment = QjsEnvironment<JSValue>::create(engine.get(), "processor_test");
  auto processor = New<QuickJSProcessor<JSValue>>(ticket, environment);

  // Test key event that should be accepted
  KeyEvent acceptEvent("space");
  EXPECT_EQ(processor->processKeyEvent(acceptEvent, environment), kAccepted);

  // Test key event that should be rejected
  KeyEvent rejectEvent("Return");
  EXPECT_EQ(processor->processKeyEvent(rejectEvent, environment), kRejected);

  // Test key event that should result in noop
  KeyEvent noopEvent("invalid_key");
  EXPECT_EQ(processor->processKeyEvent(noopEvent, environment), kNoop);

  auto& jsEngine = JsEngine<JSValue>::getInstance();
  jsEngine.freeValue(environment);
}

// NOLINTNEXTLINE(cppcoreguidelines-avoid-non-const-global-variables, readability-function-cognitive-complexity)
TEST_F(QuickJSProcessorTest, NonExistentModule) {
  the<Engine> engine(Engine::Create());
  ASSERT_TRUE(engine->schema() != nullptr);

  addSegment(engine.get(), "prompt");

  // Create a ticket with a non-existent module
  Ticket ticket(engine.get(), "non_existent", "qjs_processor@non_existent");
  JSValue environment = QjsEnvironment<JSValue>::create(engine.get(), "non_existent");
  auto processor = New<QuickJSProcessor<JSValue>>(ticket, environment);

  // Test key event - should return noop due to unloaded module
  KeyEvent event("space");
  EXPECT_EQ(processor->processKeyEvent(event, environment), kNoop);

  auto& jsEngine = JsEngine<JSValue>::getInstance();
  jsEngine.freeValue(environment);
}
