#include <gtest/gtest.h>
#include "qjs_translation.h"
#include "qjs_candidate.h"
#include "qjs_helper.h"
#include <rime/candidate.h>
#include <rime/translation.h>

#include "fake_translation.h"

using namespace rime;
class QuickJSTranslationTest : public ::testing::Test {
protected:
    void SetUp() override {
        rt_ = QjsHelper::getInstance().getRuntime();
        ctx_ = QjsHelper::getInstance().getContext();

        QjsCandidate().Register(ctx_);
        QjsHelper::exposeLogToJsConsole(ctx_);
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
    auto qjs_translation = New<QuickJSTranslation>(translation, JSValueRAII(JS_UNDEFINED), JS_UNDEFINED);
    ASSERT_TRUE(qjs_translation != nullptr);
}

TEST_F(QuickJSTranslationTest, FilterCandidates) {
    auto translation = CreateMockTranslation();
    const char* jsCode = R"(
        function filterCandidates(candidates, env) {
            console.log(`filterCandidates: ${candidates.length}`)
            console.log(`env.expectingText: ${env.expectingText}`)
            return candidates.filter((it, idx) => it.text === env.expectingText)
        }
    )";

    JSValueRAII result(JS_Eval(ctx_, jsCode, strlen(jsCode), "<input>", JS_EVAL_TYPE_GLOBAL));
    JSValueRAII global(JS_GetGlobalObject(ctx_));
    JSValueRAII filterFunc(JS_GetPropertyStr(ctx_, global, "filterCandidates"));

    JSValueRAII env(JS_NewObject(ctx_));
    JSValueRAII jsExpectingText(JS_NewString(ctx_, "text2"));
    JS_SetPropertyStr(ctx_, env, "expectingText", jsExpectingText);

    auto qjs_translation = New<QuickJSTranslation>(translation, filterFunc, env);
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
    auto qjs_translation = New<QuickJSTranslation>(translation, JSValueRAII(JS_UNDEFINED), JS_UNDEFINED);
    EXPECT_TRUE(qjs_translation->exhausted());
    EXPECT_FALSE(qjs_translation->Next());
    EXPECT_EQ(qjs_translation->Peek(), nullptr);
}
