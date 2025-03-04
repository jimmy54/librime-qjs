#ifndef RIME_QJS_ENVIRONMENT_H_
#define RIME_QJS_ENVIRONMENT_H_

#include <rime/engine.h>
#include "quickjs.h"
#include "jsvalue_raii.h"
#include "qjs_helper.h"
#include "qjs_engine.h"

#include <string>
#include <filesystem>

namespace rime {

class QjsEnvironment {
public:
  // Create and initialize a JavaScript environment object
  static JSValueRAII Create(JSContext* ctx, Engine* engine, const std::string& name_space);

  // Add utility functions to the environment
  static void AddUtilityFunctions(JSContext* ctx, JSValue environment);
  
private:
  // Utility function to load file content
  static JSValue loadFile(JSContext* ctx, JSValueConst this_val, int argc, JSValueConst* argv);

  // Utility function to check if file exists
  static JSValue fileExists(JSContext* ctx, JSValueConst this_val, int argc, JSValueConst* argv);

  // Utility function to get the Rime infomation
  static JSValue getRimeInfo(JSContext* ctx, JSValueConst this_val, int argc, JSValueConst* argv);

  // Utility function to execute commands
  static JSValue popen(JSContext* ctx, JSValueConst this_val, int argc, JSValueConst* argv);
};

} // namespace rime

#endif  // RIME_QJS_ENVIRONMENT_H_
