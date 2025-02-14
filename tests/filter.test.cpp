#include <gtest/gtest.h>
#include "qjs_filter.h"
#include "qjs_candidate.h"
#include "qjs_helper.h"
#include <rime/candidate.h>
#include <rime/translation.h>

#include "fake_translation.h"

using namespace rime;

class QuickJSFilterTest : public ::testing::Test {
protected:
    void SetUp() override {
        rt_ = JS_NewRuntime();
        ctx_ = JS_NewContext(rt_);

        QjsCandidate().Register(ctx_);
        QjsHelper::exposeLogToJsConsole(ctx_);
        QjsHelper::basePath = "tests/js";
    }

    void TearDown() override {
        JS_FreeContext(ctx_);
        JS_FreeRuntime(rt_);
    }

    JSRuntime* rt_;
    JSContext* ctx_;
};

TEST_F(QuickJSFilterTest, Initialize) {
    Ticket ticket(nullptr, "filter", "qjs_filter@filter_test");
    auto filter = New<QuickJSFilter>(ticket, ctx_, "tests/js");
    ASSERT_TRUE(filter != nullptr);
}

TEST_F(QuickJSFilterTest, ApplyFilter) {
    Ticket ticket(nullptr, "filter", "qjs_filter@filter_test");
    auto filter = New<QuickJSFilter>(ticket, ctx_, "tests/js");

    auto translation = New<FakeTranslation>();
    translation->Append(New<SimpleCandidate>("mock", 0, 1, "text1", "comment1"));
    translation->Append(New<SimpleCandidate>("mock", 0, 1, "text2", "comment2"));
    translation->Append(New<SimpleCandidate>("mock", 0, 1, "text3", "comment3"));

    CandidateList candidates;
    auto filtered = filter->Apply(translation, &candidates);
    ASSERT_TRUE(filtered != nullptr);

    auto candidate = filtered->Peek();
    ASSERT_TRUE(candidate!= nullptr);
    ASSERT_EQ(candidate->text(), "text1");

    filtered->Next();
    EXPECT_TRUE(filtered->exhausted());
    candidate = filtered->Peek();
    ASSERT_TRUE(candidate == nullptr);
    ASSERT_FALSE(filtered->Next());
}
