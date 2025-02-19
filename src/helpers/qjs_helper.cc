#include "qjs_helper.h"
#include <glog/logging.h>
#include <sstream>
#include <fstream>

std::string QjsHelper::basePath = "";

QjsHelper& QjsHelper::getInstance() {
    static QjsHelper instance;
    return instance;
}

JSModuleDef* QjsHelper::jsModuleLoader(JSContext* ctx, const char* fileName, void* opaque) {
    JSValue funcObj = loadJsModule(ctx, fileName);
    return reinterpret_cast<JSModuleDef*>(JS_VALUE_GET_PTR(funcObj));
}

JSValue QjsHelper::loadJsModuleToNamespace(JSContext* ctx, const char* fileName) {
    JSValue funcObj = loadJsModule(ctx, fileName);
    if (JS_IsException(funcObj)) {
        return funcObj;
    }

    JSModuleDef* md = reinterpret_cast<JSModuleDef*>(JS_VALUE_GET_PTR(funcObj));
    JSValue evalResult = JS_EvalFunction(ctx, funcObj);
    if (JS_IsException(funcObj)) {
        return evalResult;
    }

    return JS_GetModuleNamespace(ctx, md);
}

JSValue QjsHelper::loadJsModuleToGlobalThis(JSContext* ctx, const char* fileName) {
    std::string jsCode = readJsCode(ctx, fileName);
    return JS_Eval(ctx, jsCode.c_str(), jsCode.size(), fileName, JS_EVAL_TYPE_MODULE);
}

void QjsHelper::exposeLogToJsConsole(JSContext* ctx) {
    JSValue global_obj = JS_GetGlobalObject(ctx);
    JSValue console = JS_NewObject(ctx);
    JS_SetPropertyStr(ctx, console, "log", JS_NewCFunction(ctx, jsLog, "log", 1));
    JS_SetPropertyStr(ctx, global_obj, "console", console);
    JS_FreeValue(ctx, global_obj);
}

std::string QjsHelper::loadFile(const char* absolutePath) {
    std::ifstream stream(absolutePath);
    if (!stream.is_open()) {
        LOG(ERROR) << "Failed to open file at: " << absolutePath;
        return "";
    }
    std::string content((std::istreambuf_iterator<char>(stream)),
                         std::istreambuf_iterator<char>());
    return content;
}

std::string QjsHelper::readJsCode(JSContext* ctx, const char* fileName) {
    if (basePath.empty()) {
        LOG(ERROR) << "basePath is empty in loading js file: " << fileName;
        JS_ThrowReferenceError(ctx, "basePath is empty in loading js file: %s", fileName);
        return "";
    }
    std::string fullPath = basePath + "/" + fileName;
    return loadFile(fullPath.c_str());
}

JSValue QjsHelper::loadJsModule(JSContext* ctx, const char* fileName) {
    std::string code = readJsCode(ctx, fileName);
    if (code == "") {
        return JS_ThrowReferenceError(ctx, "Could not open %s", fileName);
    }

    JSValue funcObj = JS_Eval(ctx, code.c_str(), code.size(), fileName,
                        JS_EVAL_TYPE_MODULE | JS_EVAL_FLAG_COMPILE_ONLY);

    if (JS_IsException(funcObj)) {
        JSValue exception = JS_GetException(ctx);
        JSValue message = JS_GetPropertyStr(ctx, exception, "message");
        const char* message_str = JS_ToCString(ctx, message);
        LOG(ERROR) << "Module evaluation failed: " << message_str;

        JS_FreeCString(ctx, message_str);
        JS_FreeValue(ctx, exception);
    }
    return funcObj;
}

JSValue QjsHelper::jsLog(JSContext* ctx, JSValueConst this_val, int argc, JSValueConst* argv) {
    std::ostringstream oss;
    for (int i = 0; i < argc; i++) {
        const char* str = JS_ToCString(ctx, argv[i]);
        if (str) {
            oss << (i ? " " : "") << str;
            JS_FreeCString(ctx, str);
        }
    }
    LOG(INFO) << "$qjs$ " << oss.str();
    return JS_UNDEFINED;
}
