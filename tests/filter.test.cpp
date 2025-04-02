#include <gtest/gtest.h>
#include <rime/candidate.h>
#include <rime/config/config_component.h>
#include <rime/context.h>
#include <rime/engine.h>
#include <rime/schema.h>
#include <rime/translation.h>

#include <quickjs.h>
#include "fake_translation.hpp"
#include "qjs_filter.hpp"

using namespace rime;

class QuickJSFilterTest : public ::testing::Test {};

// NOLINTNEXTLINE(cppcoreguidelines-avoid-non-const-global-variables, readability-function-cognitive-complexity)
TEST_F(QuickJSFilterTest, ApplyFilter) {
  auto& jsEngine = JsEngine<JSValue>::getInstance();

  the<Engine> engine(Engine::Create());
  ASSERT_TRUE(engine->schema() != nullptr);

  auto* config = engine->schema()->config();
  ASSERT_TRUE(config != nullptr);
  config->SetString("greet", "hello from c++");
  config->SetString("expectingText", "text2");

  Ticket ticket(engine.get(), "filter", "qjs_filter@filter_test");
  auto env = std::make_shared<Environment>(engine.get(), "filter_test");
  JSValue environment = jsEngine.wrapShared(env);
  auto filter = New<QuickJSFilter<JSValue>>(ticket, environment);

  auto translation = New<FakeTranslation>();
  translation->append(New<SimpleCandidate>("mock", 0, 1, "text1", "comment1"));
  translation->append(New<SimpleCandidate>("mock", 0, 1, "text2", "comment2"));
  translation->append(New<SimpleCandidate>("mock", 0, 1, "text3", "comment3"));

  auto filtered = filter->apply(translation, environment);
  ASSERT_TRUE(filtered != nullptr);

  auto candidate = filtered->Peek();
  ASSERT_TRUE(candidate != nullptr);
  ASSERT_EQ(candidate->text(), "text2");

  filtered->Next();
  EXPECT_TRUE(filtered->exhausted());
  candidate = filtered->Peek();
  ASSERT_TRUE(candidate == nullptr);
  ASSERT_FALSE(filtered->Next());

  jsEngine.freeValue(environment);
}
