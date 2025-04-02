#pragma once

#include <rime/engine.h>
#include <rime/schema.h>

#include "engines/js_exception.h"
#include "engines/js_macros.h"
#include "environment.h"
#include "js_wrapper.h"

using namespace rime;

template <typename T_JS_VALUE>
class JsWrapper<Environment, T_JS_VALUE> : public JsWrapperBase<T_JS_VALUE> {
  DEFINE_GETTER_2(Environment, engine, engine.wrap(obj->getEngine()))
  DEFINE_GETTER_2(Environment, namespace, engine.toJsString(obj->getNameSpace().c_str()))
  DEFINE_GETTER_2(Environment, os, engine.wrap(obj->getSystemInfo()))
  DEFINE_GETTER_2(Environment,
                  userDataDir,
                  engine.toJsString(Environment::getUserDataDir().c_str()))
  DEFINE_GETTER_2(Environment,
                  sharedDataDir,
                  engine.toJsString(Environment::getSharedDataDir().c_str()))

  DEFINE_CFUNCTION_ARGC(loadFile, 1, {
    std::string path = engine.toStdString(argv[0]);
    if (path.empty()) {
      throw JsException(JsErrorType::SYNTAX, "The absolutePath argument should be a string");
    }
    return engine.toJsString(Environment::loadFile(path).c_str());
  })

  DEFINE_CFUNCTION_ARGC(fileExists, 1, {
    std::string path = engine.toStdString(argv[0]);
    if (path.empty()) {
      throw JsException(JsErrorType::SYNTAX, "The absolutePath argument should be a string");
    }
    return engine.toJsBool(Environment::fileExists(path));
  })

  DEFINE_CFUNCTION(getRimeInfo, {
    auto info = Environment::getRimeInfo();
    int64_t bytes = engine.getMemoryUsage();
    if (bytes >= 0) {
      TypeMap<T_JS_VALUE> typeMap;
      info = info + " | " + typeMap.engineName + " Mem: " + Environment::formatMemoryUsage(bytes);
    }
    return engine.toJsString(info.c_str());
  })

  DEFINE_CFUNCTION_ARGC(popen, 1, {
    std::string command = engine.toStdString(argv[0]);
    if (command.empty()) {
      return engine.throwError(JsErrorType::SYNTAX, "The command argument should be a string");
    }
    std::string result = Environment::popen(command);
    if (result.empty()) {
      return engine.throwError(JsErrorType::GENERIC, "Command execution failed");
    }
    return engine.toJsString(result.c_str());
  })

public:
  static const char* getTypeName() { return "Environment"; }

  EXPORT_SHARED_FINALIZER(Environment, finalizer);
  EXPORT_GETTERS(engine, namespace, userDataDir, sharedDataDir, os);
  EXPORT_FUNCTIONS(loadFile, 1, fileExists, 1, getRimeInfo, 0, popen, 1);
};
