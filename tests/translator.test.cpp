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
#include "quickjs.h"

using namespace rime;

class QuickJSTranslatorTest : public ::testing::Test {
protected:
  static Segment createSegment() {
    Segment segment;
    segment.start = 0;
    constexpr size_t A_INT_NUMBER = 10;
    segment.end = A_INT_NUMBER;
    segment.length = A_INT_NUMBER;
    return segment;
  }
};

// NOLINTNEXTLINE(cppcoreguidelines-avoid-non-const-global-variables, readability-function-cognitive-complexity)
TEST_F(QuickJSTranslatorTest, QueryTranslation) {
  auto* ctx = QjsHelper::getInstance().getContext();

  the<Engine> engine(Engine::Create());
  ASSERT_TRUE(engine->schema() != nullptr);

  auto* config = engine->schema()->config();
  ASSERT_TRUE(config != nullptr);
  config->SetString("greet", "hello from c++");
  config->SetString("expectedInput", "test_input");

  Ticket ticket(engine.get(), "translator", "qjs_translator@translator_test");
  JSValue environment = QjsEnvironment::create(ctx, engine.get(), "translator_test");
  auto translator = New<QuickJSTranslator>(ticket, environment);

  // Create a segment for testing
  Segment segment = createSegment();

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

  JS_FreeValue(ctx, environment);
}

// NOLINTNEXTLINE(cppcoreguidelines-avoid-non-const-global-variables, readability-function-cognitive-complexity)
TEST_F(QuickJSTranslatorTest, EmptyResult) {
  auto* ctx = QjsHelper::getInstance().getContext();

  the<Engine> engine(Engine::Create());
  ASSERT_TRUE(engine->schema() != nullptr);

  auto* config = engine->schema()->config();
  ASSERT_TRUE(config != nullptr);
  config->SetString("greet", "hello from c++");
  config->SetString("expectedInput", "empty_input");

  Ticket ticket(engine.get(), "translator", "qjs_translator@translator_test");
  JSValue environment = QjsEnvironment::create(ctx, engine.get(), "translator_test");
  auto translator = New<QuickJSTranslator>(ticket, environment);

  // Create a segment for testing
  Segment segment = createSegment();

  // Test the translator with input that should return empty results
  auto translation = translator->query("empty_input", segment, environment);
  ASSERT_TRUE(translation != nullptr);
  EXPECT_TRUE(translation->exhausted());

  auto candidate = translation->Peek();
  EXPECT_TRUE(candidate == nullptr);

  JS_FreeValue(ctx, environment);
}

// NOLINTNEXTLINE(cppcoreguidelines-avoid-non-const-global-variables, readability-function-cognitive-complexity)
TEST_F(QuickJSTranslatorTest, NonExistentModule) {
  auto* ctx = QjsHelper::getInstance().getContext();

  the<Engine> engine(Engine::Create());
  ASSERT_TRUE(engine->schema() != nullptr);

  // Create a ticket with a non-existent module
  Ticket ticket(engine.get(), "translator", "qjs_translator@non_existent");
  JSValue environment = QjsEnvironment::create(ctx, engine.get(), "non_existent");
  auto translator = New<QuickJSTranslator>(ticket, environment);

  // Create a segment for testing
  Segment segment = createSegment();

  // Test the translator - should return an empty translation
  auto translation = translator->query("test_input", segment, environment);
  ASSERT_TRUE(translation != nullptr);
  EXPECT_TRUE(translation->exhausted());

  auto candidate = translation->Peek();
  EXPECT_TRUE(candidate == nullptr);

  JS_FreeValue(ctx, environment);
}

TEST_F(QuickJSTranslatorTest, NoReturnShouldNotCrash) {
  auto* ctx = QjsHelper::getInstance().getContext();
  the<Engine> engine(Engine::Create());

  // Create a ticket with a poor implemented plugin
  Ticket ticket(engine.get(), "translator", "qjs_translator@translator_no_return");
  JSValue environment = QjsEnvironment::create(ctx, engine.get(), "translator_no_return");
  auto translator = New<QuickJSTranslator>(ticket, environment);
  Segment segment = createSegment();
  auto translation = translator->query("test_input", segment, environment);
  ASSERT_TRUE(translation != nullptr);
  EXPECT_TRUE(translation->exhausted());
  EXPECT_FALSE(translation->Next());
  EXPECT_EQ(translation->Peek(), nullptr);

  JS_FreeValue(ctx, environment);
}
