#ifndef RIME_QJS_JSVALUE_RAII_H_
#define RIME_QJS_JSVALUE_RAII_H_

#include <glog/logging.h>
#include <quickjs.h>
#include "qjs_helper.h"

namespace rime {

class JSValueRAII {
public:
    JSValueRAII(JSValue val) : val_(val) {
        if (JS_IsException(val)) {
            auto *ctx = QjsHelper::getInstance().getContext();
            JSValue exception = JS_GetException(ctx);
            const char *message = JS_ToCString(ctx, exception);
            LOG(ERROR) << "[qjs] JS exception: " << message;
            JS_FreeCString(ctx, message);  // Free the C string

            JSValue stack = JS_GetPropertyStr(ctx, exception, "stack");
            const char *stackTrace = JS_ToCString(ctx, stack);
            if (stackTrace != nullptr && *stackTrace != '\0') {
                LOG(ERROR) << "[qjs] JS stack trace: " << stackTrace;
                JS_FreeCString(ctx, stackTrace);  // Free the C string
            } else {
                LOG(ERROR) << "[qjs] JS stack trace is null.";
            }

            JS_FreeValue(ctx, stack);
            JS_FreeValue(ctx, exception);
        }
    }

    ~JSValueRAII() {
        auto *ctx = QjsHelper::getInstance().getContext();
        JS_FreeValue(ctx, val_);
    }

    // Move constructor
    JSValueRAII(JSValueRAII&& other) noexcept : val_(other.val_) {
        other.val_ = JS_UNDEFINED;
    }

    // Move assignment
    JSValueRAII& operator=(JSValueRAII&& other) noexcept {
        if (this != &other) {
            auto *ctx = QjsHelper::getInstance().getContext();
            JS_FreeValue(ctx, val_);
            val_ = other.val_;
            other.val_ = JS_UNDEFINED;
        }
        return *this;
    }

    // Delete copy constructor and assignment
    JSValueRAII(const JSValueRAII&) = delete;
    JSValueRAII& operator=(const JSValueRAII&) = delete;

    operator JSValue() const { return val_; }
    [[nodiscard]] JSValue get() const { return val_; }
    JSValue* getPtr() { return &val_; }

private:
    JSValue val_;
};

} // namespace rime

#endif // RIME_QJS_JSVALUE_RAII_H_
