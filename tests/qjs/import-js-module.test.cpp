#include <quickjs.h>
#include <string>
#include <gtest/gtest.h>

#include "qjs_helper.h"
#include "jsvalue_raii.h"

using namespace rime;

class QuickJSModuleTest : public testing::Test {
protected:
    void SetUp() override {
        QjsHelper::basePath = "tests/qjs/js";
    }
};

// NOLINTNEXTLINE(cppcoreguidelines-avoid-non-const-global-variables, readability-function-cognitive-complexity)
TEST_F(QuickJSModuleTest, ImportJsModuleFromAnotherJsFile) {
    auto* ctx = QjsHelper::getInstance().getContext();
    JSValueRAII module(QjsHelper::loadJsModuleToGlobalThis(ctx, "main.js"));

    JSValueRAII globalObj(JS_GetGlobalObject(ctx));
    JSValueRAII myClass(JS_GetPropertyStr(ctx, globalObj, "MyClass"));
    ASSERT_FALSE(JS_IsException(myClass));

    constexpr int A_NAMED_INT = 10;
    JSValueRAII arg(JS_NewInt32(ctx, A_NAMED_INT));
    JSValueRAII obj(JS_CallConstructor(ctx, myClass, 1, arg.getPtr()));
    ASSERT_FALSE(JS_IsException(obj));

    JSValueRAII greetArg(JS_NewString(ctx, "QuickJS"));
    JSAtom greetAtom = JS_NewAtom(ctx, "greet");
    JSValueRAII greetResult(JS_Invoke(ctx, obj, greetAtom, 1, greetArg.getPtr()));
    ASSERT_FALSE(JS_IsException(greetResult));
    JS_FreeAtom(ctx, greetAtom);

    const char* str = JS_ToCString(ctx, greetResult);
    ASSERT_TRUE(str != nullptr);
    EXPECT_STREQ(str, "Hello QuickJS!");
    JS_FreeCString(ctx, str);
}

// NOLINTNEXTLINE(cppcoreguidelines-avoid-non-const-global-variables, readability-function-cognitive-complexity)
TEST_F(QuickJSModuleTest, ImportJsModuleToNamespace) {
    auto* ctx = QjsHelper::getInstance().getContext();
    JSValueRAII moduleNamespace(QjsHelper::loadJsModuleToNamespace(ctx, "lib.js"));
    ASSERT_FALSE(JS_IsException(moduleNamespace));

    // Get the greet function from the namespace
    JSValueRAII greetFunc(JS_GetPropertyStr(ctx, moduleNamespace, "greet"));
    ASSERT_FALSE(JS_IsException(greetFunc));

    JSValueRAII arg(JS_NewString(ctx, "QuickJS"));
    JSValue args[] = { arg.get() };
    JSValueRAII result(JS_Call(ctx, greetFunc, JS_UNDEFINED, 1, static_cast<JSValue*>(args)));
    ASSERT_FALSE(JS_IsException(result));

    // Verify the result
    const char* str = JS_ToCString(ctx, result);
    ASSERT_TRUE(str != nullptr);
    EXPECT_STREQ(str, "Hello QuickJS!");
    JS_FreeCString(ctx, str);
}
