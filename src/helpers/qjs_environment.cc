#include "qjs_environment.h"

#include <quickjs.h>
#include <rime_api.h>

#include <cstddef>
#include <cstdio>
#include <iostream>
#include <sstream>

#include "jsstring_raii.h"
#include "process_memory.h"
#include "qjs_engine.h"
#include "qjs_helper.h"

#define SET_FUNCTION_TO_ENVIRONMENT_PROPERTY(funcName, argc) \
  JS_SetPropertyStr(ctx, environment, #funcName, JS_NewCFunction(ctx, funcName, #funcName, argc));

namespace rime {

JSValue QjsEnvironment::create(JSContext* ctx, Engine* engine, const std::string& nameSpace) {
  JSValue environment = JS_NewObject(ctx);  // do not free its properties/methods manually
  JS_SetPropertyStr(ctx, environment, "engine", QjsEngine::wrap(ctx, engine));
  JS_SetPropertyStr(ctx, environment, "namespace", JS_NewString(ctx, nameSpace.c_str()));

  auto jsUserDataDir = JS_NewString(ctx, rime_get_api()->get_user_data_dir());
  JS_SetPropertyStr(ctx, environment, "userDataDir", jsUserDataDir);

  auto jsSharedDataDir = JS_NewString(ctx, rime_get_api()->get_shared_data_dir());
  JS_SetPropertyStr(ctx, environment, "sharedDataDir", jsSharedDataDir);

  // Add utility functions
  SET_FUNCTION_TO_ENVIRONMENT_PROPERTY(loadFile, 1);
  SET_FUNCTION_TO_ENVIRONMENT_PROPERTY(fileExists, 1);
  SET_FUNCTION_TO_ENVIRONMENT_PROPERTY(getRimeInfo, 0);
  SET_FUNCTION_TO_ENVIRONMENT_PROPERTY(popen, 1);

  return environment;
}

JSValue QjsEnvironment::loadFile(JSContext* ctx,
                                 JSValueConst thisVal,
                                 int argc,
                                 JSValueConst* argv) {
  if (argc < 1) {
    return JS_ThrowSyntaxError(ctx, "The absolutePath argument is required");
  }

  const char* path = JS_ToCString(ctx, argv[0]);
  if (path == nullptr) {
    return JS_ThrowSyntaxError(ctx, "The absolutePath argument should be a string");
  }

  std::string content = QjsHelper::loadFile(path);
  JS_FreeCString(ctx, path);

  return JS_NewString(ctx, content.c_str());
}

static std::string formatMemoryUsage(size_t usage) {
  constexpr size_t KILOBYTE = 1024;
  return usage > KILOBYTE * KILOBYTE ? std::to_string(usage / KILOBYTE / KILOBYTE) + "M"  // in MB
                                     : std::to_string(usage / KILOBYTE) + "K";            // in KB
}

JSValue QjsEnvironment::getRimeInfo(JSContext* ctx,
                                    JSValueConst thisVal,
                                    int argc,
                                    JSValueConst* argv) {
  size_t vmUsage = 0;
  size_t residentSet = 0;  // memory usage in bytes
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

JSValue QjsEnvironment::fileExists(JSContext* ctx,
                                   JSValueConst thisVal,
                                   int argc,
                                   JSValueConst* argv) {
  if (argc < 1) {
    return JS_ThrowSyntaxError(ctx, "The absolutePath argument is required");
  }

  JSStringRAII path = JS_ToCString(ctx, argv[0]);
  if (path.cStr() == nullptr) {
    return JS_ThrowSyntaxError(ctx, "The absolutePath argument should be a string");
  }

  bool exists = std::filesystem::exists(path.cStr());
  return JS_NewBool(ctx, exists);
}

JSValue QjsEnvironment::popen(JSContext* ctx, JSValueConst thisVal, int argc, JSValueConst* argv) {
  if (argc < 1) {
    return JS_ThrowSyntaxError(ctx, "The command argument is required");
  }

  JSStringRAII command = JS_ToCString(ctx, argv[0]);
  if (command.cStr() == nullptr) {
    return JS_ThrowSyntaxError(ctx, "The command argument should be a string");
  }

  DLOG(INFO) << "[qjs] popen command: " << command.cStr();
  // Open a pipe to the command
  FILE* pipe = ::popen(command.cStr(), "r");
  if (pipe == nullptr) {
    LOG(ERROR) << "Failed to run command: " << command.cStr();
    return JS_ThrowPlainError(ctx, "Failed to run command %s", command.cStr());
  }

  // Read the output
  constexpr size_t READ_BUFFER_SIZE = 128;
  char buffer[READ_BUFFER_SIZE];
  std::string result;
  while (fgets(static_cast<char*>(buffer), sizeof(buffer), pipe) != nullptr) {
    result += static_cast<char*>(buffer);
  }

  // Close the pipe
  int status = pclose(pipe);
  if (status == 0) {
    DLOG(INFO) << "[qjs] Command output: " << result;
  } else {
    LOG(ERROR) << "[qjs] Command failed with status: " << status;
    return JS_ThrowPlainError(ctx, "Command failed with status: %d", status);
  }
  return JS_NewString(ctx, result.c_str());
}

}  // namespace rime
