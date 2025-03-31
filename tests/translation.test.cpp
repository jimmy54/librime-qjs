#include <gtest/gtest.h>
#include <rime/candidate.h>
#include <rime/translation.h>

#include <quickjs.h>
#include "fake_translation.hpp"
#include "qjs_translation.h"

using namespace rime;

class QuickJSTranslationTest : public ::testing::Test {
protected:
  static an<Translation> createMockTranslation() {
    auto translation = New<FakeTranslation>();
    translation->append(New<SimpleCandidate>("mock", 0, 1, "text1", "comment1"));
    translation->append(New<SimpleCandidate>("mock", 0, 1, "text2", "comment2"));
    translation->append(New<SimpleCandidate>("mock", 0, 1, "text3", "comment3"));
    return translation;
  }
};

TEST_F(QuickJSTranslationTest, Initialize) {
  auto translation = createMockTranslation();
  auto qjsTranslation =
      New<QuickJSTranslation<JSValue>>(translation, JS_UNDEFINED, JS_UNDEFINED, JS_UNDEFINED);
  EXPECT_TRUE(qjsTranslation->exhausted());
  EXPECT_FALSE(qjsTranslation->Next());
  EXPECT_EQ(qjsTranslation->Peek(), nullptr);
}

// NOLINTNEXTLINE(cppcoreguidelines-avoid-non-const-global-variables, readability-function-cognitive-complexity)
TEST_F(QuickJSTranslationTest, FilterCandidates) {
  auto translation = createMockTranslation();
  const char* jsCode = R"(
        function filterCandidates(candidates, env) {
            console.log(`filterCandidates: ${candidates.length}`)
            console.log(`env.expectingText: ${env.expectingText}`)
            return candidates.filter((it, idx) => it.text === env.expectingText)
        }
    )";

  auto& jsEngine = JsEngine<JSValue>::getInstance();
  auto& ctx = jsEngine.getContext();
  JSValue result = JS_Eval(ctx, jsCode, strlen(jsCode), "<input>", JS_EVAL_TYPE_GLOBAL);
  jsEngine.freeValue(result);
  JSValue global = JS_GetGlobalObject(ctx);
  JSValue filterFunc = JS_GetPropertyStr(ctx, global, "filterCandidates");
  jsEngine.freeValue(global);

  JSValue env = JS_NewObject(ctx);
  JS_SetPropertyStr(ctx, env, "expectingText", JS_NewString(ctx, "text2"));

  auto qjsTranslation =
      New<QuickJSTranslation<JSValue>>(translation, JS_UNDEFINED, filterFunc, env);
  auto candidate = qjsTranslation->Peek();

  ASSERT_TRUE(candidate != nullptr);
  EXPECT_EQ(candidate->text(), "text2");
  ASSERT_TRUE(qjsTranslation->Next());
  EXPECT_TRUE(qjsTranslation->exhausted());
  candidate = qjsTranslation->Peek();
  ASSERT_TRUE(candidate == nullptr);
  ASSERT_FALSE(qjsTranslation->Next());

  jsEngine.freeValue(filterFunc);
}

TEST_F(QuickJSTranslationTest, EmptyTranslation) {
  auto translation = New<FakeTranslation>();
  auto qjsTranslation =
      New<QuickJSTranslation<JSValue>>(translation, JS_UNDEFINED, JS_UNDEFINED, JS_UNDEFINED);
  EXPECT_TRUE(qjsTranslation->exhausted());
  EXPECT_FALSE(qjsTranslation->Next());
  EXPECT_EQ(qjsTranslation->Peek(), nullptr);
}

TEST_F(QuickJSTranslationTest, NoReturnValueShouldNotCrash) {
  auto translation = createMockTranslation();
  const char* jsCode = "function noReturn() { }";

  auto& jsEngine = JsEngine<JSValue>::getInstance();
  auto& ctx = jsEngine.getContext();
  JSValue result = JS_Eval(ctx, jsCode, strlen(jsCode), "<input>", JS_EVAL_TYPE_GLOBAL);
  jsEngine.freeValue(result);
  JSValue global = JS_GetGlobalObject(ctx);
  JSValue filterFunc = JS_GetPropertyStr(ctx, global, "noReturn");
  jsEngine.freeValue(global);

  auto qjsTranslation =
      New<QuickJSTranslation<JSValue>>(translation, JS_UNDEFINED, filterFunc, JS_UNDEFINED);
  EXPECT_TRUE(qjsTranslation->exhausted());
  EXPECT_FALSE(qjsTranslation->Next());
  EXPECT_EQ(qjsTranslation->Peek(), nullptr);

  jsEngine.freeValue(filterFunc);
}
