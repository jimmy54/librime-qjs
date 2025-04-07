#include <gtest/gtest.h>
#include <rime/candidate.h>
#include <rime/config/config_component.h>
#include <rime/context.h>
#include <rime/engine.h>
#include <rime/schema.h>

#include <quickjs.h>
#include "jsvalue_raii.hpp"
#include "qjs_engine.h"
#include "qjs_environment.h"

using namespace rime;

class QuickJSNotifierTest : public ::testing::Test {};

static int jsToInt(JSContext* ctx, JSValueConst val) {
  int ret = 0;
  if (JS_ToInt32(ctx, &ret, val) == 0) {
    return ret;
  }
  throw std::runtime_error("cannot convert js value to int");
}

// NOLINTNEXTLINE(readability-function-cognitive-complexity)
TEST_F(QuickJSNotifierTest, ConnectToRimeNotifier) {
  const char* script = R"(
    let connection = null;
    let notifiedTimes = 0;
    function connectToNotifier(env) {
      connection = env.engine.context.commitNotifier.connect((ctx) => {
        ++notifiedTimes;
      })
    }
    function disconnectFromNotifier() {
      connection.disconnect();
    }
    function isConnected() {
      return connection?.isConnected || false;
    }
    function getNotifiedTimes() {
      return notifiedTimes;
    }
  )";

  auto* ctx = QjsHelper::getInstance().getContext();
  JSValueRAII result = JS_Eval(ctx, script, strlen(script), "<input>", JS_EVAL_TYPE_GLOBAL);

  the<Engine> engine(Engine::Create());
  engine->context()->set_input("hello");
  JSValueRAII env = QjsEnvironment::create(ctx, engine.get(), "namespace");
  JS_SetPropertyStr(ctx, env, "engine", QjsEngine::wrap(ctx, engine.get()));

  JSValueRAII global = JS_GetGlobalObject(ctx);
  JSValueRAII isConnectedFunc = JS_GetPropertyStr(ctx, global, "isConnected");
  JSValueRAII getNotifiedTimesFunc = JS_GetPropertyStr(ctx, global, "getNotifiedTimes");

  {
    JSValueRAII isConnected = JS_Call(ctx, isConnectedFunc, JS_UNDEFINED, 0, nullptr);
    ASSERT_FALSE(JS_ToBool(ctx, isConnected));
  }

  {
    JSValueRAII connectToNotifierFunc = JS_GetPropertyStr(ctx, global, "connectToNotifier");
    JSValueRAII connectToNotifier =
        JS_Call(ctx, connectToNotifierFunc, JS_UNDEFINED, 1, env.getPtr());
    JSValueRAII isConnected = JS_Call(ctx, isConnectedFunc, JS_UNDEFINED, 0, nullptr);
    ASSERT_TRUE(JS_ToBool(ctx, isConnected));
    JSValueRAII notifiedTimes = JS_Call(ctx, getNotifiedTimesFunc, JS_UNDEFINED, 0, nullptr);
    ASSERT_EQ(jsToInt(ctx, notifiedTimes), 0);
  }
  {
    engine->context()->set_input("notify");
    engine->context()->Commit();
    JSValueRAII isConnected = JS_Call(ctx, isConnectedFunc, JS_UNDEFINED, 0, nullptr);
    ASSERT_TRUE(JS_ToBool(ctx, isConnected));
    JSValueRAII notifiedTimes = JS_Call(ctx, getNotifiedTimesFunc, JS_UNDEFINED, 0, nullptr);
    ASSERT_EQ(jsToInt(ctx, notifiedTimes), 1);
  }
  {
    engine->context()->set_input("notify again");
    engine->context()->Commit();
    JSValueRAII isConnected = JS_Call(ctx, isConnectedFunc, JS_UNDEFINED, 0, nullptr);
    ASSERT_TRUE(JS_ToBool(ctx, isConnected));
    JSValueRAII notifiedTimes = JS_Call(ctx, getNotifiedTimesFunc, JS_UNDEFINED, 0, nullptr);
    ASSERT_EQ(jsToInt(ctx, notifiedTimes), 2);
  }
  {
    JSValueRAII disconnectFromNotifierFunc =
        JS_GetPropertyStr(ctx, global, "disconnectFromNotifier");
    JSValueRAII disconnectFromNotifier =
        JS_Call(ctx, disconnectFromNotifierFunc, JS_UNDEFINED, 0, nullptr);
    JSValueRAII isConnected = JS_Call(ctx, isConnectedFunc, JS_UNDEFINED, 0, nullptr);
    ASSERT_FALSE(JS_ToBool(ctx, isConnected));
  }
  {
    engine->context()->set_input("should not notify");
    engine->context()->Commit();
    JSValueRAII isConnected = JS_Call(ctx, isConnectedFunc, JS_UNDEFINED, 0, nullptr);
    ASSERT_FALSE(JS_ToBool(ctx, isConnected));
    JSValueRAII notifiedTimes = JS_Call(ctx, getNotifiedTimesFunc, JS_UNDEFINED, 0, nullptr);
    ASSERT_EQ(jsToInt(ctx, notifiedTimes), 2);  // unchanged after disconnection
  }
}
