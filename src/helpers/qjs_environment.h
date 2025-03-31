#pragma once

#include <rime/engine.h>
#include <rime/schema.h>

#include <string>

#include <rime_api.h>

#include <cstddef>
#include <cstdio>

#include "node_module_loader.h"
#include "process_memory.hpp"
#include "system_info.hpp"

#include "engines/engine_manager.h"
#include "engines/js_macros.h"

using namespace rime;

template <typename T_JS_VALUE>
class QjsEnvironment {
  DEFINE_CFUNCTION(loadFile, {
    if (argc < 1) {
      return engine.throwError(JsErrorType::SYNTAX, "The absolutePath argument is required");
    }

    std::string path = engine.toStdString(argv[0]);
    if (path.empty()) {
      return engine.throwError(JsErrorType::SYNTAX, "The absolutePath argument should be a string");
    }

    char* content = ::loadFile(path.c_str());
    auto ret = engine.toJsString(content);
    // NOLINTNEXTLINE(cppcoreguidelines-no-malloc)
    free(content);
    return ret;
  });

  static std::string formatMemoryUsage(size_t usage) {
    constexpr size_t KILOBYTE = 1024;
    return usage > KILOBYTE * KILOBYTE ? std::to_string(usage / KILOBYTE / KILOBYTE) + "M"  // in MB
                                       : std::to_string(usage / KILOBYTE) + "K";            // in KB
  }

  DEFINE_CFUNCTION(getRimeInfo, {
    size_t vmUsage = 0;
    size_t residentSet = 0;  // memory usage in bytes
    getMemoryUsage(vmUsage, residentSet);

    JSMemoryUsage qjsMemStats;
    JS_ComputeMemoryUsage(JS_GetRuntime(ctx), &qjsMemStats);

    std::stringstream ss{};
    ss << "libRime v" << rime_get_api()->get_version() << " | "
       << "libRime-qjs v" << RIME_QJS_VERSION << " | "
       << "Process RSS Mem: " << formatMemoryUsage(residentSet) << " | "
       << "QuickJS Mem: " << formatMemoryUsage(qjsMemStats.memory_used_size);

    return engine.toJsString(ss.str().c_str());
  })

  DEFINE_CFUNCTION(fileExists, {
    if (argc < 1) {
      return engine.throwError(JsErrorType::SYNTAX, "The absolutePath argument is required");
    }

    std::string path = engine.toStdString(argv[0]);
    if (path.empty()) {
      return engine.throwError(JsErrorType::SYNTAX, "The absolutePath argument should be a string");
    }

    return engine.toJsBool(std::filesystem::exists(path));
  })

  DEFINE_CFUNCTION(popen, {
    if (argc < 1) {
      return engine.throwError(JsErrorType::SYNTAX, "The command argument is required");
    }

    std::string command = engine.toStdString(argv[0]);
    if (command.empty()) {
      return engine.throwError(JsErrorType::SYNTAX, "The command argument should be a string");
    }

    DLOG(INFO) << "[qjs] popen command: " << command;
    // Open a pipe to the command
#ifdef _WIN32
    FILE* pipe = _popen(command.c_str(), "r");
#else
    FILE* pipe = ::popen(command.c_str(), "r");
#endif
    if (pipe == nullptr) {
      LOG(ERROR) << "Failed to run command: " << command;
      return engine.throwError(JsErrorType::GENERIC, "Failed to run command %s", command.c_str());
    }

    // Read the output
    constexpr size_t READ_BUFFER_SIZE = 128;
    char buffer[READ_BUFFER_SIZE];
    std::string result;
    while (fgets(static_cast<char*>(buffer), sizeof(buffer), pipe) != nullptr) {
      result += static_cast<char*>(buffer);
    }

    // Close the pipe
#ifdef _WIN32
    int status = _pclose(pipe);
#else
    int status = ::pclose(pipe);
#endif
    if (status == 0) {
      DLOG(INFO) << "[qjs] Command output: " << result;
    } else {
      LOG(ERROR) << "[qjs] Command failed with status: " << status;
      return engine.throwError(JsErrorType::GENERIC, "Command failed with status: %d", status);
    }
    return engine.toJsString(result.c_str());
  })

public:
  static T_JS_VALUE create(Engine* engine, const std::string& nameSpace) {
    auto& jsEngine = JsEngine<T_JS_VALUE>::getInstance();
    T_JS_VALUE environment = jsEngine.newObject();  // do not free its properties/methods manually
    jsEngine.setObjectProperty(environment, "engine", jsEngine.wrap(engine));

    T_JS_VALUE jsEngineObj = jsEngine.getObjectProperty(environment, "engine");
    auto* enginePtr = jsEngine.template unwrap<Engine>(jsEngineObj);
    LOG(INFO) << "[qjs] schema_id: " << enginePtr->schema()->schema_id();

    jsEngine.setObjectProperty(environment, "namespace", jsEngine.toJsString(nameSpace.c_str()));
    jsEngine.setObjectProperty(environment, "userDataDir",
                               jsEngine.toJsString(rime_get_api()->get_user_data_dir()));
    jsEngine.setObjectProperty(environment, "sharedDataDir",
                               jsEngine.toJsString(rime_get_api()->get_shared_data_dir()));

    T_JS_VALUE os = jsEngine.newObject();
    jsEngine.setObjectProperty(os, "name", jsEngine.toJsString(SystemInfo::getOSName().c_str()));
    jsEngine.setObjectProperty(os, "version",
                               jsEngine.toJsString(SystemInfo::getOSVersion().c_str()));
    jsEngine.setObjectProperty(os, "architecture",
                               jsEngine.toJsString(SystemInfo::getArchitecture().c_str()));
    jsEngine.setObjectProperty(environment, "os", os);

    // Add utility functions
    jsEngine.setObjectFunction(environment, "loadFile", loadFile, 1);
    jsEngine.setObjectFunction(environment, "fileExists", fileExists, 1);
    jsEngine.setObjectFunction(environment, "getRimeInfo", getRimeInfo, 0);
    jsEngine.setObjectFunction(environment, "popen", popen, 1);

    return environment;
  }
};
