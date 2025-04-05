#include <JavaScriptCore/JavaScriptCore.h>
#include <gtest/gtest.h>
#include <sstream>
#include <string>
#include <utility>

// A simple C++ class to expose to JavaScript
class Person {
public:
  Person(std::string name, int age) : name_(std::move(name)), age_(age) {}

  [[nodiscard]] std::string getName() const { return name_; }
  void setName(const std::string& name) { name_ = name; }

  [[nodiscard]] int getAge() const { return age_; }
  void setAge(int age) { age_ = age; }

  [[nodiscard]] std::string greet(const std::string& other) const {
    std::stringstream ss;
    ss << "Hello " << other << ", my name is " << name_ << " and I am " << age_ << " years old.";
    return ss.str();
  }

private:
  std::string name_;
  int age_;
};

// Helper function to convert JSValue to string
static std::string jsValueToString(JSContextRef ctx, JSValueRef value, JSValueRef* exception) {
  JSStringRef str = JSValueToStringCopy(ctx, value, exception);
  if (*exception != nullptr) {
    return "";
  }

  size_t bufferSize = JSStringGetMaximumUTF8CStringSize(str);
  std::vector<char> buffer(bufferSize);
  JSStringGetUTF8CString(str, buffer.data(), bufferSize);
  JSStringRelease(str);
  return {buffer.data()};
}

// NOLINTNEXTLINE(cppcoreguidelines-avoid-non-const-global-variables)
static JSClassRef personClass = nullptr;

// Finalizer for Person objects (called when the JS object is garbage collected)
static void personFinalizer(JSObjectRef object) {
  auto* person = static_cast<Person*>(JSObjectGetPrivate(object));
  delete person;
}

// Property getters and setters
static JSValueRef getPersonName(JSContextRef ctx,
                                JSObjectRef object,
                                JSStringRef propertyName,
                                JSValueRef* exception) {
  auto* person = static_cast<Person*>(JSObjectGetPrivate(object));
  if (person == nullptr) {
    return JSValueMakeUndefined(ctx);
  }

  JSStringRef nameStr = JSStringCreateWithUTF8CString(person->getName().c_str());
  JSValueRef result = JSValueMakeString(ctx, nameStr);
  JSStringRelease(nameStr);
  return result;
}

static bool setPersonName(JSContextRef ctx,
                          JSObjectRef object,
                          JSStringRef propertyName,
                          JSValueRef value,
                          JSValueRef* exception) {
  auto* person = static_cast<Person*>(JSObjectGetPrivate(object));
  if (person == nullptr) {
    return false;
  }

  std::string newName = jsValueToString(ctx, value, exception);
  if (*exception != nullptr) {
    return false;
  }

  person->setName(newName);
  return true;
}

static JSValueRef getPersonAge(JSContextRef ctx,
                               JSObjectRef object,
                               JSStringRef propertyName,
                               JSValueRef* exception) {
  auto* person = static_cast<Person*>(JSObjectGetPrivate(object));
  if (person == nullptr) {
    return JSValueMakeUndefined(ctx);
  }

  return JSValueMakeNumber(ctx, person->getAge());
}

static bool setPersonAge(JSContextRef ctx,
                         JSObjectRef object,
                         JSStringRef propertyName,
                         JSValueRef value,
                         JSValueRef* exception) {
  auto* person = static_cast<Person*>(JSObjectGetPrivate(object));
  if (person == nullptr) {
    return false;
  }

  double age = JSValueToNumber(ctx, value, exception);
  if (*exception != nullptr) {
    return false;
  }

  person->setAge(static_cast<int>(age));
  return true;
}

// Constructor for Person
static JSObjectRef personConstructor(JSContextRef ctx,
                                     JSObjectRef function,
                                     size_t argumentCount,
                                     const JSValueRef arguments[],
                                     JSValueRef* exception) {
  std::string name = "Unknown";
  int age = 0;

  if (argumentCount > 0) {
    name = jsValueToString(ctx, arguments[0], exception);
    if (*exception != nullptr) {
      return nullptr;
    }
  }

  if (argumentCount > 1) {
    age = static_cast<int>(JSValueToNumber(ctx, arguments[1], exception));
    if (*exception != nullptr) {
      return nullptr;
    }
  }

  auto* personObj = new Person(name, age);

  JSObjectRef jsObj = JSObjectMake(ctx, personClass, personObj);
  return jsObj;
}

