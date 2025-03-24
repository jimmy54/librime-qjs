#include <quickjs.h>

#include <memory>
#include <type_traits>
// #include <boost/stacktrace.hpp> // Include Boost.Stacktrace

class AbstractIterator {
public:
  virtual ~AbstractIterator() = default;
  virtual bool next() = 0;
  virtual JSValue peek() = 0;
};

template <typename T>
class JSIteratorWrapper {
public:
  JSIteratorWrapper(const JSIteratorWrapper&) = delete;
  JSIteratorWrapper(JSIteratorWrapper&&) = delete;
  JSIteratorWrapper& operator=(const JSIteratorWrapper&) = delete;
  JSIteratorWrapper& operator=(JSIteratorWrapper&&) = delete;

  explicit JSIteratorWrapper(JSContext* ctx) : ctx_{ctx} {
    static_assert(std::is_base_of_v<AbstractIterator, T>,
                  "Template parameter T must inherit from AbstractIterator");

    auto* rt = JS_GetRuntime(ctx);
    if (classId == 0) {  // Only register the class once
      JS_NewClassID(rt, &classId);
      JS_NewClass(rt, classId, &CLASS_DEF);

      // Create and store the prototype globally
      proto_ = JS_NewObject(ctx);
      if (!JS_IsException(proto_)) {
        JS_SetPropertyFunctionList(ctx, proto_,
                                   static_cast<const JSCFunctionListEntry*>(PROTO_FUNCS),
                                   sizeof(PROTO_FUNCS) / sizeof(PROTO_FUNCS[0]));
        // Store prototype in class registry
        JS_SetClassProto(ctx, classId, JS_DupValue(ctx, proto_));
      }
    } else {
      // Get the stored prototype
      proto_ = JS_GetClassProto(ctx, classId);
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

    JSValue obj = JS_NewObjectProtoClass(ctx_, proto_, classId);
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
  // NOLINTNEXTLINE(cppcoreguidelines-avoid-non-const-global-variables)
  static JSClassID classId;
  JSContext* ctx_{nullptr};
  JSValue proto_{JS_UNDEFINED};

  static void finalizer(JSRuntime* rt, JSValue val) {
    void* ptr = JS_GetOpaque(val, classId);
    if (ptr != nullptr) {
      // Clean up the shared_ptr
      auto* iterPtr = static_cast<std::shared_ptr<T>*>(ptr);
      delete iterPtr;
    }
  }

  static JSValue next(JSContext* ctx, JSValueConst thisVal, int /*unused*/, JSValueConst*) {
    auto* ptr = static_cast<std::shared_ptr<T>*>(JS_GetOpaque(thisVal, classId));
    return ptr && ptr->get() ? JS_NewBool(ctx, (*ptr)->next())
                             : JS_ThrowTypeError(ctx, "Invalid iterator");
  }

  static JSValue peek(JSContext* ctx, JSValueConst thisVal, int /*unused*/, JSValueConst*) {
    auto* ptr = static_cast<std::shared_ptr<T>*>(JS_GetOpaque(thisVal, classId));
    return ptr && ptr->get() ? (*ptr)->peek() : JS_ThrowTypeError(ctx, "Invalid iterator");
  }

  static constexpr JSClassDef CLASS_DEF{
      .class_name = "JSIteratorWrapper",
      .finalizer = finalizer,
      .gc_mark = nullptr,
      .call = nullptr,
      .exotic = nullptr,
  };

  static constexpr JSCFunctionListEntry PROTO_FUNCS[] = {JS_CFUNC_DEF("next", 0, next),
                                                         JS_CFUNC_DEF("peek", 0, peek)};

  void registerClass() {
    proto_ = JS_NewObject(ctx_);
    if (JS_IsException(proto_)) {
      return;
    }

    // Set the prototype functions
    JS_SetPropertyFunctionList(ctx_, proto_, static_cast<const JSCFunctionListEntry*>(PROTO_FUNCS),
                               sizeof(PROTO_FUNCS) / sizeof(PROTO_FUNCS[0]));

    // Set the class prototype
    JS_SetClassProto(ctx_, classId, proto_);
  }
};
