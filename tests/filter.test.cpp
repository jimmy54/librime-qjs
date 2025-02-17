#include <gtest/gtest.h>
#include "qjs_types.h"
#include "qjs_filter.h"
#include "qjs_candidate.h"
#include "qjs_engine.h"
#include <rime/candidate.h>
#include <rime/translation.h>
#include <rime/engine.h>
#include <rime/schema.h>
#include <rime/context.h>
#include <rime/config/config_component.h>

#include "fake_translation.h"

using namespace rime;

class QuickJSFilterTest : public ::testing::Test {
protected:
    void SetUp() override {
        QjsHelper::basePath = "tests/js";
    }
};

TEST_F(QuickJSFilterTest, ApplyFilter) {
    auto ctx = QjsHelper::getInstance().getContext();
    QjsHelper::exposeLogToJsConsole(ctx);

    auto arg = JSValueRAII(JS_NewObject(ctx));
    auto engine = Engine::Create();
    // engine->ApplySchema(&schema); // ApplySchema 会触发回调函数，导致 segfault
    // engine->schema()->schema_id() = .default, engine->schema()->schema_name() = .default
    ASSERT_TRUE(engine->schema() != nullptr);

    auto config = engine->schema()->config();
    ASSERT_TRUE(config != nullptr);
    config->SetString("greet", "hello from c++");
    config->SetString("expectingText", "text2");

    Ticket ticket(engine, "filter", "qjs_filter@filter_test");

    auto filter = New<QuickJSFilter>(ticket, "tests/js");

    auto translation = New<FakeTranslation>();
    translation->Append(New<SimpleCandidate>("mock", 0, 1, "text1", "comment1"));
    translation->Append(New<SimpleCandidate>("mock", 0, 1, "text2", "comment2"));
    translation->Append(New<SimpleCandidate>("mock", 0, 1, "text3", "comment3"));

    CandidateList candidates;
    auto filtered = filter->Apply(translation, &candidates);
    ASSERT_TRUE(filtered != nullptr);

    auto candidate = filtered->Peek();
    ASSERT_TRUE(candidate!= nullptr);
    ASSERT_EQ(candidate->text(), "text2");

    filtered->Next();
    EXPECT_TRUE(filtered->exhausted());
    candidate = filtered->Peek();
    ASSERT_TRUE(candidate == nullptr);
    ASSERT_FALSE(filtered->Next());
}
