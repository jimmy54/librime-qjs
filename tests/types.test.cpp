#include <gtest/gtest.h>
#include "jsvalue_raii.h"
#include "qjs_candidate.h"
#include "qjs_engine.h"
#include <rime/candidate.h>
#include <rime/engine.h>
#include <rime/schema.h>
#include <rime/context.h>
#include <rime/config/config_component.h>

using namespace rime;

class QuickJSTypesTest : public ::testing::Test {
protected:
    void SetUp() override {
        QjsHelper::basePath = "tests/js";
    }
};

TEST_F(QuickJSTypesTest, WrapUnwrapRimeGears) {
    auto ctx = QjsHelper::getInstance().getContext();
    QjsHelper::exposeLogToJsConsole(ctx);

    auto arg = JSValueRAII(JS_NewObject(ctx));
    auto engine = Engine::Create();
    // engine->ApplySchema(&schema); // ApplySchema 会触发回调函数，导致 segfault
    // engine->schema()->schema_id() = .default, engine->schema()->schema_name() = .default
    ASSERT_TRUE(engine->schema() != nullptr);
    auto config = engine->schema()->config();
    ASSERT_TRUE(config != nullptr);
    config->SetBool("key1", true);
    config->SetBool("key2", false);
    config->SetInt("key3", 666);
    config->SetDouble("key4", 0.999);
    config->SetString("key5", "string");

    auto list = New<ConfigList>();
    list->Append(New<ConfigValue>("item1"));
    list->Append(New<ConfigValue>("item2"));
    list->Append(New<ConfigValue>("item3"));
    config->SetItem("list", list);

    auto context = engine->context();
    ASSERT_TRUE(context != nullptr);
    context->set_input("hello");

    auto jsEngine = JSValueRAII(QjsEngine::Wrap(ctx, engine));
    JS_SetPropertyStr(ctx, arg, "engine", jsEngine);

    JSValueRAII jsNamespace(JS_NewString(ctx, "namespace"));
    JS_SetPropertyStr(ctx, arg, "namespace", jsNamespace);

    auto candidate = New<SimpleCandidate>("mock", 0, 1, "text", "comment");
    auto jsCandidate = QjsCandidate::Wrap(ctx, candidate);
    JS_SetPropertyStr(ctx, arg, "candidate", jsCandidate);

    QjsHelper::loadJsModuleToGlobalThis(ctx, "types_test.js");

    JSValueRAII global(JS_GetGlobalObject(ctx));
    JSValueRAII jsFunc(JS_GetPropertyStr(ctx, global, "checkArgument"));
    JSValueRAII retValue(JS_Call(ctx, jsFunc, JS_UNDEFINED, 1, (JSValueConst[]){arg.get()}));

    Engine* retEngine = QjsEngine::Unwrap(ctx, JS_GetPropertyStr(ctx, retValue, "engine"));
    ASSERT_EQ(retEngine, engine);
    ASSERT_EQ(retEngine->schema()->schema_name(), engine->schema()->schema_name());
    an<Candidate> retCandidate = QjsCandidate::Unwrap(ctx, JS_GetPropertyStr(ctx, retValue, "candidate"));
    ASSERT_EQ(retCandidate->text(), "new text");
    ASSERT_EQ(retCandidate.get(), candidate.get());

    Config* retConfig = retEngine->schema()->config();
    string greet;
    bool success = retEngine->schema()->config()->GetString("greet", &greet);
    ASSERT_TRUE(success);
    ASSERT_EQ(greet, "hello from js");

    Context* retContext = retEngine->context();
    ASSERT_EQ(retContext->input(), "world");

    // arg.newCandidate = new Candidate('js', 32, 100, 'the text', 'the comment', 888)
    auto newCandidate = QjsCandidate::Unwrap(ctx, JS_GetPropertyStr(ctx, retValue, "newCandidate"));
    ASSERT_EQ(newCandidate->type(), "js");
    ASSERT_EQ(newCandidate->start(), 32);
    ASSERT_EQ(newCandidate->end(), 100);
    ASSERT_EQ(newCandidate->text(), "the text");
    ASSERT_EQ(newCandidate->comment(), "the comment");
    ASSERT_EQ(newCandidate->quality(), 888);
}