// Static function for Person.greet
static JSValueRef personGreet(JSContextRef ctx,
                              JSObjectRef function,
                              JSObjectRef thisObject,
                              size_t argumentCount,
                              const JSValueRef arguments[],
                              JSValueRef* exception) {
  auto* person = static_cast<Person*>(JSObjectGetPrivate(thisObject));
  if (person == nullptr) {
    return JSValueMakeUndefined(ctx);
  }

  if (argumentCount < 1) {
    return JSValueMakeUndefined(ctx);
  }

  std::string other = jsValueToString(ctx, arguments[0], exception);
  if (*exception != nullptr) {
    return JSValueMakeUndefined(ctx);
  }

  std::string greeting = person->greet(other);
  JSStringRef greetingStr = JSStringCreateWithUTF8CString(greeting.c_str());
  JSValueRef result = JSValueMakeString(ctx, greetingStr);
  JSStringRelease(greetingStr);
  return result;
}

// Test fixture
class JavaScriptCoreExposeClassTest : public ::testing::Test {
protected:
  void SetUp() override {
    ctx_ = JSGlobalContextCreate(nullptr);

    static JSStaticValue personStaticValues[] = {
        {"name", getPersonName, setPersonName, kJSPropertyAttributeNone},
        {"age", getPersonAge, setPersonAge, kJSPropertyAttributeNone},
        {nullptr, nullptr, nullptr, 0}};

    static JSStaticFunction personStaticFunctions[] = {
        {"greet", personGreet, kJSPropertyAttributeNone}, {nullptr, nullptr, 0}};

    // Create the Person class definition
    JSClassDefinition personClassDef = {
        .version = 0,
        .attributes = kJSClassAttributeNone,
        .className = "Person",
        .parentClass = nullptr,
        .staticValues = static_cast<JSStaticValue*>(personStaticValues),
        .staticFunctions = static_cast<JSStaticFunction*>(personStaticFunctions),
        .initialize = nullptr,
        .finalize = personFinalizer,
        .hasProperty = nullptr,
        .getProperty = nullptr,
        .setProperty = nullptr,
        .deleteProperty = nullptr,
        .getPropertyNames = nullptr,
        .callAsFunction = nullptr,
        .callAsConstructor = personConstructor,
        .hasInstance = nullptr,
        .convertToType = nullptr};

    // Create and register the Person class
    personClass = JSClassCreate(&personClassDef);

    // Add the constructor to the global object
    JSObjectRef globalObj = JSContextGetGlobalObject(ctx_);
    JSObjectRef constructorObj = JSObjectMake(ctx_, personClass, nullptr);
    JSStringRef classNameStr = JSStringCreateWithUTF8CString("Person");
    JSObjectSetProperty(ctx_, globalObj, classNameStr, constructorObj, kJSPropertyAttributeNone,
                        nullptr);
    JSStringRelease(classNameStr);
  }

  JSContextRef& getContext() { return ctx_; }

private:
  JSContextRef ctx_;
};

// Helper function to get array item as string
static std::string getArrayItemAsString(JSContextRef ctx,
                                        JSObjectRef array,
                                        unsigned index,
                                        JSValueRef* exception) {
  JSValueRef value = JSObjectGetPropertyAtIndex(ctx, array, index, exception);
  if (*exception != nullptr) {
    return "";
  }
  return jsValueToString(ctx, value, exception);
}

TEST_F(JavaScriptCoreExposeClassTest, TestExposeClassToJS) {
  const char* script = R"(
    function testPersonClass() {
      var person = new Person("Alice", 30);
      var nameResult = person.name;
      person.name = "Bob";
      var newNameResult = person.name;
      var greeting = person.greet("Charlie");
      return [nameResult, newNameResult, greeting];
    }
    testPersonClass();
  )";

  auto& ctx = getContext();
  JSValueRef exception = nullptr;
  auto* scriptStr = JSStringCreateWithUTF8CString(script);
  JSValueRef result = JSEvaluateScript(ctx, scriptStr, nullptr, nullptr, 0, &exception);
  JSStringRelease(scriptStr);

  ASSERT_FALSE(exception) << "JavaScript execution failed";
  ASSERT_TRUE(JSValueIsObject(ctx, result)) << "Result should be an array";

  JSObjectRef resultArray = JSValueToObject(ctx, result, &exception);
  ASSERT_FALSE(exception);

  std::string originalName = getArrayItemAsString(ctx, resultArray, 0, &exception);
  std::string modifiedName = getArrayItemAsString(ctx, resultArray, 1, &exception);
  std::string greeting = getArrayItemAsString(ctx, resultArray, 2, &exception);

  EXPECT_EQ(originalName, "Alice");
  EXPECT_EQ(modifiedName, "Bob");
  EXPECT_EQ(greeting, "Hello Charlie, my name is Bob and I am 30 years old.");
}
