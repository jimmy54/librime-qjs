#include <JavaScriptCore/JavaScript.h>
#include <JavaScriptCore/JavaScriptCore.h>
#include <gtest/gtest.h>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

class Candidate {
public:
  Candidate() = default;

  static JSObjectRef create(JSContextRef ctx,
                            JSObjectRef constructor,
                            size_t argumentCount,
                            const JSValueRef arguments[],
                            JSValueRef* exception) {
    std::cout << "Candidate::create" << '\n';
    JSClassRef candidateClass = JSClassCreate(Candidate::classDefinition());
    JSObjectRef obj = JSObjectMake(ctx, candidateClass, new Candidate());
    JSClassRelease(candidateClass);
    return obj;
  }

  static void finalize(JSObjectRef object) {
    std::cout << "Candidate::finalize" << '\n';
    delete static_cast<Candidate*>(JSObjectGetPrivate(object));
  }

  static JSClassDefinition* classDefinition() {
    static JSStaticValue staticValues[] = {{nullptr, nullptr, nullptr, 0}};

    static JSStaticFunction staticFunctions[] = {{nullptr, nullptr, 0}};

    static JSClassDefinition definition = {0,
                                           kJSClassAttributeNone,
                                           "Candidate",
                                           nullptr,
                                           static_cast<JSStaticValue*>(staticValues),
                                           static_cast<JSStaticFunction*>(staticFunctions),
                                           nullptr,
                                           finalize,
                                           nullptr,
                                           nullptr,
                                           nullptr,
                                           nullptr,
                                           nullptr,
                                           nullptr,
                                           nullptr,
                                           nullptr,
                                           nullptr};
    return &definition;
  }

  static void registerClass(JSContextRef ctx) {
    JSClassRef candidateClass = JSClassCreate(Candidate::classDefinition());
    JSObjectRef candidateConstructor =
        JSObjectMakeConstructor(ctx, candidateClass, Candidate::create);
    JSObjectSetProperty(ctx, JSContextGetGlobalObject(ctx),
                        JSStringCreateWithUTF8CString("Candidate"), candidateConstructor,
                        kJSPropertyAttributeNone, nullptr);
  }
};

class JscLoadBundledPluginTest : public testing::Test {
protected:
  static JSValueRef consoleLogCallback(JSContextRef ctx,
                                       JSObjectRef function,
                                       JSObjectRef thisObject,
                                       size_t argumentCount,
                                       const JSValueRef arguments[],
                                       JSValueRef* exception) {
    for (size_t i = 0; i < argumentCount; ++i) {
      JSStringRef str = JSValueToStringCopy(ctx, arguments[i], exception);
      if (str != nullptr) {
        size_t bufSize = JSStringGetMaximumUTF8CStringSize(str);
        std::vector<char> buf(bufSize);
        JSStringGetUTF8CString(str, buf.data(), bufSize);
        std::cout << buf.data() << " ";
        JSStringRelease(str);
      }
    }
    std::cout << '\n';
    return JSValueMakeUndefined(ctx);
  }

  static JSObjectRef setupConsoleLog(JSContextRef ctx) {
    JSObjectRef consoleObj = JSObjectMake(ctx, nullptr, nullptr);
    JSObjectSetProperty(ctx, JSContextGetGlobalObject(ctx),
                        JSStringCreateWithUTF8CString("console"), consoleObj,
                        kJSPropertyAttributeNone, nullptr);

    JSObjectRef logFunction = JSObjectMakeFunctionWithCallback(
        ctx, JSStringCreateWithUTF8CString("log"), consoleLogCallback);

    JSObjectSetProperty(ctx, consoleObj, JSStringCreateWithUTF8CString("log"), logFunction,
                        kJSPropertyAttributeNone, nullptr);
    return consoleObj;
  }
  // Helper function to read file content
  static std::string readFile(const std::string& path) {
    std::filesystem::path filePath = std::filesystem::path("../tests/js/dist") / path;
    std::ifstream file(filePath);
    if (!file.is_open()) {
      throw std::runtime_error("Failed to open file: " + filePath.string());
    }

    std::stringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
  }

