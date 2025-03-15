#include <gtest/gtest.h>
#include <quickjs.h>

#include <memory>
#include <string_view>
#include <vector>

#include "jsvalue_raii.h"
#include "qjs_helper.h"
#include "qjs_iterator.h"

using std::move;

using namespace rime;

class VectorIterator : public AbstractIterator {
public:
  explicit VectorIterator(JSContext* context, std::vector<int>& vec)
      : context_{context}, vec_{vec} {}

  bool next() override { return current_ < vec_.size(); }

  JSValue peek() override {
    int value = next() ? vec_[current_++] : -1;
    return JS_NewInt32(context_, value);
  }

private:
  JSContext* context_{nullptr};
  std::vector<int> vec_;
  size_t current_{0};
};

// NOLINTNEXTLINE(cppcoreguidelines-avoid-non-const-global-variables)
template <>
JSClassID JSIteratorWrapper<VectorIterator>::classId = 0;  // Initialize with 0

class QuickJSGeneratorTest : public ::testing::Test {
protected:
  void SetUp() override {
    auto* context = QjsHelper::getInstance().getContext();
    wrapper_ = std::make_unique<JSIteratorWrapper<VectorIterator>>(context);
  }

  void TearDown() override {
    // free the wrapper before freeing the context and runtime
    wrapper_.reset();
  }

  std::unique_ptr<JSIteratorWrapper<VectorIterator>>& getWrapper() { return wrapper_; }

private:
  std::unique_ptr<JSIteratorWrapper<VectorIterator>> wrapper_;
};

// NOLINTNEXTLINE(cppcoreguidelines-avoid-non-const-global-variables, readability-function-cognitive-complexity)
TEST_F(QuickJSGeneratorTest, TestVectorIterator) {
  // NOLINTNEXTLINE(readability-magic-numbers, cppcoreguidelines-avoid-magic-numbers)
  std::vector<int> numbers{1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
  auto* context = QjsHelper::getInstance().getContext();
  auto* vecIterator = new VectorIterator(context, numbers);
  JSValueRAII iterator(getWrapper()->createIterator(vecIterator));
  // vecIterator is now managed by the shared_ptr, don't delete it manually
  ASSERT_FALSE(JS_IsException(iterator));

  constexpr std::string_view SCRIPT = R"(
        function* filterEvenNumbers(iter) {
            while (iter.next()) {
                const num = iter.peek();
                if (num % 2 === 0) {
                    yield num;
                }
            }
        }
    )";

  JSValueRAII result =
      JS_Eval(context, SCRIPT.data(), SCRIPT.size(), "<input>", JS_EVAL_TYPE_GLOBAL);
  ASSERT_FALSE(JS_IsException(result));

  JSValueRAII global(JS_GetGlobalObject(context));
  JSValueRAII filterFunc(JS_GetPropertyStr(context, global, "filterEvenNumbers"));
  ASSERT_FALSE(JS_IsException(filterFunc));

  // Update the JS_Call line to use getPtr() instead of taking address of get()
  JSValueRAII generator(JS_Call(context, filterFunc, JS_UNDEFINED, 1, iterator.getPtr()));
  ASSERT_FALSE(JS_IsException(generator));

  JSValueRAII nextMethod(JS_GetPropertyStr(context, generator, "next"));

  std::vector<int> filteredNumbers;
  while (true) {
    JSValueRAII nextResult(JS_Call(context, nextMethod, generator, 0, nullptr));
    if (JS_IsException(nextResult)) {
      break;
    }

    JSValueRAII done(JS_GetPropertyStr(context, nextResult, "done"));
    if (JS_ToBool(context, done) != 0) {
      break;
    }

    JSValueRAII value(JS_GetPropertyStr(context, nextResult, "value"));
    int32_t num = 0;
    JS_ToInt32(context, &num, value);
    filteredNumbers.push_back(num);
  }

  ASSERT_EQ(filteredNumbers, (std::vector<int>{2, 4, 6, 8, 10}));
}
