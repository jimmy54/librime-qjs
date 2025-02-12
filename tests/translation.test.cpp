#include <gtest/gtest.h>
#include "qjs_translation.h"
#include "qjs_candidate.h"
#include "qjs_helpers.h"
#include <rime/candidate.h>
#include <rime/translation.h>

#include "fake_translation.h"

using namespace rime;
class QuickJSTranslationTest : public ::testing::Test {
protected:
    void SetUp() override {
        rt_ = JS_NewRuntime();
        ctx_ = JS_NewContext(rt_);

        QjsCandidate().Register(ctx_);
        registerLogToJsConsole(ctx_);
    }

    void TearDown() override {
        JS_FreeContext(ctx_);
        JS_FreeRuntime(rt_);
    }

    an<Translation> CreateMockTranslation() {
        auto translation = New<FakeTranslation>();
        translation->Append(New<SimpleCandidate>("mock", 0, 1, "text1", "comment1"));
        translation->Append(New<SimpleCandidate>("mock", 0, 1, "text2", "comment2"));
        translation->Append(New<SimpleCandidate>("mock", 0, 1, "text3", "comment3"));
        return translation;
    }

    JSRuntime* rt_;
    JSContext* ctx_;
};

TEST_F(QuickJSTranslationTest, Initialize) {
    auto translation = CreateMockTranslation();
    auto qjs_translation = New<QuickJSTranslation>(translation, ctx_, "", "");
    ASSERT_TRUE(qjs_translation != nullptr);
}

TEST_F(QuickJSTranslationTest, FilterCandidates) {
    auto translation = CreateMockTranslation();
    const char* jsCode = R"(
        function filterCandidates(candidates) {
            console.log(`filterCandidates: ${candidates.length}`)
            return candidates.filter((it, idx) => it.text === "text2");
        }
    )";
    auto qjs_translation = New<QuickJSTranslation>(translation, ctx_, jsCode, "filterCandidates");
    auto candidate = qjs_translation->Peek();

    ASSERT_TRUE(candidate != nullptr);
    EXPECT_EQ(candidate->text(), "text2");
    ASSERT_TRUE(qjs_translation->Next());
    EXPECT_TRUE(qjs_translation->exhausted());
    candidate = qjs_translation->Peek();
    ASSERT_TRUE(candidate == nullptr);
    ASSERT_FALSE(qjs_translation->Next());
}

TEST_F(QuickJSTranslationTest, EmptyTranslation) {
    auto translation = New<FakeTranslation>();
    auto qjs_translation = New<QuickJSTranslation>(translation, ctx_, "", "");
    EXPECT_TRUE(qjs_translation->exhausted());
    EXPECT_FALSE(qjs_translation->Next());
    EXPECT_EQ(qjs_translation->Peek(), nullptr);
}
