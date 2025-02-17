#include <quickjs.h>
#include <fstream>
#include <string>
#include <iostream>
#include <gtest/gtest.h>

#include "qjs_helper.h"

std::string trim(const std::string& str) {
    const auto start = str.find_first_not_of(" \t\n\r");
    if (start == std::string::npos) return "";

    const auto end = str.find_last_not_of(" \t\n\r");
    return str.substr(start, end - start + 1);
}

class QuickJSErrorTest : public ::testing::Test {
protected:
    JSContext* ctx;

    void SetUp() override {
        ctx = QjsHelper::getInstance().getContext();
        auto rt = JS_GetRuntime(ctx);
        JS_SetModuleLoaderFunc(rt, nullptr, QjsHelper::jsModuleLoader, nullptr);
        QjsHelper::exposeLogToJsConsole(ctx);
        QjsHelper::basePath = "tests/qjs/js";
    }
};

TEST_F(QuickJSErrorTest, TestJsRuntimeError) {
    JSValue module = QjsHelper::loadJsModuleToGlobalThis(ctx, "runtime-error.js");
    JSValue global_obj = JS_GetGlobalObject(ctx);
    JSValue func = JS_GetPropertyStr(ctx, global_obj, "funcWithRuntimeError");
    ASSERT_FALSE(JS_IsException(func));

    JSValue result = JS_Call(ctx, func, JS_UNDEFINED, 0, nullptr);
    ASSERT_TRUE(JS_IsException(result));

    // ReferenceError: abcdefg is not defined
    //      at <anonymous> (runtime-error.js:7:21)
    JSValue exception = JS_GetException(ctx);
    const char *message = JS_ToCString(ctx, exception);
    ASSERT_STREQ(message, "ReferenceError: abcdefg is not defined");
    JS_FreeCString(ctx, message);

    JSValue stack = JS_GetPropertyStr(ctx, exception, "stack");
    const char *stack_trace = JS_ToCString(ctx, stack);
    std::string trimmed_stack_trace = trim(stack_trace);
    ASSERT_STREQ(trimmed_stack_trace.c_str(), "at <anonymous> (runtime-error.js:7:21)");
    JS_FreeCString(ctx, stack_trace);
    JS_FreeValue(ctx, stack);

    JS_FreeValue(ctx, exception);
    JS_FreeValue(ctx, result);

    JS_FreeValue(ctx, func);
    JS_FreeValue(ctx, global_obj);
    JS_FreeValue(ctx, module);
}
