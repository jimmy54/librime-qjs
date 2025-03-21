#include "qjs_helper.h"

#include <glog/logging.h>

#include <cstddef>
#include <sstream>

#include "jsvalue_raii.h"
#include "quickjs.h"

#include "node_module_loader.h"

using namespace rime;

QjsHelper& QjsHelper::getInstance() {
  static QjsHelper instance;
  return instance;
}

JSValue QjsHelper::loadJsModuleToNamespace(JSContext* ctx, const char* moduleName) {
  JSValue funcObj = loadJsModule(ctx, moduleName);
  if (JS_IsException(funcObj)) {
    return funcObj;
  }

  auto* md = reinterpret_cast<JSModuleDef*>(JS_VALUE_GET_PTR(funcObj));
  JSValue evalResult = JS_EvalFunction(ctx, funcObj);
  if (JS_IsException(funcObj)) {
    return evalResult;
  }

  JS_FreeValue(ctx, evalResult);
  return JS_GetModuleNamespace(ctx, md);
}

JSValue QjsHelper::loadJsModuleToGlobalThis(JSContext* ctx, const char* moduleName) {
  char* jsCode = readJsCode(ctx, moduleName);
  std::string jsCodeStr(jsCode);
  // NOLINTNEXTLINE(cppcoreguidelines-no-malloc)
  free(jsCode);
  return JS_Eval(ctx, jsCodeStr.c_str(), jsCodeStr.size(), moduleName, JS_EVAL_TYPE_MODULE);
}

void QjsHelper::exposeLogToJsConsole(JSContext* ctx) {
  JSValue globalObj = JS_GetGlobalObject(ctx);
  JSValue console = JS_NewObject(ctx);
  JS_SetPropertyStr(ctx, console, "log", JS_NewCFunction(ctx, jsLog, "log", 1));
  JS_SetPropertyStr(ctx, console, "error", JS_NewCFunction(ctx, jsError, "error", 1));
  JS_SetPropertyStr(ctx, globalObj, "console", console);
  JS_FreeValue(ctx, globalObj);
}

static std::string logToStringStream(JSContext* ctx, int argc, JSValueConst* argv) {
  // FIXME: Unicode characters display incorrectly on Windows, showing "鉁?" instead of "✓".
  // While quickjs-ng's `js_print` function could help with console output,
  // I haven't found a way to integrate it with glog's `LOG(severity)` statements.
  // See related fix: https://github.com/quickjs-ng/quickjs/pull/449/files
  std::ostringstream oss;
  for (int i = 0; i < argc; i++) {
    const char* str = JS_ToCString(ctx, argv[i]);
    if (str != nullptr) {
      oss << (i != 0 ? " " : "") << str;
      JS_FreeCString(ctx, str);
    }
  }
  return oss.str();
}

JSValue QjsHelper::jsLog(JSContext* ctx, JSValueConst thisVal, int argc, JSValueConst* argv) {
  LOG(INFO) << "$qjs$ " << logToStringStream(ctx, argc, argv);
  return JS_UNDEFINED;
}

JSValue QjsHelper::jsError(JSContext* ctx, JSValueConst thisVal, int argc, JSValueConst* argv) {
  LOG(ERROR) << "$qjs$ " << logToStringStream(ctx, argc, argv);
  return JS_UNDEFINED;
}

JSValue QjsHelper::getMethodByNameInClass(JSContext* ctx,
                                          JSValue classObj,
                                          const char* methodName) {
  JSValueRAII proto = JS_GetPropertyStr(ctx, classObj, "prototype");
  if (JS_IsException(proto)) {
    return JS_UNDEFINED;
  }

  JSValue method = JS_GetPropertyStr(ctx, proto, methodName);
  if (JS_IsException(method) || !JS_IsFunction(ctx, method)) {
    JS_FreeValue(ctx, method);
    return JS_UNDEFINED;
  }

  return method;
}

JSValue QjsHelper::getExportedClassByNameInModule(JSContext* ctx,
                                                  JSValue moduleObj,
                                                  const char* className) {
  JSPropertyEnum* props = nullptr;
  uint32_t propCount = 0;  // Get all enumerable properties from namespace
  int flags = JS_GPN_STRING_MASK | JS_GPN_SYMBOL_MASK | JS_GPN_ENUM_ONLY;
  if (JS_GetOwnPropertyNames(ctx, &props, &propCount, moduleObj, flags) == 0) {
    size_t n = strlen(className);
    for (uint32_t i = 0; i < propCount; i++) {
      JSValue propVal = JS_GetProperty(ctx, moduleObj, props[i].atom);
      const char* propName = JS_AtomToCString(ctx, props[i].atom);
      bool isMatched = !JS_IsException(propVal) && strncmp(propName, className, n) == 0;

      JS_FreeCString(ctx, propName);
      JS_FreeAtom(ctx, props[i].atom);

      if (isMatched) {
        js_free(ctx, props);
        return propVal;
      }

      JS_FreeValue(ctx, propVal);
    }
  }

  js_free(ctx, props);
  return JS_UNDEFINED;
}

JSValue QjsHelper::getExportedClassHavingMethodNameInModule(JSContext* ctx,
                                                            JSValue moduleObj,
                                                            const char* methodName) {
  JSPropertyEnum* props = nullptr;
  uint32_t propCount = 0;  // Get all enumerable properties from namespace
  int flags = JS_GPN_STRING_MASK | JS_GPN_SYMBOL_MASK | JS_GPN_ENUM_ONLY;
  if (JS_GetOwnPropertyNames(ctx, &props, &propCount, moduleObj, flags) == 0) {
    for (uint32_t i = 0; i < propCount; i++) {
      JSValue propVal = JS_GetProperty(ctx, moduleObj, props[i].atom);
      const char* propName = JS_AtomToCString(ctx, props[i].atom);

      bool found = false;
      if (JS_IsObject(propVal)) {
        JSValueRAII method = getMethodByNameInClass(ctx, propVal, methodName);
        found = !JS_IsException(method) && !JS_IsUndefined(method) && JS_IsFunction(ctx, method);
      }

      JS_FreeCString(ctx, propName);
      JS_FreeAtom(ctx, props[i].atom);

      if (found) {
        js_free(ctx, props);
        return propVal;
      }

      JS_FreeValue(ctx, propVal);
    }
  }

  js_free(ctx, props);
  return JS_UNDEFINED;
}
