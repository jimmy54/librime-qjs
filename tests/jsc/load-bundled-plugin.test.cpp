#include <JavaScriptCore/JavaScriptCore.h>
#include <gtest/gtest.h>
#include <filesystem>
#include <iostream>
#include <string>

#include "engines/javascriptcore/javascriptcore_engine.h"
#include "types/js_wrapper.h"
#include "types/qjs_candidate.h"

class JscLoadBundledPluginTest : public testing::Test {
protected:
  void SetUp() override {
    std::filesystem::path path = __FILE__;
    path = path.parent_path().parent_path() / "js";

    auto& jsEngine = JsEngine<JSValueRef>::getInstance();
    jsEngine.setBaseFolderPath(path.generic_string().c_str());
  }

  static JSValueRef getRimeInfoCallback(JSContextRef ctx,
                                        JSObjectRef function,
                                        JSObjectRef thisObject,
                                        size_t argumentCount,
                                        const JSValueRef arguments[],
                                        JSValueRef* exception) {
    auto& engine = JsEngine<JSValueRef>::getInstance();
    std::cout << "getRimeInfoCallback" << '\n';
    return engine.toJsString("rimeInfo");
  }

  static JSObjectRef createMockEnvObject() {
    auto& engine = JsEngine<JSValueRef>::getInstance();
    JSObjectRef envObj = engine.newObject();
    JSObjectRef osObj = engine.newObject();
    engine.setObjectProperty(osObj, "name", engine.toJsString("macOS"));
    engine.setObjectProperty(envObj, "os", osObj);
    engine.setObjectFunction(envObj, "getRimeInfo", getRimeInfoCallback, 0);
    return envObj;
  }

  static JSObjectRef createMockSegmentObject() {
    auto& engine = JsEngine<JSValueRef>::getInstance();
    JSObjectRef segmentObj = engine.newObject();
    engine.setObjectProperty(segmentObj, "start", engine.toJsInt(0));
    engine.setObjectProperty(segmentObj, "end", engine.toJsInt(std::string("/help").size()));
    engine.setObjectProperty(segmentObj, "prompt", engine.toJsString(""));
    return segmentObj;
  }
};

// NOLINTNEXTLINE(readability-function-cognitive-complexity)
TEST_F(JscLoadBundledPluginTest, RunTranslatorWithJavaScriptCore) {
  auto& engine = JsEngine<JSValueRef>::getInstance();
  JsWrapper<rime::Candidate, JSValueRef> wrapper;
  engine.registerType(wrapper);

  const auto* container = engine.loadJsFile("help_menu.dist.js");
  EXPECT_TRUE(container != nullptr) << "JavaScript evaluation failed";

  const auto* clazz = engine.getJsClassHavingMethod(container, "translate");
  EXPECT_TRUE(clazz != nullptr);

  JSValueRef arg = createMockEnvObject();
  JSObjectRef instance = engine.callConstructor(engine.toObject(clazz), 1, &arg);
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
