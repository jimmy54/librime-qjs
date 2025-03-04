#include <rime_api.h>
#include <quickjs.h>
#include <sstream>
#include <cstdio>
#include <iostream>

#include "qjs_environment.h"
#include "jsstring_raii.h"
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
  JS_SetPropertyStr(ctx, environment, "popen", JS_NewCFunction(ctx, popen, "popen", 1));
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

JSValue QjsEnvironment::popen(JSContext* ctx, JSValueConst this_val, int argc, JSValueConst* argv) {
  if (argc < 1) {
    return JS_ThrowSyntaxError(ctx, "The command argument is required");
  }

  auto command = JSStringRAII(JS_ToCString(ctx, argv[0]));
  if (!command.c_str()) {
    return JS_ThrowSyntaxError(ctx, "The command argument should be a string");
  }

  // Open a pipe to the command
  FILE* pipe = ::popen(command.c_str(), "r");
  if (!pipe) {
    LOG(ERROR) << "Failed to run command: " << command.c_str();
    return JS_ThrowPlainError(ctx, "Failed to run command %s", command.c_str());
  }

  // Read the output
  char buffer[128];
  std::string result;
  while (fgets(buffer, sizeof(buffer), pipe) != nullptr) {
      result += buffer;
  }

  // Close the pipe
  int status = pclose(pipe);
  if (status == 0) {
    DLOG(INFO) << "Command output: " << result;
  } else {
    LOG(ERROR) << "Command failed with status: " << status;
    return JS_ThrowPlainError(ctx, "Command failed with status: %d", status);
  }
  return JS_NewString(ctx, result.c_str());
}

} // namespace rime
