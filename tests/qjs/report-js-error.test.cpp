#include <gtest/gtest.h>
#include <quickjs.h>

#include <string>

#include "engines/engine_manager.h"

std::string trim(const std::string& str) {
  const auto start = str.find_first_not_of(" \t\n\r");
  if (start == std::string::npos) {
    return "";
  }

  const auto end = str.find_last_not_of(" \t\n\r");
  return str.substr(start, end - start + 1);
}

class QuickJSErrorTest : public ::testing::Test {};

TEST_F(QuickJSErrorTest, TestJsRuntimeError) {
  auto jsEngine = newOrShareEngine<JSValue>();
  JSValue module = jsEngine.loadJsFile("runtime-error.js");
  JSValue globalObj = jsEngine.getGlobalObject();
  JSValue func = jsEngine.getObjectProperty(globalObj, "funcWithRuntimeError");
  ASSERT_FALSE(JS_IsException(func));

  JSValue result = jsEngine.callFunction(func, JS_UNDEFINED, 0, nullptr);
  ASSERT_TRUE(JS_IsException(result));

  // ReferenceError: abcdefg is not defined
  //      at <anonymous> (runtime-error.js:7:21)
  JSValue exception = jsEngine.getLatestException();
  auto message = jsEngine.toStdString(exception);
  ASSERT_STREQ(message.c_str(), "ReferenceError: abcdefg is not defined");

  JSValue stack = jsEngine.getObjectProperty(exception, "stack");
  auto stackTrace = trim(jsEngine.toStdString(stack));
  ASSERT_STREQ(stackTrace.c_str(), "at <anonymous> (runtime-error.js:7:19)");

  for (auto obj : {module, globalObj, func, result, exception, stack}) {
    jsEngine.freeValue(obj);
  }
}
