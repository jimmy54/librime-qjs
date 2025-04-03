#include <gtest/gtest.h>
#include <rime/candidate.h>
#include <rime/config/config_component.h>
#include <rime/context.h>
#include <rime/engine.h>
#include <rime/gear/translator_commons.h>
#include <rime/schema.h>
#include <rime/segmentation.h>
#include <rime/translation.h>

#include "qjs_translator.h"
#include "test_switch.h"

using namespace rime;

template <typename T>
class QuickJSTranslatorTest : public ::testing::Test {
protected:
  Segment createSegment() {
    Segment segment;
    segment.start = 0;
    constexpr size_t A_INT_NUMBER = 10;
    segment.end = A_INT_NUMBER;
    segment.length = A_INT_NUMBER;
    return segment;
  }
};

SETUP_JS_ENGINES(QuickJSTranslatorTest);

// NOLINTNEXTLINE(cppcoreguidelines-avoid-non-const-global-variables, readability-function-cognitive-complexity)
TYPED_TEST(QuickJSTranslatorTest, QueryTranslation) {
  auto& jsEngine = JsEngine<TypeParam>::getInstance();

  the<Engine> engine(Engine::Create());
  ASSERT_TRUE(engine->schema() != nullptr);

  auto* config = engine->schema()->config();
  ASSERT_TRUE(config != nullptr);
  config->SetString("greet", "hello from c++");
  config->SetString("expectedInput", "test_input");

  Ticket ticket(engine.get(), "translator", "qjs_translator@translator_test");
  auto env = std::make_shared<Environment>(engine.get(), "translator_test");
  TypeParam environment = jsEngine.wrapShared(env);
  auto translator = New<QuickJSTranslator<TypeParam>>(ticket, environment);

  // Create a segment for testing
  Segment segment = this->createSegment();

  // Test the translator with the expected input
  auto translation = translator->query("test_input", segment, environment);
  ASSERT_TRUE(translation != nullptr);

  // Verify the first candidate
  auto candidate = translation->Peek();
  ASSERT_TRUE(candidate != nullptr);
  EXPECT_EQ(candidate->text(), "candidate1");
  EXPECT_EQ(candidate->comment(), "comment1");

  // Move to the next candidate
  translation->Next();
  candidate = translation->Peek();
  ASSERT_TRUE(candidate != nullptr);
  EXPECT_EQ(candidate->text(), "candidate2");
  EXPECT_EQ(candidate->comment(), "comment2");

  // Move to the third candidate
  translation->Next();
  candidate = translation->Peek();
  ASSERT_TRUE(candidate != nullptr);
  EXPECT_EQ(candidate->text(), "candidate3");
  EXPECT_EQ(candidate->comment(), "comment3");

  // Should be exhausted after the third candidate
  translation->Next();
  EXPECT_TRUE(translation->exhausted());
  candidate = translation->Peek();
  EXPECT_TRUE(candidate == nullptr);

  jsEngine.freeValue(environment);
}

// NOLINTNEXTLINE(cppcoreguidelines-avoid-non-const-global-variables, readability-function-cognitive-complexity)
TYPED_TEST(QuickJSTranslatorTest, EmptyResult) {
  auto& jsEngine = JsEngine<TypeParam>::getInstance();

  the<Engine> engine(Engine::Create());
  ASSERT_TRUE(engine->schema() != nullptr);

  auto* config = engine->schema()->config();
  ASSERT_TRUE(config != nullptr);
  config->SetString("greet", "hello from c++");
  config->SetString("expectedInput", "empty_input");

  Ticket ticket(engine.get(), "translator", "qjs_translator@translator_test");
  auto env = std::make_shared<Environment>(engine.get(), "translator_test");
  TypeParam environment = jsEngine.wrapShared(env);
  auto translator = New<QuickJSTranslator<TypeParam>>(ticket, environment);

  // Create a segment for testing
  Segment segment = this->createSegment();

  // Test the translator with input that should return empty results
  auto translation = translator->query("empty_input", segment, environment);
  ASSERT_TRUE(translation != nullptr);
  EXPECT_TRUE(translation->exhausted());

  auto candidate = translation->Peek();
  EXPECT_TRUE(candidate == nullptr);

  jsEngine.freeValue(environment);
}

// NOLINTNEXTLINE(cppcoreguidelines-avoid-non-const-global-variables, readability-function-cognitive-complexity)
TYPED_TEST(QuickJSTranslatorTest, NonExistentModule) {
  auto& jsEngine = JsEngine<TypeParam>::getInstance();
  the<Engine> engine(Engine::Create());
  ASSERT_TRUE(engine->schema() != nullptr);

  // Create a ticket with a non-existent module
  Ticket ticket(engine.get(), "translator", "qjs_translator@non_existent");
  auto env = std::make_shared<Environment>(engine.get(), "non_existent");
  TypeParam environment = jsEngine.wrapShared(env);
  auto translator = New<QuickJSTranslator<TypeParam>>(ticket, environment);

  // Create a segment for testing
  Segment segment = this->createSegment();

  // Test the translator - should return an empty translation
  auto translation = translator->query("test_input", segment, environment);
  ASSERT_TRUE(translation != nullptr);
  EXPECT_TRUE(translation->exhausted());

  auto candidate = translation->Peek();
  EXPECT_TRUE(candidate == nullptr);

  jsEngine.freeValue(environment);
  // FIXME: it crashes randomly, maybe memory used after free?
}

TYPED_TEST(QuickJSTranslatorTest, NoReturnShouldNotCrash) {
  auto& jsEngine = JsEngine<TypeParam>::getInstance();

  the<Engine> engine(Engine::Create());

  // Create a ticket with a poor implemented plugin
  Ticket ticket(engine.get(), "translator", "qjs_translator@translator_no_return");
  auto env = std::make_shared<Environment>(engine.get(), "translator_no_return");
  TypeParam environment = jsEngine.wrapShared(env);
  auto translator = New<QuickJSTranslator<TypeParam>>(ticket, environment);
  Segment segment = this->createSegment();
  auto translation = translator->query("test_input", segment, environment);
  ASSERT_TRUE(translation != nullptr);
  EXPECT_TRUE(translation->exhausted());
  EXPECT_FALSE(translation->Next());
  EXPECT_EQ(translation->Peek(), nullptr);

  jsEngine.freeValue(environment);
}
