#ifndef RIME_QJS_HELPER_H_
#define RIME_QJS_HELPER_H_

#include <quickjs.h>
#include <string>
// #include <boost/stacktrace.hpp> // Include Boost.Stacktrace
#include <glog/logging.h>

class QjsHelper {
public:
    static QjsHelper& getInstance();

    QjsHelper(const QjsHelper&) = delete;
    QjsHelper& operator=(const QjsHelper&) = delete;

    static JSModuleDef* jsModuleLoader(JSContext* ctx, const char* fileName, void* opaque);
    static JSValue loadJsModuleToNamespace(JSContext* ctx, const char* fileName);
    static JSValue loadJsModuleToGlobalThis(JSContext* ctx, const char* fileName);
    static void exposeLogToJsConsole(JSContext* ctx);

    static std::string basePath;

    // Getters for runtime and context
    JSRuntime* getRuntime() const { return rt; }
    JSContext* getContext() const { return ctx; }

private:
    QjsHelper() {
        rt = JS_NewRuntime();
        ctx = JS_NewContext(rt);
        // LOG(INFO) << "new QjsHelper instance ctx = " << ctx << "\n"
        //          << "Stacktrace:\n" << boost::stacktrace::stacktrace();
    }
    ~QjsHelper() {
        JS_FreeContext(ctx);
        JS_FreeRuntime(rt);
    }

    // Private non-static members
    JSRuntime *rt{nullptr};
    JSContext *ctx{nullptr};

    static std::string readJsCode(JSContext* ctx, const char* fileName);
    static JSValue loadJsModule(JSContext* ctx, const char* fileName);
    static JSValue jsLog(JSContext* ctx, JSValueConst this_val, int argc, JSValueConst* argv);
};

#endif
