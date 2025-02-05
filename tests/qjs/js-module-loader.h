#ifndef JS_MODULE_LOADER_H
#define JS_MODULE_LOADER_H

#include <string>
#include <fstream>
#include <gtest/gtest.h>
#include <quickjs.h>

std::string base_path = "tests/qjs/js/";

JSModuleDef* js_module_loader(JSContext* ctx,
                             const char* module_name,
                             void* opaque) {
    std::string full_path = base_path + module_name;

    std::ifstream module_file(full_path);
    if (!module_file.is_open()) {
        GTEST_LOG_(ERROR) << "Could not open module file: " << full_path;
        JS_ThrowReferenceError(ctx, "Could not open module file: %s", full_path.c_str());
        return nullptr;
    }

    std::string code((std::istreambuf_iterator<char>(module_file)),
                    std::istreambuf_iterator<char>());

    JSValue val = JS_Eval(ctx, code.c_str(), code.size(), module_name,
                        JS_EVAL_TYPE_MODULE | JS_EVAL_FLAG_COMPILE_ONLY);

    if (JS_IsException(val)) {
        JSValue exception = JS_GetException(ctx);
        JSValue message = JS_GetPropertyStr(ctx, exception, "message");
        const char* message_str = JS_ToCString(ctx, message);
        GTEST_LOG_(ERROR) << "Module evaluation failed: " << message_str;

        JS_FreeCString(ctx, message_str);
        JS_FreeValue(ctx, exception);
        JS_FreeValue(ctx, val);
        return nullptr;
    }

    return reinterpret_cast<JSModuleDef*>(JS_VALUE_GET_PTR(val));
}

JSValue loadMjsFile(JSContext* ctx, const char* fileName) {
    std::string full_path = base_path + fileName;
    std::ifstream jsFile(full_path);
    if (!jsFile.is_open()) {
        GTEST_LOG_(ERROR) << "Could not open" << full_path;
        JS_ThrowReferenceError(ctx, "Could not open runtime-error.js");
        return JS_NULL;
    }

    std::string jsCode((std::istreambuf_iterator<char>(jsFile)),
                        std::istreambuf_iterator<char>());

    return JS_Eval(ctx, jsCode.c_str(), jsCode.size(), "runtime-error.js", JS_EVAL_TYPE_MODULE);
}

#endif
