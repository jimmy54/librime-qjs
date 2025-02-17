#ifndef RIME_QJS_JSVALUE_RAII_H_
#define RIME_QJS_JSVALUE_RAII_H_

#include <glog/logging.h>
#include <quickjs.h>
#include "qjs_helper.h"

namespace rime {

class JSValueRAII {
public:
    explicit JSValueRAII(JSValue val) : val_(val) {
        if (JS_IsException(val)) {
            auto ctx = QjsHelper::getInstance().getContext();
            JSValue exception = JS_GetException(ctx);
            const char *message = JS_ToCString(ctx, exception);
            LOG(ERROR) << "[qjs] JS exception: " << message;
            JS_FreeCString(ctx, message);  // Free the C string

            JSValueRAII stack(JS_GetPropertyStr(ctx, exception, "stack"));
            const char *stack_trace = JS_ToCString(ctx, stack);
            if (stack_trace) {
                LOG(ERROR) << "[qjs] JS stack trace: " << stack_trace;
                JS_FreeCString(ctx, stack_trace);  // Free the C string
            } else {
                LOG(ERROR) << "[qjs] JS stack trace is null.";
            }

            JS_FreeValue(ctx, exception);
        }
    }
    ~JSValueRAII() {
        auto ctx = QjsHelper::getInstance().getContext();
        if (ctx && JS_IsUndefined(val_)) {  // Only free if we have a valid context and value
            JS_FreeValue(ctx, val_);
        }
    }

    // Add move constructor and assignment
    JSValueRAII(JSValueRAII&& other) noexcept : val_(other.val_) {
        other.val_ = JS_UNDEFINED;
    }

    JSValueRAII& operator=(JSValueRAII&& other) noexcept {
        if (this != &other) {
            auto ctx = QjsHelper::getInstance().getContext();
            if (ctx && JS_IsUndefined(val_)) {
                JS_FreeValue(ctx, val_);
            }
            val_ = other.val_;
            other.val_ = JS_UNDEFINED;
        }
        return *this;
    }

    // Delete copy constructor and assignment
    JSValueRAII(const JSValueRAII&) = delete;
    JSValueRAII& operator=(const JSValueRAII&) = delete;

    operator JSValue() const { return val_; }
    JSValue get() const { return val_; }
    JSValue* getPtr() { return &val_; }

    // Add methods to duplicate/protect values
    JSValue dup() const {
        auto ctx = QjsHelper::getInstance().getContext();
        return JS_DupValue(ctx, val_);
    }

private:
    JSValue val_;
};

} // namespace rime

#endif // RIME_QJS_JSVALUE_RAII_H_
