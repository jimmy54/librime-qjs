#pragma once

#include <rime/engine.h>
#include <rime/schema.h>

#include <cstdint>
#include <string>

#include <rime_api.h>

#include <cstddef>
#include <cstdio>

#include "engines/type_map.h"
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

    std::stringstream ss{};
    ss << "libRime v" << rime_get_api()->get_version() << " | "
       << "libRime-qjs v" << RIME_QJS_VERSION << " | "
       << "Process RSS Mem: " << formatMemoryUsage(residentSet);

    int64_t bytes = engine.getMemoryUsage();
    if (bytes >= 0) {
      TypeMap<T_JS_VALUE> typeMap;
      ss << " | " << typeMap.engineName << " Mem: " << formatMemoryUsage(bytes);
    }

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

  static FILE* popenx(const std::string& command) {
#ifdef _WIN32
    return _popen(command.c_str(), "r");
#else
    return ::popen(command.c_str(), "r");
#endif
  }

  static int pclosex(FILE* pipe) {
#ifdef _WIN32
    return _pclose(pipe);
#else
    return ::pclose(pipe);
#endif
  }

  DEFINE_CFUNCTION(popen, {
    if (argc < 1) {
      return engine.throwError(JsErrorType::SYNTAX, "The command argument is required");
    }

    std::string command = engine.toStdString(argv[0]);
    if (command.empty()) {
      return engine.throwError(JsErrorType::SYNTAX, "The command argument should be a string");
    }

    FILE* pipe = popenx(command);
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

    int status = pclosex(pipe);
    if (status != 0) {
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
