#include <gtest/gtest.h>
#include <rime/config/config_component.h>
#include <rime/context.h>
#include <rime/engine.h>
#include <rime/key_event.h>
#include <rime/schema.h>

#include <memory>
#include "environment.h"
#include "qjs_processor.h"
#include "test_switch.h"

using namespace rime;

template <typename T>
class QuickJSProcessorTest : public ::testing::Test {
protected:
  static void addSegment(Engine* engine, const std::string& prompt) {
    Segment segment(0, static_cast<int>(prompt.length()));
    segment.prompt = prompt;
    engine->context()->composition().AddSegment(segment);
  }
};

SETUP_JS_ENGINES(QuickJSProcessorTest);

TYPED_TEST(QuickJSProcessorTest, ProcessKeyEvent) {
  auto& jsEngine = getJsEngine<TypeParam>();

  the<Engine> engine(Engine::Create());
  ASSERT_TRUE(engine->schema() != nullptr);

  auto* config = engine->schema()->config();
  ASSERT_TRUE(config != nullptr);
  config->SetString("greet", "hello from c++");

  this->addSegment(engine.get(), "prompt");

  Ticket ticket(engine.get(), "processor_test", "qjs_processor@processor_test");
  auto env = std::make_shared<Environment>(engine.get(), "processor_test");
  TypeParam environment = jsEngine.wrapShared(env);
  auto processor = New<QuickJSProcessor<TypeParam>>(ticket, environment);

  // Test key event that should be accepted
  KeyEvent acceptEvent("space");
  EXPECT_EQ(processor->processKeyEvent(acceptEvent, environment), kAccepted);

  // Test key event that should be rejected
  KeyEvent rejectEvent("Return");
  EXPECT_EQ(processor->processKeyEvent(rejectEvent, environment), kRejected);

  // Test key event that should result in noop
  KeyEvent noopEvent("invalid_key");
  EXPECT_EQ(processor->processKeyEvent(noopEvent, environment), kNoop);

  jsEngine.freeValue(environment);
}

TYPED_TEST(QuickJSProcessorTest, NonExistentModule) {
  auto& jsEngine = getJsEngine<TypeParam>();

  the<Engine> engine(Engine::Create());
  ASSERT_TRUE(engine->schema() != nullptr);

  this->addSegment(engine.get(), "prompt");

  // Create a ticket with a non-existent module
  Ticket ticket(engine.get(), "non_existent", "qjs_processor@non_existent");
  auto env = std::make_shared<Environment>(engine.get(), "non_existent");
  TypeParam environment = jsEngine.wrapShared(env);
  auto processor = New<QuickJSProcessor<TypeParam>>(ticket, environment);

  // Test key event - should return noop due to unloaded module
  KeyEvent event("space");
  EXPECT_EQ(processor->processKeyEvent(event, environment), kNoop);

  jsEngine.freeValue(environment);
}
