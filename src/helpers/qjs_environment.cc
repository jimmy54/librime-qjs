#include <rime_api.h>
#include <quickjs.h>
#include <sstream>
#include "qjs_environment.h"
#include "process_memory.h"

namespace rime {

JSValueRAII QjsEnvironment::Create(JSContext* ctx, Engine* engine, const std::string& name_space) {
  JSValueRAII environment(JS_NewObject(ctx)); // do not free its properties/methods manually
  JS_SetPropertyStr(ctx, environment, "engine", QjsEngine::Wrap(ctx, engine));
  JS_SetPropertyStr(ctx, environment, "namespace", JS_NewString(ctx, name_space.c_str()));
  JS_SetPropertyStr(ctx, environment, "userDataDir", JS_NewString(ctx, QjsHelper::basePath.c_str()));

  // Add utility functions
  AddUtilityFunctions(ctx, environment);

  return environment;
}

void QjsEnvironment::AddUtilityFunctions(JSContext* ctx, JSValue environment) {
  JS_SetPropertyStr(ctx, environment, "loadFile", JS_NewCFunction(ctx, loadFile, "loadFile", 1));
  JS_SetPropertyStr(ctx, environment, "fileExists", JS_NewCFunction(ctx, fileExists, "fileExists", 1));
  JS_SetPropertyStr(ctx, environment, "getRimeInfo", JS_NewCFunction(ctx, getRimeInfo, "getRimeInfo", 0));
}

bool QjsEnvironment::CallInitFunction(JSContext* ctx, JSValue moduleNamespace, JSValue environment) {
  JSValueRAII initFunc(JS_GetPropertyStr(ctx, moduleNamespace, "init"));
  if (!JS_IsUndefined(initFunc)) {
    JSValueRAII initResult(JS_Call(ctx, initFunc, JS_UNDEFINED, 1, &environment));
    if (JS_IsException(initResult)) {
      return false;
    }
    return true;
  }
  return true; // No init function is not an error
}

bool QjsEnvironment::CallFinitFunction(JSContext* ctx, JSValue finitFunc, JSValue environment) {
  if (!JS_IsUndefined(finitFunc)) {
    JSValueRAII finitResult(JS_Call(ctx, finitFunc, JS_UNDEFINED, 1, &environment));
    if (JS_IsException(finitResult)) {
      return false;
    }
  }
  return true;
}

JSValue QjsEnvironment::loadFile(JSContext* ctx, JSValueConst this_val, int argc, JSValueConst* argv) {
  if (argc < 1) {
    return JS_ThrowSyntaxError(ctx, "The absolutePath argument is required");
  }

  const char* path = JS_ToCString(ctx, argv[0]);
  if (!path) {
    return JS_ThrowSyntaxError(ctx, "The absolutePath argument should be a string");
  }

  std::string content = QjsHelper::loadFile(path);
  JS_FreeCString(ctx, path);

  return JS_NewString(ctx, content.c_str());
}

static std::string formatMemoryUsage(size_t usage) {
  return usage > 1024 * 1024
    ? std::to_string(usage / 1024 / 1024) + "M" // in MB
    : std::to_string(usage / 1024) + "K"; // in KB
}

JSValue QjsEnvironment::getRimeInfo(JSContext* ctx, JSValueConst this_val, int argc, JSValueConst* argv) {
  size_t vmUsage, residentSet; // memory usage in bytes
  getMemoryUsage(vmUsage, residentSet);

  JSMemoryUsage qjsMemStats;
  JS_ComputeMemoryUsage(JS_GetRuntime(ctx), &qjsMemStats);

  std::stringstream ss;
  ss << "libRime v" << rime_get_api()->get_version() << " | "
     << "libRime-qjs v" << RIME_QJS_VERSION << " | "
     << "Process RSS Mem: " << formatMemoryUsage(residentSet) << " | "
     << "QuickJS Mem: " << formatMemoryUsage(qjsMemStats.memory_used_size);

  return JS_NewString(ctx, ss.str().c_str());
}

JSValue QjsEnvironment::fileExists(JSContext* ctx, JSValueConst this_val, int argc, JSValueConst* argv) {
  if (argc < 1) {
    return JS_ThrowSyntaxError(ctx, "The absolutePath argument is required");
  }

  const char* path = JS_ToCString(ctx, argv[0]);
  if (!path) {
    return JS_ThrowSyntaxError(ctx, "The absolutePath argument should be a string");
  }

  bool exists = std::filesystem::exists(path);
  JS_FreeCString(ctx, path);

  return JS_NewBool(ctx, exists);
}

} // namespace rime
