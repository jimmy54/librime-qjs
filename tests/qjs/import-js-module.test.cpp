#include <quickjs.h>
#include <fstream>
#include <string>
#include <iostream>
#include <gtest/gtest.h>

#include "./js-module-loader.h"

TEST(QuickJSTest, TestImportJsModule) {
    JSRuntime* rt = JS_NewRuntime();
    JSContext* ctx = JS_NewContext(rt);
    JS_SetModuleLoaderFunc(rt, nullptr, js_module_loader, nullptr);

    JSValue module = loadMjsFile(ctx, "main.js");
    JS_FreeValue(ctx, module);

    // Get the global object and MyClass
    JSValue global_obj = JS_GetGlobalObject(ctx);
    JSValue myClass = JS_GetPropertyStr(ctx, global_obj, "MyClass");
    ASSERT_FALSE(JS_IsException(myClass));

    // Create an instance of MyClass with value 10
    JSValue args[] = { JS_NewInt32(ctx, 10) };
    JSValue obj = JS_CallConstructor(ctx, myClass, 1, args);
    ASSERT_FALSE(JS_IsException(obj));

    // Call the greet method with "QuickJS" argument
    JSValue greet_arg = JS_NewString(ctx, "QuickJS");
    JSAtom greet_atom = JS_NewAtom(ctx, "greet");
    JSValue greet_result = JS_Invoke(ctx, obj, greet_atom, 1, &greet_arg);
    ASSERT_FALSE(JS_IsException(greet_result));
    JS_FreeAtom(ctx, greet_atom);

    // Verify the result
    const char* str = JS_ToCString(ctx, greet_result);
    ASSERT_TRUE(str != nullptr);
    EXPECT_STREQ(str, "Hello QuickJS!");
    JS_FreeCString(ctx, str);

    // Clean up
    JS_FreeValue(ctx, greet_arg);
    JS_FreeValue(ctx, greet_result);
    JS_FreeValue(ctx, obj);
    JS_FreeValue(ctx, myClass);
    JS_FreeValue(ctx, global_obj);
    JS_FreeValue(ctx, args[0]);

    JS_FreeContext(ctx);
    JS_FreeRuntime(rt);
}