  // Helper function to check if a JavaScript object has a method with the given name
  static bool isClassHavingMethod(JSContextRef ctx, JSValueRef value, const char* methodName) {
    JSObjectRef obj = nullptr;
    if (JSValueIsObject(ctx, value)) {
      obj = JSValueToObject(ctx, value, nullptr);
    } else {
      return false;
    }
    // Check if it has a prototype property (which confirms it's a constructor function/class)
    JSValueRef prototype =
        JSObjectGetProperty(ctx, obj, JSStringCreateWithUTF8CString("prototype"), nullptr);
    if (JSValueIsUndefined(ctx, prototype) || !JSValueIsObject(ctx, prototype)) {
      return false;
    }

    // Check if the prototype has the method
    JSObjectRef prototypeObj = JSValueToObject(ctx, prototype, nullptr);
    JSValueRef translateMethod =
        JSObjectGetProperty(ctx, prototypeObj, JSStringCreateWithUTF8CString(methodName), nullptr);
    return !JSValueIsUndefined(ctx, translateMethod) && JSValueIsObject(ctx, translateMethod);
  }

  static JSObjectRef getClassWithMethod(JSContextRef ctx, const char* methodName) {
    JSObjectRef globalObj = JSContextGetGlobalObject(ctx);
    JSPropertyNameArrayRef propertyNames = JSObjectCopyPropertyNames(ctx, globalObj);
    size_t count = JSPropertyNameArrayGetCount(propertyNames);
    for (size_t i = 0; i < count; i++) {
      JSStringRef propertyName = JSPropertyNameArrayGetNameAtIndex(propertyNames, i);
      JSValueRef value = JSObjectGetProperty(ctx, globalObj, propertyName, nullptr);
      if (isClassHavingMethod(ctx, value, methodName)) {
        JSPropertyNameArrayRelease(propertyNames);
        return JSValueToObject(ctx, value, nullptr);
      }
    }
    JSPropertyNameArrayRelease(propertyNames);
    return nullptr;
  }

  static void removeExportStatementsInPlace(std::string& source) {
    size_t pos = 0;
    while ((pos = source.find("export", pos)) != std::string::npos) {
      size_t rightBracket = source.find('}', pos);
      if (rightBracket == std::string::npos) {
        rightBracket = source.length();
      } else {
        rightBracket++;
      }
      source.replace(pos, rightBracket - pos, "");
      pos = rightBracket;
    }
  }

  static void logJsError(JSContextRef ctx, JSValueRef exception) {
    JSStringRef exceptionStr = JSValueToStringCopy(ctx, exception, nullptr);
    size_t bufferSize = JSStringGetMaximumUTF8CStringSize(exceptionStr);
    std::vector<char> buffer(bufferSize);
    JSStringGetUTF8CString(exceptionStr, buffer.data(), bufferSize);
    std::cerr << "JavaScript exception: " << buffer.data() << '\n';
    JSStringRelease(exceptionStr);
  }

  static JSObjectRef createMockEnvObject(JSContextRef ctx) {
    JSObjectRef envObj = JSObjectMake(ctx, nullptr, nullptr);
    JSObjectRef osObj = JSObjectMake(ctx, nullptr, nullptr);
    JSStringRef osName = JSStringCreateWithUTF8CString("macOS");
    JSObjectSetProperty(ctx, osObj, JSStringCreateWithUTF8CString("name"),
                        JSValueMakeString(ctx, osName), kJSPropertyAttributeNone, nullptr);
    JSObjectSetProperty(ctx, envObj, JSStringCreateWithUTF8CString("os"), osObj,
                        kJSPropertyAttributeNone, nullptr);
    JSStringRelease(osName);

    // Add getRimeInfo method
    JSObjectRef getRimeInfoFunc = JSObjectMakeFunctionWithCallback(
        ctx, JSStringCreateWithUTF8CString("getRimeInfo"),
        [](JSContextRef ctx, JSObjectRef function, JSObjectRef thisObject, size_t argumentCount,
           const JSValueRef arguments[], JSValueRef* exception) {
          JSStringRef rimeInfo = JSStringCreateWithUTF8CString("rimeInfo");
          JSValueRef infoObj = JSValueMakeString(ctx, rimeInfo);
          return infoObj;
        });
    JSObjectSetProperty(ctx, envObj, JSStringCreateWithUTF8CString("getRimeInfo"), getRimeInfoFunc,
                        kJSPropertyAttributeNone, nullptr);

    return envObj;
  }

