#include <gtest/gtest.h>
#include <rime/candidate.h>
#include <rime/config/config_component.h>
#include <rime/context.h>
#include <rime/engine.h>
#include <rime/schema.h>
#include <rime/translation.h>

#include "fake_translation.hpp"
#include "qjs_filter.hpp"
#include "test_switch.h"

using namespace rime;

template <typename T>
class QuickJSFilterTest : public ::testing::Test {
public:
  static an<Translation> createMockTranslation() {
    auto translation = New<FakeTranslation>();
    translation->append(New<SimpleCandidate>("mock", 0, 1, "text1", "comment1"));
    translation->append(New<SimpleCandidate>("mock", 0, 1, "text2", "comment2"));
    translation->append(New<SimpleCandidate>("mock", 0, 1, "text3", "comment3"));
    return translation;
  }
};

SETUP_JS_ENGINES(QuickJSFilterTest);

template <typename T>
// NOLINTNEXTLINE(readability-function-cognitive-complexity)
static void testFilter() {
  the<Engine> engine(Engine::Create());
  ASSERT_TRUE(engine->schema() != nullptr);

  auto* config = engine->schema()->config();
  ASSERT_TRUE(config != nullptr);
  config->SetString("greet", "hello from c++");
  config->SetString("expectingText", "text2");

  Ticket ticket(engine.get(), "filter", "qjs_filter@filter_test");
  auto env = std::make_unique<Environment>(engine.get(), "filter_test");
  auto filter = New<QuickJSFilter<T>>(ticket, env.get());
  auto translation = QuickJSFilterTest<T>::createMockTranslation();
  auto filtered = filter->apply(translation, env.get());
  ASSERT_TRUE(filtered != nullptr);

  auto candidate = filtered->Peek();
  ASSERT_TRUE(candidate != nullptr);
  ASSERT_EQ(candidate->text(), "text2");

  filtered->Next();
  EXPECT_TRUE(filtered->exhausted());
  candidate = filtered->Peek();
  ASSERT_TRUE(candidate == nullptr);
  ASSERT_FALSE(filtered->Next());
}

TYPED_TEST(QuickJSFilterTest, ApplyFilter) {
  testFilter<TypeParam>();
}

TYPED_TEST(QuickJSFilterTest, TestRestartEngine) {
  JsEngine<TypeParam>::shutdown();

  std::filesystem::path path(rime_get_api()->get_user_data_dir());
  path.append("js");

  auto& engine = JsEngine<TypeParam>::instance();
  registerTypesToJsEngine<TypeParam>();
  engine.setBaseFolderPath(path.generic_string().c_str());

  testFilter<TypeParam>();
}
