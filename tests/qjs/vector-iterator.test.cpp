#include <quickjs.h>
#include <string>
#include <string_view>
#include <vector>
#include <memory>
#include <gtest/gtest.h>

#include "jsvalue_raii.h"
#include "qjs_helper.h"
#include "qjs_iterator.h"

using namespace rime;

class VectorIterator: public AbstractIterator {
public:
    explicit VectorIterator(JSContext* context, std::vector<int>& vec) : context_{context}, vec_{vec} {}

    bool next() {
        return current_ < vec_.size();
    }

    JSValue peek() {
        int value = next() ? vec_[current_++] : -1;
        return JS_NewInt32(context_, value);
    }

private:
    JSContext* context_{nullptr};
    std::vector<int>& vec_;
    size_t current_{0};
};

template<>
JSClassID JSIteratorWrapper<VectorIterator>::class_id_ = 0;  // Initialize with 0

class QuickJSGeneratorTest : public ::testing::Test {
protected:
    void SetUp() override {
        context_ = QjsHelper::getInstance().getContext();
        wrapper_ = std::make_unique<JSIteratorWrapper<VectorIterator>>(context_);
    }

    void TearDown() override {
        // free the wrapper before freeing the context and runtime
        wrapper_.reset();
    }

    JSContext* context_{nullptr};
    std::unique_ptr<JSIteratorWrapper<VectorIterator>> wrapper_;
};

// Update the test to remove context_ parameter from JSValueRAII construction
TEST_F(QuickJSGeneratorTest, TestVectorIterator) {
    std::vector<int> numbers{1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
    auto* vecIterator = new VectorIterator(context_, numbers);
    JSValueRAII iterator(wrapper_->createIterator(vecIterator));
    // vecIterator is now managed by the shared_ptr, don't delete it manually
    ASSERT_FALSE(JS_IsException(iterator));

    constexpr std::string_view script = R"(
        function* filterEvenNumbers(iter) {
            while (iter.next()) {
                const num = iter.peek();
                if (num % 2 === 0) {
                    yield num;
                }
            }
        }
    )";

    JSValueRAII result(JS_Eval(context_, script.data(), script.size(), "<input>", JS_EVAL_TYPE_GLOBAL));
    ASSERT_FALSE(JS_IsException(result));

    JSValueRAII global(JS_GetGlobalObject(context_));
    JSValueRAII filter_func(JS_GetPropertyStr(context_, global, "filterEvenNumbers"));
    ASSERT_FALSE(JS_IsException(filter_func));

    // Update the JS_Call line to use getPtr() instead of taking address of get()
    JSValueRAII generator(JS_Call(context_, filter_func, JS_UNDEFINED, 1, iterator.getPtr()));
    ASSERT_FALSE(JS_IsException(generator));

    JSValueRAII next_method(JS_GetPropertyStr(context_, generator, "next"));

    std::vector<int> filtered_numbers;
    while (true) {
        JSValueRAII next_result(JS_Call(context_, next_method, generator, 0, nullptr));
        if (JS_IsException(next_result)) {
            break;
        }

        JSValueRAII done(JS_GetPropertyStr(context_, next_result, "done"));
        if (JS_ToBool(context_, done)) {
            break;
        }

        JSValueRAII value(JS_GetPropertyStr(context_, next_result, "value"));
        int32_t num;
        JS_ToInt32(context_, &num, value);
        filtered_numbers.push_back(num);
    }

    ASSERT_EQ(filtered_numbers, (std::vector<int>{2, 4, 6, 8, 10}));
}