  static JSObjectRef createMockSegmentObject(JSContextRef ctx) {
    JSObjectRef segmentObj = JSObjectMake(ctx, nullptr, nullptr);

    // Set basic segment properties
    JSObjectSetProperty(ctx, segmentObj, JSStringCreateWithUTF8CString("start"),
                        JSValueMakeNumber(ctx, 0), kJSPropertyAttributeNone, nullptr);
    JSObjectSetProperty(ctx, segmentObj, JSStringCreateWithUTF8CString("end"),
                        JSValueMakeNumber(ctx, static_cast<double>(std::string("/help").size())),
                        kJSPropertyAttributeNone, nullptr);

    JSStringRef prompt = JSStringCreateWithUTF8CString("");
    JSObjectSetProperty(ctx, segmentObj, JSStringCreateWithUTF8CString("prompt"),
                        JSValueMakeString(ctx, prompt), kJSPropertyAttributeNone, nullptr);
    JSStringRelease(prompt);
    return segmentObj;
  }
};

TEST_F(JscLoadBundledPluginTest, FindClassWithTranslateMethod) {
  auto* ctx = JSGlobalContextCreate(nullptr);

  setupConsoleLog(ctx);

  // Expose Candidate class to JS
  Candidate::registerClass(ctx);
  std::string source = readFile("help_menu.dist.js");
  removeExportStatementsInPlace(source);

  JSStringRef scriptJS = JSStringCreateWithUTF8CString(source.c_str());
  JSValueRef exception = nullptr;
  JSEvaluateScript(ctx, scriptJS, nullptr, nullptr, 0, &exception);
  JSStringRelease(scriptJS);

  if (exception != nullptr) {
    logJsError(ctx, exception);
    FAIL() << "JavaScript evaluation failed with exception";
  }

  auto* classWithTranslateMethod = getClassWithMethod(ctx, "translate");
  EXPECT_TRUE(classWithTranslateMethod != nullptr);

  JSValueRef args[] = {createMockEnvObject(ctx)};
  JSObjectRef instance = JSObjectCallAsConstructor(ctx, classWithTranslateMethod, 1,
                                                   static_cast<JSValueRef*>(args), &exception);
  if (exception != nullptr) {
    logJsError(ctx, exception);
    FAIL() << "Failed to create instance of class with translate method";
  }
  EXPECT_TRUE(instance != nullptr);

  // Verify the instance has the translate method
  JSValueRef translateMethod =
      JSObjectGetProperty(ctx, instance, JSStringCreateWithUTF8CString("translate"), nullptr);
  EXPECT_TRUE(!JSValueIsUndefined(ctx, translateMethod) && JSValueIsObject(ctx, translateMethod));

  // Execute the translate method
  JSStringRef input = JSStringCreateWithUTF8CString("/help");
  JSObjectRef segmentObj = createMockSegmentObject(ctx);
  JSValueRef translateArgs[] = {JSValueMakeString(ctx, input), segmentObj,
                                createMockEnvObject(ctx)};
  auto* ptrTranslateArgs = static_cast<JSValueRef*>(translateArgs);
  JSValueRef translateResult = JSObjectCallAsFunction(
      ctx, JSValueToObject(ctx, translateMethod, nullptr), instance,
      sizeof(translateArgs) / sizeof(translateArgs[0]), ptrTranslateArgs, &exception);
  JSStringRelease(input);
  if (exception != nullptr) {
    logJsError(ctx, exception);
    FAIL() << "Failed to execute translate method";
  }
  EXPECT_TRUE(!JSValueIsUndefined(ctx, translateResult));

  // Count items in translateResult
  size_t itemCount = 0;
  if (JSValueIsArray(ctx, translateResult)) {
    JSObjectRef arrayObj = JSValueToObject(ctx, translateResult, nullptr);
    JSValueRef lengthValue =
        JSObjectGetProperty(ctx, arrayObj, JSStringCreateWithUTF8CString("length"), nullptr);
    itemCount = static_cast<size_t>(JSValueToNumber(ctx, lengthValue, nullptr));
  }
  EXPECT_EQ(itemCount, 14) << "translateResult should contain items";

  JSGlobalContextRelease(ctx);
}
