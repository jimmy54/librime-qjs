#ifndef RIME_QJS_HELPERS_H_

#include <glog/logging.h>
#include <sstream>
#include <quickjs.h>

static JSValue jsLog(JSContext* ctx, JSValueConst this_val, int argc, JSValueConst* argv) {
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

static void registerLogToJsConsole(JSContext* ctx) {
  JSValue global_obj = JS_GetGlobalObject(ctx);
  JSValue console = JS_NewObject(ctx);
  JS_SetPropertyStr(ctx, console, "log", JS_NewCFunction(ctx, jsLog, "log", 1));
  JS_SetPropertyStr(ctx, global_obj, "console", console);
  JS_FreeValue(ctx, global_obj);
}

#endif
