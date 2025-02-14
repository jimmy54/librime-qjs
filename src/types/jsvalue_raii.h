#ifndef RIME_QJS_JSVALUE_RAII_H_
#define RIME_QJS_JSVALUE_RAII_H_

#include <glog/logging.h>
#include <quickjs.h>

namespace rime {

class JSValueRAII {
public:
    explicit JSValueRAII(JSValue val) : val_(val) {
        ctx_ = context_;
        if (JS_IsException(val)) {
            JSValueRAII exception(JS_GetException(context_));
            const char *message = JS_ToCString(context_, exception);
            LOG(ERROR) << "[qjs] JS exception: " << message;
            JS_FreeCString(context_, message);  // Free the C string

            JSValueRAII stack(JS_GetPropertyStr(context_, exception, "stack"));
            const char *stack_trace = JS_ToCString(context_, stack);
            if (stack_trace) {
                LOG(ERROR) << "[qjs] JS stack trace: " << stack_trace;
                JS_FreeCString(context_, stack_trace);  // Free the C string
            } else {
                LOG(ERROR) << "[qjs] JS stack trace is null.";
            }
        }
    }
    ~JSValueRAII() {
        if (ctx_ && JS_IsUndefined(val_)) {  // Only free if we have a valid context and value
            JS_FreeValue(ctx_, val_);
        }
    }

    // Add move constructor and assignment
    JSValueRAII(JSValueRAII&& other) noexcept : val_(other.val_), ctx_(other.ctx_) {
        other.val_ = JS_UNDEFINED;
        other.ctx_ = nullptr;
    }

    JSValueRAII& operator=(JSValueRAII&& other) noexcept {
        if (this != &other) {
            if (ctx_ && JS_IsUndefined(val_)) {
                JS_FreeValue(ctx_, val_);
            }
            val_ = other.val_;
            ctx_ = other.ctx_;
            other.val_ = JS_UNDEFINED;
            other.ctx_ = nullptr;
        }
        return *this;
    }

    // Delete copy constructor and assignment
    JSValueRAII(const JSValueRAII&) = delete;
    JSValueRAII& operator=(const JSValueRAII&) = delete;

    operator JSValue() const { return val_; }
    JSValue get() const { return val_; }
    JSValue* getPtr() { return &val_; }

    static JSContext* context_;

    // Add methods to duplicate/protect values
    JSValue dup() const {
        return JS_DupValue(ctx_, val_);
    }

private:
    JSValue val_;
    JSContext* ctx_{nullptr};
};

} // namespace rime

#endif // RIME_QJS_JSVALUE_RAII_H_
