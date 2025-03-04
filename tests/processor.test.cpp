#include <gtest/gtest.h>
#include "qjs_types.h"
#include "qjs_processor.h"
#include "qjs_key_event.h"
#include "qjs_engine.h"
#include <rime/key_event.h>
#include <rime/engine.h>
#include <rime/schema.h>
#include <rime/context.h>
#include <rime/config/config_component.h>

using namespace rime;

class QuickJSProcessorTest : public ::testing::Test {
protected:
    void SetUp() override {
        QjsHelper::basePath = "tests/js";
    }
};

TEST_F(QuickJSProcessorTest, ProcessKeyEvent) {
    auto engine = Engine::Create();
    ASSERT_TRUE(engine->schema() != nullptr);

    auto config = engine->schema()->config();
    ASSERT_TRUE(config != nullptr);
    config->SetString("greet", "hello from c++");

    Segment segment(0, 10);
    segment.prompt = "prompt";
    engine->context()->composition().AddSegment(segment);

    Ticket ticket(engine, "processor", "qjs_processor@processor_test");
    auto processor = New<QuickJSProcessor>(ticket);

    // Test key event that should be accepted
    KeyEvent acceptEvent("space");
    EXPECT_EQ(processor->ProcessKeyEvent(acceptEvent), kAccepted);

    // Test key event that should be rejected
    KeyEvent rejectEvent("Return");
    EXPECT_EQ(processor->ProcessKeyEvent(rejectEvent), kRejected);

    // Test key event that should result in noop
    KeyEvent noopEvent("invalid_key");
    EXPECT_EQ(processor->ProcessKeyEvent(noopEvent), kNoop);
}

TEST_F(QuickJSProcessorTest, NonExistentModule) {
    auto engine = Engine::Create();
    ASSERT_TRUE(engine->schema() != nullptr);

    Segment segment(0, 10);
    segment.prompt = "prompt";
    engine->context()->composition().AddSegment(segment);

    // Create a ticket with a non-existent module
    Ticket ticket(engine, "processor", "qjs_processor@non_existent");
    auto processor = New<QuickJSProcessor>(ticket);

    // Test key event - should return noop due to unloaded module
    KeyEvent event("space");
    EXPECT_EQ(processor->ProcessKeyEvent(event), kNoop);
}

TEST_F(QuickJSProcessorTest, ModuleInitialization) {
    auto engine = Engine::Create();
    ASSERT_TRUE(engine->schema() != nullptr);

    auto config = engine->schema()->config();
    ASSERT_TRUE(config != nullptr);
    config->SetString("init_test", "test_value");

    Segment segment(0, 10);
    segment.prompt = "prompt";
    engine->context()->composition().AddSegment(segment);

    Ticket ticket(engine, "processor", "qjs_processor@processor_test");
    auto processor = New<QuickJSProcessor>(ticket);

    // Test a key event to verify the processor was properly initialized
    KeyEvent event("space");
    EXPECT_EQ(processor->ProcessKeyEvent(event), kAccepted);
}
