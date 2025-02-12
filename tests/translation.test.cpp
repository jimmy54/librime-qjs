#include <gtest/gtest.h>
#include "qjs_translation.h"
#include "qjs_candidate.h"
#include <rime/candidate.h>
#include <rime/translation.h>

using namespace rime;

class FakeTranslation : public Translation {
public:
    bool Next() override {
        if (exhausted()) {
            return false;
        }
        if (++iter_ >= candidates_.size()) {
            set_exhausted(true);
        }
        return true;
    }

    an<Candidate> Peek() override {
        if (exhausted() || iter_ >= candidates_.size()) {
            set_exhausted(true);
            return nullptr;
        }
        return candidates_[iter_];
    }

    void Append(an<Candidate> candidate) {
        candidates_.push_back(candidate);
    }

private:
    std::vector<an<Candidate>> candidates_;
    size_t iter_ = 0;
};

class QuickJSTranslationTest : public ::testing::Test {
protected:
    void SetUp() override {
        rt_ = JS_NewRuntime();
        ctx_ = JS_NewContext(rt_);
        QjsCandidate().Register(ctx_);
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
    auto qjs_translation = New<QuickJSTranslation>(translation, ctx_);
    ASSERT_TRUE(qjs_translation != nullptr);
}

TEST_F(QuickJSTranslationTest, FilterCandidates) {
    auto translation = CreateMockTranslation();
    auto qjs_translation = New<QuickJSTranslation>(translation, ctx_);
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
    auto qjs_translation = New<QuickJSTranslation>(translation, ctx_);
    EXPECT_TRUE(qjs_translation->exhausted());
    EXPECT_FALSE(qjs_translation->Next());
    EXPECT_EQ(qjs_translation->Peek(), nullptr);
}
