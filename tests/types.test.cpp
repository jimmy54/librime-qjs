#include <gtest/gtest.h>
#include <rime/candidate.h>
#include <rime/config/config_component.h>
#include <rime/context.h>
#include <rime/engine.h>
#include <rime/schema.h>

#include "jsvalue_raii.h"
#include "qjs_candidate.h"
#include "qjs_engine.h"
#include "qjs_environment.h"
#include "trie_data_helper.h"

using namespace rime;

class QuickJSTypesTest : public ::testing::Test {
private:
  TrieDataHelper trieDataHelper_ = TrieDataHelper("./tests", "dummy_dict.txt");

protected:
  void SetUp() override {
    QjsHelper::basePath = "tests/js";
    trieDataHelper_.createDummyTextFile();
  }

  void TearDown() override {
    trieDataHelper_.cleanupDummyFiles();
    std::remove("tests/dumm.bin");  // the file generated in js
  }
};

// NOLINTNEXTLINE(cppcoreguidelines-avoid-non-const-global-variables, readability-function-cognitive-complexity)
TEST_F(QuickJSTypesTest, WrapUnwrapRimeGears) {
  auto* ctx = QjsHelper::getInstance().getContext();

  auto* engine = Engine::Create();
  // engine->ApplySchema(&schema); // ApplySchema 会触发回调函数，导致 segfault
  // engine->schema()->schema_id() = .default, engine->schema()->schema_name() = .default
  ASSERT_TRUE(engine->schema() != nullptr);
  auto* config = engine->schema()->config();
  ASSERT_TRUE(config != nullptr);
  config->SetBool("key1", true);
  config->SetBool("key2", false);
  constexpr int A_INT_NUMBER = 666;
  config->SetInt("key3", A_INT_NUMBER);
  constexpr double A_DOUBLE_NUMBER = 0.999;
  config->SetDouble("key4", A_DOUBLE_NUMBER);
  config->SetString("key5", "string");

  auto list = New<ConfigList>();
  list->Append(New<ConfigValue>("item1"));
  list->Append(New<ConfigValue>("item2"));
  list->Append(New<ConfigValue>("item3"));
  config->SetItem("list", list);

  auto* context = engine->context();
  ASSERT_TRUE(context != nullptr);
  context->set_input("hello");

  JSValueRAII env = QjsEnvironment::create(ctx, engine, "namespace");
  auto candidate = New<SimpleCandidate>("mock", 0, 1, "text", "comment");
  JS_SetPropertyStr(ctx, env, "candidate", QjsCandidate::Wrap(ctx, candidate));

  JSValueRAII module = QjsHelper::loadJsModuleToGlobalThis(ctx, "types_test.js");

  JSValueRAII global = JS_GetGlobalObject(ctx);
  JSValueRAII jsFunc = JS_GetPropertyStr(ctx, global, "checkArgument");
  JSValueRAII retValue = JS_Call(ctx, jsFunc, JS_UNDEFINED, 1, env.getPtr());

  JSValueRAII retJsEngine = JS_GetPropertyStr(ctx, retValue, "engine");
  Engine* retEngine = QjsEngine::Unwrap(ctx, retJsEngine);
  ASSERT_EQ(retEngine, engine);
  ASSERT_EQ(retEngine->schema()->schema_name(), engine->schema()->schema_name());
  JSValueRAII retJsCandidate = JS_GetPropertyStr(ctx, retValue, "candidate");
  an<Candidate> retCandidate = QjsCandidate::Unwrap(ctx, retJsCandidate);
  ASSERT_EQ(retCandidate->text(), "new text");
  ASSERT_EQ(retCandidate.get(), candidate.get());

  string greet;
  bool success = retEngine->schema()->config()->GetString("greet", &greet);
  ASSERT_TRUE(success);
  ASSERT_EQ(greet, "hello from js");

  Context* retContext = retEngine->context();
  ASSERT_EQ(retContext->input(), "world");

  // js code: env.newCandidate = new Candidate('js', 32, 100, 'the text', 'the comment', 888)
  JSValueRAII retJsNewCandidate = JS_GetPropertyStr(ctx, retValue, "newCandidate");
  auto newCandidate = QjsCandidate::Unwrap(ctx, retJsNewCandidate);
  ASSERT_EQ(newCandidate->type(), "js");
  ASSERT_EQ(newCandidate->start(), 32);
  ASSERT_EQ(newCandidate->end(), 100);
  ASSERT_EQ(newCandidate->text(), "the text");
  ASSERT_EQ(newCandidate->comment(), "the comment");
  ASSERT_EQ(newCandidate->quality(), 888);
}
