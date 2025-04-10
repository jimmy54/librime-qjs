#include <JavaScriptCore/JavaScriptCore.h>
#include <gtest/gtest.h>
#include <filesystem>
#include <iostream>
#include <string>

#include "engines/engine_manager.h"
#include "types/js_wrapper.h"
#include "types/qjs_candidate.h"

class JscLoadBundledPluginTest : public testing::Test {
protected:
  void SetUp() override {
    std::filesystem::path path = __FILE__;
    path = path.parent_path().parent_path() / "js";

    engine.setBaseFolderPath(path.generic_string().c_str());
  }

  static JSValueRef getRimeInfoCallback(JSContextRef ctx,
                                        JSObjectRef function,
                                        JSObjectRef thisObject,
                                        size_t argumentCount,
                                        const JSValueRef arguments[],
                                        JSValueRef* exception) {
    std::cout << "getRimeInfoCallback" << '\n';
    return engine.toJsString("rimeInfo");
  }

  static JSObjectRef createMockEnvObject() {
    JSObjectRef envObj = engine.newObject();
    JSObjectRef osObj = engine.newObject();
    engine.setObjectProperty(osObj, "name", engine.toJsString("macOS"));
    engine.setObjectProperty(envObj, "os", osObj);
    engine.setObjectFunction(envObj, "getRimeInfo", getRimeInfoCallback, 0);
    return envObj;
  }

  static JSObjectRef createMockSegmentObject() {
    JSObjectRef segmentObj = engine.newObject();
    engine.setObjectProperty(segmentObj, "start", engine.toJsInt(0));
    engine.setObjectProperty(segmentObj, "end", engine.toJsInt(std::string("/help").size()));
    engine.setObjectProperty(segmentObj, "prompt", engine.toJsString(""));
    return segmentObj;
  }

  static JsEngine<JSValueRef> engine;
};

JsEngine<JSValueRef> JscLoadBundledPluginTest::engine = newOrShareEngine<JSValueRef>();

// NOLINTNEXTLINE(readability-function-cognitive-complexity)
TEST_F(JscLoadBundledPluginTest, RunTranslatorWithJavaScriptCore) {
  engine.registerType<rime::Candidate>();

  const auto* container = engine.loadJsFile("help_menu.dist.js");
  EXPECT_TRUE(container != nullptr) << "JavaScript evaluation failed";

  const auto* clazz = engine.getJsClassHavingMethod(container, "translate");
  EXPECT_TRUE(clazz != nullptr);

  JSValueRef arg = createMockEnvObject();
  JSObjectRef instance = engine.newClassInstance(engine.toObject(clazz), 1, &arg);
  EXPECT_TRUE(instance != nullptr) << "Failed to create instance of class with translate method";

  // Verify the instance has the translate method
  JSValueRef translateMethod = engine.getObjectProperty(instance, "translate");
  EXPECT_TRUE(engine.isObject(translateMethod));
  EXPECT_FALSE(engine.isUndefined(translateMethod));

  // Execute the translate method
  JSValueRef input = engine.toJsString("/help");
  JSValueRef args[] = {input, createMockSegmentObject(), createMockEnvObject()};
  JSValueRef translateResult = engine.callFunction(engine.toObject(translateMethod), instance,
                                                   countof(args), static_cast<JSValueRef*>(args));
  EXPECT_FALSE(engine.isUndefined(translateResult));

  // Count items in translateResult
  size_t itemCount = engine.getArrayLength(translateResult);
  EXPECT_EQ(itemCount, 14) << "translateResult should contain items";
}
