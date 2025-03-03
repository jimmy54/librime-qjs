#include <gtest/gtest.h>
#include "qjs_types.h"
#include "qjs_translator.h"
#include "qjs_candidate.h"
#include "qjs_engine.h"
#include "qjs_segment.h"
#include <rime/candidate.h>
#include <rime/translation.h>
#include <rime/engine.h>
#include <rime/schema.h>
#include <rime/context.h>
#include <rime/config/config_component.h>
#include <rime/gear/translator_commons.h>


using namespace rime;

class QuickJSTranslatorTest : public ::testing::Test {
protected:
    void SetUp() override {
        QjsHelper::basePath = "tests/js";
    }
};

TEST_F(QuickJSTranslatorTest, QueryTranslation) {
    auto ctx = QjsHelper::getInstance().getContext();
    QjsHelper::exposeLogToJsConsole(ctx);

    auto engine = Engine::Create();
    ASSERT_TRUE(engine->schema() != nullptr);

    auto config = engine->schema()->config();
    ASSERT_TRUE(config != nullptr);
    config->SetString("greet", "hello from c++");
    config->SetString("expectedInput", "test_input");

    Ticket ticket(engine, "translator", "qjs_translator@translator_test");

    auto translator = New<QuickJSTranslator>(ticket);

    // Create a segment for testing
    Segment segment;
    segment.start = 0;
    segment.end = 10;
    segment.length = 10;

    // Test the translator with the expected input
    auto translation = translator->Query("test_input", segment);
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
}

TEST_F(QuickJSTranslatorTest, EmptyResult) {
    auto ctx = QjsHelper::getInstance().getContext();
    QjsHelper::exposeLogToJsConsole(ctx);

    auto engine = Engine::Create();
    ASSERT_TRUE(engine->schema() != nullptr);

    auto config = engine->schema()->config();
    ASSERT_TRUE(config != nullptr);
    config->SetString("greet", "hello from c++");
    config->SetString("expectedInput", "empty_input");

    Ticket ticket(engine, "translator", "qjs_translator@translator_test");

    auto translator = New<QuickJSTranslator>(ticket);

    // Create a segment for testing
    Segment segment;
    segment.start = 0;
    segment.end = 10;
    segment.length = 10;

    // Test the translator with input that should return empty results
    auto translation = translator->Query("empty_input", segment);
    ASSERT_TRUE(translation != nullptr);
    EXPECT_TRUE(translation->exhausted());

    auto candidate = translation->Peek();
    EXPECT_TRUE(candidate == nullptr);
}

TEST_F(QuickJSTranslatorTest, NonExistentModule) {
    auto ctx = QjsHelper::getInstance().getContext();
    QjsHelper::exposeLogToJsConsole(ctx);

    auto engine = Engine::Create();
    ASSERT_TRUE(engine->schema() != nullptr);

    // Create a ticket with a non-existent module
    Ticket ticket(engine, "translator", "qjs_translator@non_existent");

    auto translator = New<QuickJSTranslator>(ticket);

    // Create a segment for testing
    Segment segment;
    segment.start = 0;
    segment.end = 10;
    segment.length = 10;

    // Test the translator - should return an empty translation
    auto translation = translator->Query("test_input", segment);
    ASSERT_TRUE(translation != nullptr);
    EXPECT_TRUE(translation->exhausted());

    auto candidate = translation->Peek();
    EXPECT_TRUE(candidate == nullptr);
}
