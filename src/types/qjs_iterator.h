#include <type_traits>
#include <memory>
#include <quickjs.h>
// #include <boost/stacktrace.hpp> // Include Boost.Stacktrace

class AbstractIterator {
public:
    virtual ~AbstractIterator() = default;
    virtual bool next() = 0;
    virtual JSValue peek() = 0;
};

template<typename T>
class JSIteratorWrapper {
public:
    explicit JSIteratorWrapper(JSContext* ctx) : ctx_{ctx} {
        static_assert(std::is_base_of_v<AbstractIterator, T>,
                     "Template parameter T must inherit from AbstractIterator");

        auto rt = JS_GetRuntime(ctx);
        if (class_id_ == 0) {  // Only register the class once
            JS_NewClassID(rt, &class_id_);
            JS_NewClass(rt, class_id_, &class_def_);

            // Create and store the prototype globally
            proto_ = JS_NewObject(ctx);
            if (!JS_IsException(proto_)) {
                JS_SetPropertyFunctionList(ctx, proto_, proto_funcs_,
                                        sizeof(proto_funcs_) / sizeof(proto_funcs_[0]));
                // Store prototype in class registry
                JS_SetClassProto(ctx, class_id_, JS_DupValue(ctx, proto_));
            }
        } else {
            // Get the stored prototype
            proto_ = JS_GetClassProto(ctx, class_id_);
        }
    }

    ~JSIteratorWrapper() {
        if (!JS_IsUndefined(proto_)) {
            JS_FreeValue(ctx_, proto_);
        }
    }

    [[nodiscard]] JSValue createIterator(T* itor) const {
        // Create a shared_ptr to manage the iterator's lifetime
        auto iter = std::make_shared<T>(*itor);
        delete itor;  // Delete the original pointer since we've copied it

        JSValue obj = JS_NewObjectProtoClass(ctx_, proto_, class_id_);
        if (JS_IsException(obj)) {
            return obj;
        }

        // Store the shared_ptr in the opaque pointer
        auto* ptr = new std::shared_ptr<T>(iter);
        if (JS_SetOpaque(obj, ptr) == -1) {
            delete ptr;
            JS_FreeValue(ctx_, obj);
            return JS_EXCEPTION;
        }
        return obj;
    }

private:
    // Add these member variables
    static JSClassID class_id_;
    JSContext* ctx_{nullptr};
    JSValue proto_{JS_UNDEFINED};

    static void finalizer(JSRuntime* rt, JSValue val) {
        void* ptr = JS_GetOpaque(val, class_id_);
        if (ptr) {
            // Clean up the shared_ptr
            auto* iterPtr = static_cast<std::shared_ptr<T>*>(ptr);
            delete iterPtr;
        }
    }

    static JSValue next(JSContext* ctx, JSValueConst this_val, int, JSValueConst*) {
        auto* ptr = static_cast<std::shared_ptr<T>*>(JS_GetOpaque(this_val, class_id_));
        return ptr && ptr->get() ? JS_NewBool(ctx, (*ptr)->next()) : JS_ThrowTypeError(ctx, "Invalid iterator");
    }

    static JSValue peek(JSContext* ctx, JSValueConst this_val, int, JSValueConst*) {
        auto* ptr = static_cast<std::shared_ptr<T>*>(JS_GetOpaque(this_val, class_id_));
        return ptr && ptr->get() ? (*ptr)->peek() : JS_ThrowTypeError(ctx, "Invalid iterator");
    }

    static constexpr JSClassDef class_def_{
        "JSIteratorWrapper",
        .finalizer = finalizer
    };

    static constexpr JSCFunctionListEntry proto_funcs_[] = {
        JS_CFUNC_DEF("next", 0, next),
        JS_CFUNC_DEF("peek", 0, peek)
    };

    void registerClass() {
        proto_ = JS_NewObject(ctx_);
        if (JS_IsException(proto_)) {
            return;
        }

        // Set the prototype functions
        JS_SetPropertyFunctionList(ctx_, proto_, proto_funcs_,
                                 sizeof(proto_funcs_) / sizeof(proto_funcs_[0]));

        // Set the class prototype
        JS_SetClassProto(ctx_, class_id_, proto_);
    }
};
