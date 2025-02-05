#include <quickjs.h>
#include <fstream>
#include <string>
#include <iostream>
#include <gtest/gtest.h>

// Module loader implementation
JSModuleDef* js_module_loader(JSContext* ctx,
                             const char* module_name,
                             void* opaque) {
    // Construct the full path relative to tests/qjs directory
    std::string base_path = "tests/qjs/";
    std::string full_path = base_path + module_name;

    std::ifstream module_file(full_path);
    if (!module_file.is_open()) {
        JS_ThrowReferenceError(ctx, "Could not open module file: %s", full_path.c_str());
        return nullptr;
    }

    std::string code((std::istreambuf_iterator<char>(module_file)),
                    std::istreambuf_iterator<char>());

    JSValue val = JS_Eval(ctx, code.c_str(), code.size(), module_name,
                        JS_EVAL_TYPE_MODULE | JS_EVAL_FLAG_COMPILE_ONLY);

    if (JS_IsException(val)) {
        JS_FreeValue(ctx, val);
        return nullptr;
    }

    return reinterpret_cast<JSModuleDef*>(JS_VALUE_GET_PTR(val));
}

TEST(MainJSTest, TestMyClassGreet) {
    JSRuntime* rt = JS_NewRuntime();
    JSContext* ctx = JS_NewContext(rt);
    JS_SetModuleLoaderFunc(rt, nullptr, js_module_loader, nullptr);

    // Read and evaluate main.js
    std::ifstream main_file("tests/qjs/main.js");
    ASSERT_TRUE(main_file.is_open()) << "Could not open main.js";

    std::string main_code((std::istreambuf_iterator<char>(main_file)),
                        std::istreambuf_iterator<char>());
    // GTEST_LOG_(INFO) << "main_code: " << main_code;

    JSValue module = JS_Eval(ctx, main_code.c_str(), main_code.size(), "main.js", JS_EVAL_TYPE_MODULE);
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
