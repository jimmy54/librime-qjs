#include <gtest/gtest.h>
#include <rime/candidate.h>
#include <rime/config/config_component.h>
#include <rime/context.h>
#include <rime/engine.h>
#include <rime/schema.h>

#include <quickjs.h>
#include "engines/engine_manager.h"
#include "test_switch.h"
#include "types/environment.h"
#include "types/qjs_types.h"

using namespace rime;

template <typename T>
class QuickJSNotifierTest : public ::testing::Test {};

SETUP_JS_ENGINES(QuickJSNotifierTest);

// NOLINTNEXTLINE(readability-function-cognitive-complexity)
TYPED_TEST(QuickJSNotifierTest, ConnectToRimeNotifier) {
  const char* script = R"(
    let connection = null;
    let notifiedTimes = 0;
    function connectToNotifier(env) {
      connection = env.engine.context.commitNotifier.connect((ctx) => {
        ++notifiedTimes;
        ctx.commitHistory.push('js', 'text' + notifiedTimes);
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

  auto jsEngine = newOrShareEngine<TypeParam>();
  registerTypesToJsEngine(jsEngine);

  auto result = jsEngine.eval(script, "<input>");

  the<Engine> engine(Engine::Create());
  engine->context()->set_input("hello");
  auto environment = std::make_unique<Environment>(engine.get(), "namespace");
  TypeParam env = jsEngine.wrap(environment.get());

  auto global = jsEngine.toObject(jsEngine.getGlobalObject());
  auto isConnectedFunc = jsEngine.toObject(jsEngine.getObjectProperty(global, "isConnected"));
  auto getNotifiedTimesFunc =
      jsEngine.toObject(jsEngine.getObjectProperty(global, "getNotifiedTimes"));
  auto undefined = jsEngine.toObject(jsEngine.undefined());

  {
    auto isConnected = jsEngine.callFunction(isConnectedFunc, global, 0, nullptr);
    ASSERT_FALSE(jsEngine.toBool(isConnected));
    jsEngine.freeValue(isConnected);
  }

  {
    auto connectToNotifierFunc =
        jsEngine.toObject(jsEngine.getObjectProperty(global, "connectToNotifier"));
    auto connectToNotifier = jsEngine.callFunction(connectToNotifierFunc, undefined, 1, &env);
    auto isConnected = jsEngine.callFunction(isConnectedFunc, undefined, 0, nullptr);
    ASSERT_TRUE(jsEngine.toBool(isConnected));
    auto notifiedTimes = jsEngine.callFunction(getNotifiedTimesFunc, undefined, 0, nullptr);
    ASSERT_EQ(jsEngine.toInt(notifiedTimes), 0);
    jsEngine.freeValue(notifiedTimes, isConnected, connectToNotifier, connectToNotifierFunc);
  }
  {
    engine->context()->set_input("notify");
    engine->context()->Commit();
    auto isConnected = jsEngine.callFunction(isConnectedFunc, undefined, 0, nullptr);
    ASSERT_TRUE(jsEngine.toBool(isConnected));
    auto notifiedTimes = jsEngine.callFunction(getNotifiedTimesFunc, undefined, 0, nullptr);
    ASSERT_EQ(jsEngine.toInt(notifiedTimes), 1);
    LOG(INFO) << "commit_history: " << engine->context()->commit_history().repr();
    ASSERT_TRUE(engine->context()->commit_history().repr().find("[js]text1") != std::string::npos);
    jsEngine.freeValue(notifiedTimes, isConnected);
  }
  {
    engine->context()->set_input("notify again");
    engine->context()->Commit();
    auto isConnected = jsEngine.callFunction(isConnectedFunc, global, 0, nullptr);
    ASSERT_TRUE(jsEngine.toBool(isConnected));
    auto notifiedTimes = jsEngine.callFunction(getNotifiedTimesFunc, global, 0, nullptr);
    ASSERT_EQ(jsEngine.toInt(notifiedTimes), 2);
    ASSERT_TRUE(engine->context()->commit_history().repr().find("[js]text2") != std::string::npos);
    jsEngine.freeValue(notifiedTimes, isConnected);
  }
  {
    auto disconnectFromNotifierFunc =
        jsEngine.toObject(jsEngine.getObjectProperty(global, "disconnectFromNotifier"));
    auto disconnectFromNotifier =
        jsEngine.callFunction(disconnectFromNotifierFunc, global, 0, nullptr);
    auto isConnected = jsEngine.callFunction(isConnectedFunc, global, 0, nullptr);
    ASSERT_FALSE(jsEngine.toBool(isConnected));
    jsEngine.freeValue(disconnectFromNotifierFunc, disconnectFromNotifier, isConnected);
  }
  {
    engine->context()->set_input("should not notify");
    engine->context()->Commit();
    auto isConnected = jsEngine.callFunction(isConnectedFunc, global, 0, nullptr);
    ASSERT_FALSE(jsEngine.toBool(isConnected));
    auto notifiedTimes = jsEngine.callFunction(getNotifiedTimesFunc, global, 0, nullptr);
    ASSERT_EQ(jsEngine.toInt(notifiedTimes), 2);  // unchanged after disconnection
    ASSERT_TRUE(engine->context()->commit_history().repr().find("[js]text3") == std::string::npos);
    jsEngine.freeValue(notifiedTimes, isConnected);
  }

  jsEngine.freeValue(getNotifiedTimesFunc, isConnectedFunc, global, env, result);
}
