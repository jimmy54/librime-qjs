#include <gtest/gtest.h>
#include <quickjs.h>

#include "engines/quickjs/quickjs_engine.h"

class QuickJSModuleTest : public testing::Test {};

TEST_F(QuickJSModuleTest, ImportJsModuleFromAnotherJsFile) {
  auto& jsEngine = JsEngine<JSValue>::getInstance();
  auto& ctx = jsEngine.getContext();
  JSValue module = QuickJSCodeLoader::loadJsModuleToGlobalThis(ctx, "main.js");

  JSValue globalObj = JS_GetGlobalObject(ctx);
  JSValue myClass = JS_GetPropertyStr(ctx, globalObj, "MyClass");
  ASSERT_FALSE(JS_IsException(myClass));

  constexpr int A_NAMED_INT = 10;
  JSValue arg = JS_NewInt32(ctx, A_NAMED_INT);
  JSValue obj = JS_CallConstructor(ctx, myClass, 1, &arg);
  ASSERT_FALSE(JS_IsException(obj));

  JSValue greetArg = JS_NewString(ctx, "QuickJS");
  JSAtom greetAtom = JS_NewAtom(ctx, "greet");
  JSValue greetResult = JS_Invoke(ctx, obj, greetAtom, 1, &greetArg);
  ASSERT_FALSE(JS_IsException(greetResult));
  JS_FreeAtom(ctx, greetAtom);

  const char* str = JS_ToCString(ctx, greetResult);
  ASSERT_TRUE(str != nullptr);
  EXPECT_STREQ(str, "Hello QuickJS!");
  JS_FreeCString(ctx, str);

  for (auto obj : {module, globalObj, myClass, arg, obj, greetArg, greetResult}) {
    JS_FreeValue(ctx, obj);
  }
}

TEST_F(QuickJSModuleTest, ImportJsModuleToNamespace) {
  auto& jsEngine = JsEngine<JSValue>::getInstance();
  auto& ctx = jsEngine.getContext();
  JSValue moduleNamespace = QuickJSCodeLoader::loadJsModuleToNamespace(ctx, "lib.js");
  ASSERT_FALSE(JS_IsException(moduleNamespace));

  // Get the greet function from the namespace
  JSValue greetFunc = JS_GetPropertyStr(ctx, moduleNamespace, "greet");
  ASSERT_FALSE(JS_IsException(greetFunc));

  JSValue arg = JS_NewString(ctx, "QuickJS");
  JSValue result = JS_Call(ctx, greetFunc, JS_UNDEFINED, 1, &arg);
  ASSERT_FALSE(JS_IsException(result));

  // Verify the result
  const char* str = JS_ToCString(ctx, result);
  ASSERT_TRUE(str != nullptr);
  EXPECT_STREQ(str, "Hello QuickJS!");
  JS_FreeCString(ctx, str);

  for (auto obj : {moduleNamespace, greetFunc, arg, result}) {
    JS_FreeValue(ctx, obj);
  }
}

// NOLINTNEXTLINE(cppcoreguidelines-avoid-non-const-global-variables, readability-function-cognitive-complexity)
TEST_F(QuickJSModuleTest, FindImportedClass) {
  auto& jsEngine = JsEngine<JSValue>::getInstance();
  auto& ctx = jsEngine.getContext();
  JSValue moduleNamespace = QuickJSCodeLoader::loadJsModuleToNamespace(ctx, "lib.js");
  ASSERT_FALSE(JS_IsException(moduleNamespace));

  JSValue myClass =
      QuickJSCodeLoader::getExportedClassByNameInModule(ctx, moduleNamespace, "MyClass");
  ASSERT_FALSE(JS_IsException(myClass));
  ASSERT_FALSE(JS_IsUndefined(myClass));

  JSValue classHavingMethodName =
      QuickJSCodeLoader::getExportedClassHavingMethodNameInModule(ctx, moduleNamespace, "myMethod");
  ASSERT_FALSE(JS_IsException(classHavingMethodName));
  // ASSERT_EQ(myClass.getPtr(), classHavingMethodName.getPtr()); // <-- this is not true, because the class is not the same object.

  for (auto clazz : {myClass, classHavingMethodName}) {
    JSValue proto = JS_GetPropertyStr(ctx, clazz, "prototype");
    ASSERT_FALSE(JS_IsException(proto));

    JSValue constructor = JS_GetPropertyStr(ctx, proto, "constructor");
    ASSERT_FALSE(JS_IsException(constructor));

    constexpr int A_NAMED_INT = 10;
    JSValue arg = JS_NewInt32(ctx, A_NAMED_INT);
    JSValue obj = JS_CallConstructor(ctx, constructor, 1, &arg);
    ASSERT_FALSE(JS_IsException(obj));

    JSValue myMethod = QuickJSCodeLoader::getMethodByNameInClass(ctx, clazz, "myMethod");
    ASSERT_FALSE(JS_IsException(myMethod));
    ASSERT_FALSE(JS_IsUndefined(myMethod));
    JSValue myMethodResult = JS_Call(ctx, myMethod, obj, 0, nullptr);

    int intResult = 0;
    JS_ToInt32(ctx, &intResult, myMethodResult);
    ASSERT_EQ(intResult, A_NAMED_INT + 1);

    for (auto obj : {proto, constructor, arg, obj, myMethod, myMethodResult}) {
      JS_FreeValue(ctx, obj);
    }
  }

  for (auto obj : {
           moduleNamespace,
           myClass,
           classHavingMethodName,
       }) {
    JS_FreeValue(ctx, obj);
  }
}

TEST_F(QuickJSModuleTest, ImportNodeModule) {
  auto& jsEngine = JsEngine<JSValue>::getInstance();
  auto& ctx = jsEngine.getContext();
  JSValue module = QuickJSCodeLoader::loadJsModuleToGlobalThis(ctx, "node-modules.test.js");
  ASSERT_FALSE(JS_IsException(module));
  JS_FreeValue(ctx, module);
}
