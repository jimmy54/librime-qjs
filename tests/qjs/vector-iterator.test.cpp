#include <quickjs.h>
#include <string>
#include <string_view>
#include <vector>
#include <memory>
#include <gtest/gtest.h>

class VectorIterator {
public:
    explicit VectorIterator(const std::vector<int>& vec) : vec_{vec} {}

    [[nodiscard]] bool hasNext() const {
        return current_ < vec_.size();
    }

    [[nodiscard]] int next() {
        return hasNext() ? vec_[current_++] : -1;
    }

private:
    const std::vector<int>& vec_;
    size_t current_{0};
};

class JSIteratorWrapper {
public:
    explicit JSIteratorWrapper(JSContext* ctx) : ctx_{ctx} {
        auto rt = JS_GetRuntime(ctx);
        JS_NewClassID(rt, &class_id_);
        JS_NewClass(rt, class_id_, &class_def_);
        registerClass();
    }

    ~JSIteratorWrapper() {
        if (!JS_IsUndefined(proto_)) {
            JS_FreeValue(ctx_, proto_);
        }
    }

    [[nodiscard]] JSValue createIterator(const std::vector<int>& vec) const {
        auto obj = JS_NewObjectClass(ctx_, class_id_);
        if (JS_IsException(obj)) {
            return obj;
        }

        auto iter = std::make_unique<VectorIterator>(vec);
        if (JS_SetOpaque(obj, iter.release()) == -1) {
            return JS_EXCEPTION;
        }
        return obj;
    }

private:
    static JSClassID class_id_;
    JSContext* ctx_;
    JSValue proto_{JS_UNDEFINED};

    static void finalizer(JSRuntime* rt, JSValue val) {
        if (auto iter = static_cast<VectorIterator*>(JS_GetOpaque(val, class_id_))) {
            delete iter;
        }
    }

    static JSValue hasNext(JSContext* ctx, JSValueConst this_val, int, JSValueConst*) {
        auto iter = static_cast<VectorIterator*>(JS_GetOpaque(this_val, class_id_));
        return iter ? JS_NewBool(ctx, iter->hasNext()) : JS_ThrowTypeError(ctx, "Invalid iterator");
    }

    static JSValue next(JSContext* ctx, JSValueConst this_val, int, JSValueConst*) {
        auto iter = static_cast<VectorIterator*>(JS_GetOpaque(this_val, class_id_));
        return iter ? JS_NewInt32(ctx, iter->next()) : JS_ThrowTypeError(ctx, "Invalid iterator");
    }

    static constexpr JSClassDef class_def_{
        "VectorIterator",
        .finalizer = finalizer
    };

    static constexpr JSCFunctionListEntry proto_funcs_[] = {
        JS_CFUNC_DEF("hasNext", 0, hasNext),
        JS_CFUNC_DEF("next", 0, next)
    };

    void registerClass() {
        proto_ = JS_NewObject(ctx_);
        JS_SetPropertyFunctionList(ctx_, proto_, proto_funcs_,
                                 sizeof(proto_funcs_) / sizeof(proto_funcs_[0]));
        JS_SetClassProto(ctx_, class_id_, proto_);
    }
};

JSClassID JSIteratorWrapper::class_id_;


class JSValueRAII {
public:
    explicit JSValueRAII(JSValue val) : val_(val) {}
    ~JSValueRAII() { JS_FreeValue(context_, val_); }
    operator JSValue() const { return val_; }
    JSValue get() const { return val_; }
    JSValue* getPtr() { return &val_; }

    static JSContext* context_;
private:
    JSValue val_;
};

// Add after JSValueRAII class definition and before QuickJSTest class
JSContext* JSValueRAII::context_ = nullptr;

class QuickJSTest : public ::testing::Test {
protected:
    void SetUp() override {
        runtime_ = JS_NewRuntime();
        ASSERT_TRUE(runtime_);
        context_ = JS_NewContext(runtime_);
        ASSERT_TRUE(context_);

        JSValueRAII::context_ = context_;
        wrapper_ = std::make_unique<JSIteratorWrapper>(context_);
    }

    void TearDown() override {
        // free the wrapper before freeing the context and runtime
        wrapper_.reset();

        JS_FreeContext(context_);
        JS_FreeRuntime(runtime_);
    }

    JSRuntime* runtime_{nullptr};
    JSContext* context_{nullptr};
    std::unique_ptr<JSIteratorWrapper> wrapper_;
};

// Update the test to remove context_ parameter from JSValueRAII construction
TEST_F(QuickJSTest, TestVectorIterator) {
    std::vector<int> numbers{1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
    JSValueRAII iterator(wrapper_->createIterator(numbers));
    ASSERT_FALSE(JS_IsException(iterator));

    constexpr std::string_view script = R"(
        function* filterEvenNumbers(iter) {
            while (iter.hasNext()) {
                const num = iter.next();
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

    std::vector<int> filtered_numbers;
    while (true) {
        JSValueRAII next_method(JS_GetPropertyStr(context_, generator, "next"));
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
