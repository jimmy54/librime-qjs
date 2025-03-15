#include <gtest/gtest.h>
#include <quickjs.h>

#include <string>

#include "qjs_helper.h"
#include "test_helper.h"

std::string trim(const std::string& str) {
  const auto start = str.find_first_not_of(" \t\n\r");
  if (start == std::string::npos) {
    return "";
  }

  const auto end = str.find_last_not_of(" \t\n\r");
  return str.substr(start, end - start + 1);
}

class QuickJSErrorTest : public ::testing::Test {
protected:
  void SetUp() override { setJsBasePath(__FILE__, "/js"); }
};

// NOLINTNEXTLINE(cppcoreguidelines-avoid-non-const-global-variables,
// readability-function-cognitive-complexity)
TEST_F(QuickJSErrorTest, TestJsRuntimeError) {
  JSContext* ctx = QjsHelper::getInstance().getContext();
  JSValue module = QjsHelper::loadJsModuleToGlobalThis(ctx, "runtime-error.js");
  JSValue globalObj = JS_GetGlobalObject(ctx);
  JSValue func = JS_GetPropertyStr(ctx, globalObj, "funcWithRuntimeError");
  ASSERT_FALSE(JS_IsException(func));

  JSValue result = JS_Call(ctx, func, JS_UNDEFINED, 0, nullptr);
  ASSERT_TRUE(JS_IsException(result));

  // ReferenceError: abcdefg is not defined
  //      at <anonymous> (runtime-error.js:7:21)
  JSValue exception = JS_GetException(ctx);
  const char* message = JS_ToCString(ctx, exception);
  ASSERT_STREQ(message, "ReferenceError: abcdefg is not defined");
  JS_FreeCString(ctx, message);

  JSValue stack = JS_GetPropertyStr(ctx, exception, "stack");
  const char* stackTrace = JS_ToCString(ctx, stack);
  std::string trimmedStackTrace = trim(stackTrace);
  ASSERT_STREQ(trimmedStackTrace.c_str(), "at <anonymous> (runtime-error.js:7:19)");
  JS_FreeCString(ctx, stackTrace);
  JS_FreeValue(ctx, stack);

  JS_FreeValue(ctx, exception);
  JS_FreeValue(ctx, result);

  JS_FreeValue(ctx, func);
  JS_FreeValue(ctx, globalObj);
  JS_FreeValue(ctx, module);
}
