#ifndef RIME_QJS_HELPER_H_
#define RIME_QJS_HELPER_H_

#include <quickjs.h>

#include <string>
// #include <boost/stacktrace.hpp> // Include Boost.Stacktrace
#include <glog/logging.h>

class QjsHelper {
public:
  static QjsHelper& getInstance();

  QjsHelper(QjsHelper&&) = delete;
  QjsHelper& operator=(QjsHelper&&) = delete;
  QjsHelper(const QjsHelper&) = delete;
  QjsHelper& operator=(const QjsHelper&) = delete;

  static JSValue loadJsModuleToNamespace(JSContext* ctx, const char* moduleName);
  static JSValue loadJsModuleToGlobalThis(JSContext* ctx, const char* moduleName);
  static void exposeLogToJsConsole(JSContext* ctx);
  static JSValue getExportedClassHavingMethodNameInModule(JSContext* ctx,
                                                          JSValue moduleObj,
                                                          const char* methodName);
  static JSValue getExportedClassByNameInModule(JSContext* ctx,
                                                JSValue moduleObj,
                                                const char* className);
  static JSValue getMethodByNameInClass(JSContext* ctx, JSValue classObj, const char* methodName);

  // Getters for runtime and context
  [[nodiscard]] JSRuntime* getRuntime() const { return rt_; }
  [[nodiscard]] JSContext* getContext() const { return ctx_; }

private:
  QjsHelper() : rt_(JS_NewRuntime()), ctx_(JS_NewContext(rt_)) {
    // LOG(INFO) << "new QjsHelper instance ctx = " << ctx << "\n"
    //          << "Stacktrace:\n" << boost::stacktrace::stacktrace();
  }
  ~QjsHelper() {
    JS_FreeContext(ctx_);
    JS_FreeRuntime(rt_);
  }

  // Private non-static members
  JSRuntime* rt_{nullptr};
  JSContext* ctx_{nullptr};

  static JSValue jsLog(JSContext* ctx, JSValueConst thisVal, int argc, JSValueConst* argv);
  static JSValue jsError(JSContext* ctx, JSValueConst thisVal, int argc, JSValueConst* argv);
};

#endif
