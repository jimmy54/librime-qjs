#ifndef RIME_QJS_ENVIRONMENT_H_
#define RIME_QJS_ENVIRONMENT_H_

#include <rime/engine.h>

#include <string>

#include "quickjs.h"

namespace rime {

class QjsEnvironment {
public:
  // Create and initialize a JavaScript environment object
  static JSValue create(JSContext* ctx, Engine* engine, const std::string& nameSpace);

private:
  // Utility function to load file content
  static JSValue loadFile(JSContext* ctx, JSValueConst thisVal, int argc, JSValueConst* argv);

  // Utility function to check if file exists
  static JSValue fileExists(JSContext* ctx, JSValueConst thisVal, int argc, JSValueConst* argv);

  // Utility function to get the Rime infomation
  static JSValue getRimeInfo(JSContext* ctx, JSValueConst thisVal, int argc, JSValueConst* argv);

  // Utility function to execute commands
  static JSValue popen(JSContext* ctx, JSValueConst thisVal, int argc, JSValueConst* argv);
};

}  // namespace rime

#endif  // RIME_QJS_ENVIRONMENT_H_
