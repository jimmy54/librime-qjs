#include <quickjs.h>
#include <fstream>
#include <string>
#include <iostream>
#include <gtest/gtest.h>

#include "qjs_helper.h"
#include "jsvalue_raii.h"

using namespace rime;

class QuickJSModuleTest : public testing::Test {
protected:
    JSContext* ctx_;

    void SetUp() override {
        ctx_ = QjsHelper::getInstance().getContext();
        QjsHelper::basePath = "tests/qjs/js";
    }
};

TEST_F(QuickJSModuleTest, ImportJsModuleFromAnotherJsFile) {
    JSValueRAII module(QjsHelper::loadJsModuleToGlobalThis(ctx_, "main.js"));

    JSValueRAII global_obj(JS_GetGlobalObject(ctx_));
    JSValueRAII myClass(JS_GetPropertyStr(ctx_, global_obj, "MyClass"));
    ASSERT_FALSE(JS_IsException(myClass));

    JSValueRAII arg(JS_NewInt32(ctx_, 10));
    JSValueRAII obj(JS_CallConstructor(ctx_, myClass, 1, arg.getPtr()));
    ASSERT_FALSE(JS_IsException(obj));

    JSValueRAII greet_arg(JS_NewString(ctx_, "QuickJS"));
    JSAtom greet_atom = JS_NewAtom(ctx_, "greet");
    JSValueRAII greet_result(JS_Invoke(ctx_, obj, greet_atom, 1, greet_arg.getPtr()));
    ASSERT_FALSE(JS_IsException(greet_result));
    JS_FreeAtom(ctx_, greet_atom);

    const char* str = JS_ToCString(ctx_, greet_result);
    ASSERT_TRUE(str != nullptr);
    EXPECT_STREQ(str, "Hello QuickJS!");
    JS_FreeCString(ctx_, str);
}

TEST_F(QuickJSModuleTest, ImportJsModuleToNamespace) {
    JSValueRAII moduleNamespace(QjsHelper::loadJsModuleToNamespace(ctx_, "lib.js"));
    ASSERT_FALSE(JS_IsException(moduleNamespace));

    // Get the greet function from the namespace
    JSValueRAII greetFunc(JS_GetPropertyStr(ctx_, moduleNamespace, "greet"));
    ASSERT_FALSE(JS_IsException(greetFunc));

    JSValueRAII arg(JS_NewString(ctx_, "QuickJS"));
    JSValue args[] = { arg.get() };
    JSValueRAII result(JS_Call(ctx_, greetFunc, JS_UNDEFINED, 1, args));
    ASSERT_FALSE(JS_IsException(result));

    // Verify the result
    const char* str = JS_ToCString(ctx_, result);
    ASSERT_TRUE(str != nullptr);
    EXPECT_STREQ(str, "Hello QuickJS!");
    JS_FreeCString(ctx_, str);
}
