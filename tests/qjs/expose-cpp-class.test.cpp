#include <string>
#include <sstream>
#include <gtest/gtest.h>
#include "quickjs.h"
#include "qjs_helper.h"

class MyClass {
public:
    MyClass(const std::string& name) : name_(name) {}

    std::string sayHello() const {
        std::stringstream ss;
        ss << "Hello, " << name_ << "!";
        return ss.str();
    }

    std::string getName() const {
        return name_;
    }

    void setName(const std::string& name) {
        name_ = name;
    }

private:
    std::string name_;
};

static JSClassID js_myclass_class_id;

// C++ Class Instance Finalizer (called when the object is GC'd)
static void js_myclass_finalizer(JSRuntime* rt, JSValue val) {
    MyClass* instance = static_cast<MyClass*>(JS_GetOpaque(val, js_myclass_class_id));
    if (instance) {  // Add null check
        delete instance;
    }
}

// Constructor for MyClass
static JSValue js_myclass_constructor(JSContext* ctx, JSValueConst new_target, int argc, JSValueConst* argv) {
    const char* name = nullptr;

    if (argc > 0 && JS_IsString(argv[0])) {
        name = JS_ToCString(ctx, argv[0]);
    }

    if (!name) {
        return JS_ThrowTypeError(ctx, "Expected a string as the first argument");
    }

    MyClass* instance = new MyClass(name);
    JSValue obj = JS_NewObjectClass(ctx, js_myclass_class_id);  // Use the class ID here
    if (JS_IsException(obj)) {
        delete instance;
        return obj;
    }

    JS_SetOpaque(obj, instance);
    JS_FreeCString(ctx, name);
    return obj;
}

// Method: sayHello
static JSValue js_myclass_say_hello(JSContext* ctx, JSValueConst this_val, int argc, JSValueConst* argv) {
    MyClass* instance = static_cast<MyClass*>(JS_GetOpaque(this_val, js_myclass_class_id));
    if (!instance) {
        return JS_ThrowTypeError(ctx, "Invalid MyClass instance");
    }

    return JS_NewString(ctx, instance->sayHello().c_str());
}

// Method: getName
static JSValue js_myclass_get_name(JSContext* ctx, JSValueConst this_val, int argc, JSValueConst* argv) {
    MyClass* instance = static_cast<MyClass*>(JS_GetOpaque(this_val, js_myclass_class_id));
    if (!instance) {
        return JS_ThrowTypeError(ctx, "Invalid MyClass instance");
    }

    return JS_NewString(ctx, instance->getName().c_str());
}

// Method: setName
static JSValue js_myclass_set_name(JSContext* ctx, JSValueConst this_val, int argc, JSValueConst* argv) {
    MyClass* instance = static_cast<MyClass*>(JS_GetOpaque(this_val, js_myclass_class_id));
    if (!instance) {
        return JS_ThrowTypeError(ctx, "Invalid MyClass instance");
    }

    const char* name = JS_ToCString(ctx, argv[0]);
    if (!name) {
        return JS_ThrowTypeError(ctx, "Expected a string");
    }

    instance->setName(name);
    JS_FreeCString(ctx, name);
    return JS_UNDEFINED;
}

// Define the class
static JSClassDef js_myclass_class = {
    "MyClass",
    .finalizer = js_myclass_finalizer,
};

// Export the class and methods
static const JSCFunctionListEntry js_myclass_proto_funcs[] = {
    JS_CFUNC_DEF("sayHello", 0, js_myclass_say_hello),
    JS_CFUNC_DEF("getName", 0, js_myclass_get_name),
    JS_CFUNC_DEF("setName", 1, js_myclass_set_name),
};

void register_myclass(JSContext* ctx) {
    auto rt = JS_GetRuntime(ctx);
    JS_NewClassID(rt, &js_myclass_class_id);
    JS_NewClass(rt, js_myclass_class_id, &js_myclass_class);

    JSValue proto = JS_NewObject(ctx);
    JS_DupValue(ctx, proto); // Duplicate the reference for safety

    JS_SetPropertyFunctionList(ctx, proto, js_myclass_proto_funcs, sizeof(js_myclass_proto_funcs) / sizeof(js_myclass_proto_funcs[0]));

    JSValue constructor = JS_NewCFunction2(ctx, js_myclass_constructor, "MyClass", 1, JS_CFUNC_constructor, 0);
    JS_DupValue(ctx, constructor); // Duplicate the reference for safety
    JS_SetConstructor(ctx, constructor, proto);
    JS_SetClassProto(ctx, js_myclass_class_id, proto);

    // Expose to the global object
    auto global_obj = JS_GetGlobalObject(ctx);
    JS_SetPropertyStr(ctx, global_obj, "MyClass", constructor);

    JS_FreeValue(ctx, global_obj);
    JS_FreeValue(ctx, proto);
    JS_FreeValue(ctx, constructor);
}

class QuickJSExposeClassTest : public ::testing::Test {
protected:
    JSContext* ctx;

    void SetUp() override {
        ctx = QjsHelper::getInstance().getContext();
        register_myclass(ctx);
    }
};

TEST_F(QuickJSExposeClassTest, TestExposeClassToQuickJS) {
    const char* script = R"(
        function testExposedCppClass() {
            let ret;
            const obj = new MyClass("QuickJS");
            ret = obj.sayHello();
            obj.setName("Trae");
            ret += ' ' + obj.sayHello();
            return ret;
        }
        testExposedCppClass();  // Execute immediately to avoid reference issues
    )";

    JSValue result = JS_Eval(ctx, script, strlen(script), "<input>", JS_EVAL_TYPE_GLOBAL);
    // Handle the result
    const char* result_str = JS_ToCString(ctx, result);
    EXPECT_STREQ(result_str, "Hello, QuickJS! Hello, Trae!");
    JS_FreeCString(ctx, result_str);
}
