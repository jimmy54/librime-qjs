#include <gtest/gtest.h>
#include <rime/candidate.h>
#include <rime/translation.h>

#include <JavaScriptCore/JavaScript.h>
#include <quickjs.h>
#include "fake_translation.hpp"
#include "qjs_translation.h"

using namespace rime;

using JsTypes = ::testing::Types<JSValue, JSValueRef>;

template <typename T>
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

TYPED_TEST_SUITE(QuickJSTranslationTest, JsTypes);

TYPED_TEST(QuickJSTranslationTest, Initialize) {
  auto translation = this->createMockTranslation();
  auto qjsTranslation =
      New<QuickJSTranslation<TypeParam>>(translation, TypeParam(), TypeParam(), TypeParam());
  EXPECT_TRUE(qjsTranslation->exhausted());
  EXPECT_FALSE(qjsTranslation->Next());
  EXPECT_EQ(qjsTranslation->Peek(), nullptr);
}

// NOLINTNEXTLINE(cppcoreguidelines-avoid-non-const-global-variables, readability-function-cognitive-complexity)
TYPED_TEST(QuickJSTranslationTest, FilterCandidates) {
  auto translation = this->createMockTranslation();
  const char* jsCode = R"(
        function filterCandidates(candidates, env) {
            console.log(`filterCandidates: ${candidates.length}`)
            console.log(`env.expectingText: ${env.expectingText}`)
            const ret = candidates.filter((it, idx) => {
              console.log(`it.text = ${it.text}`)
              return it.text === env.expectingText
            })
            console.log(`ret.length: ${ret.length}`)
            return ret
        }
    )";

  auto& jsEngine = JsEngine<TypeParam>::getInstance();
  auto result = jsEngine.eval(jsCode, "<input>");
  auto global = jsEngine.getGlobalObject();
  auto filterFunc = jsEngine.getObjectProperty(jsEngine.toObject(global), "filterCandidates");

  auto env = jsEngine.newObject();
  jsEngine.setObjectProperty(env, "expectingText", jsEngine.toJsString("text2"));

  auto qjsTranslation =
      New<QuickJSTranslation<TypeParam>>(translation, TypeParam(), filterFunc, env);
  auto candidate = qjsTranslation->Peek();

  ASSERT_TRUE(candidate != nullptr);
  EXPECT_EQ(candidate->text(), "text2");
  ASSERT_TRUE(qjsTranslation->Next());
  EXPECT_TRUE(qjsTranslation->exhausted());
  candidate = qjsTranslation->Peek();
  ASSERT_TRUE(candidate == nullptr);
  ASSERT_FALSE(qjsTranslation->Next());

  jsEngine.freeValue(global);
  jsEngine.freeValue(result);
  jsEngine.freeValue(filterFunc);
}

TYPED_TEST(QuickJSTranslationTest, EmptyTranslation) {
  auto translation = New<FakeTranslation>();
  auto qjsTranslation =
      New<QuickJSTranslation<TypeParam>>(translation, TypeParam(), TypeParam(), TypeParam());
  EXPECT_TRUE(qjsTranslation->exhausted());
  EXPECT_FALSE(qjsTranslation->Next());
  EXPECT_EQ(qjsTranslation->Peek(), nullptr);
}

TYPED_TEST(QuickJSTranslationTest, NoReturnValueShouldNotCrash) {
  auto translation = this->createMockTranslation();
  const char* jsCode = "function noReturn() { }";

  auto& jsEngine = JsEngine<TypeParam>::getInstance();
  auto& ctx = jsEngine.getContext();
  auto result = jsEngine.eval(jsCode, "<input>");
  jsEngine.freeValue(result);
  auto global = jsEngine.getGlobalObject();
  auto filterFunc = jsEngine.getObjectProperty(jsEngine.toObject(global), "noReturn");
  jsEngine.freeValue(global);

  auto qjsTranslation =
      New<QuickJSTranslation<TypeParam>>(translation, TypeParam(), filterFunc, TypeParam());
  EXPECT_TRUE(qjsTranslation->exhausted());
  EXPECT_FALSE(qjsTranslation->Next());
  EXPECT_EQ(qjsTranslation->Peek(), nullptr);

  jsEngine.freeValue(filterFunc);
}
