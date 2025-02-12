#include "qjs_candidate.h"

namespace rime {

  static JSClassID js_candidate_class_id;

  static void js_candidate_finalizer(JSRuntime* rt, JSValue val) {
    // do not delete the candidate object here, since it is held by the shared pointer by other components
  }

  static JSClassDef js_candidate_class = {
    "Candidate",
    .finalizer = js_candidate_finalizer
  };

  void QjsCandidate::Register(JSContext* ctx) {
    auto rt = JS_GetRuntime(ctx);
    // Register class
    JS_NewClassID(rt, &js_candidate_class_id);
    JS_NewClass(rt, js_candidate_class_id, &js_candidate_class);

    // Create prototype
    JSValue proto = JS_NewObject(ctx);

    // Register properties with getters and setters
    const JSCFunctionListEntry properties[] = {
      JS_CGETSET_DEF("text", get_text, set_text),
      JS_CGETSET_DEF("comment", get_comment, set_comment),
      JS_CGETSET_DEF("type", get_type, set_type),
      JS_CGETSET_DEF("start", get_start, set_start),
      JS_CGETSET_DEF("end", get_end, set_end),
      JS_CGETSET_DEF("quality", get_quality, set_quality),
      JS_CGETSET_DEF("preedit", get_preedit, set_preedit),
    };
    JS_SetPropertyFunctionList(ctx, proto, properties, countof(properties));

    // Set the prototype for the class and keep a reference to it
    JSValue dup_proto = JS_DupValue(ctx, proto);
    JS_SetClassProto(ctx, js_candidate_class_id, dup_proto);

    JS_FreeValue(ctx, proto);
  }

  JSValue QjsCandidate::Wrap(JSContext* ctx, an<Candidate> candidate) {
    if (!candidate) return JS_NULL;

    // Create new object with the class ID
    JSValue obj = JS_NewObjectClass(ctx, js_candidate_class_id);
    if (JS_IsException(obj)) {
      return obj;
    }

    // Set the opaque pointer
    if (JS_SetOpaque(obj, candidate.get()) < 0) {
      JS_FreeValue(ctx, obj);
      return JS_EXCEPTION;
    }

    return obj;
  }

  Candidate* QjsCandidate::Unwrap(JSContext* ctx, JSValue value) {
    void* ptr = JS_GetOpaque(value, js_candidate_class_id);
    return static_cast<Candidate*>(ptr);
  }

  JSValue QjsCandidate::get_text(JSContext* ctx, JSValueConst this_val) {
    auto candidate = Unwrap(ctx, this_val);
    if (!candidate) {
      LOG(ERROR) << "[qjs] get_text: Candidate is null";
      return JS_UNDEFINED;
    }
    else {
      LOG(INFO) << "[qjs] get_text: " << candidate->text();
    }
    return JS_NewString(ctx, candidate->text().c_str());
  }

  JSValue QjsCandidate::get_comment(JSContext* ctx, JSValueConst this_val) {
    auto candidate = Unwrap(ctx, this_val);
    if (!candidate) return JS_UNDEFINED;
    return JS_NewString(ctx, candidate->comment().c_str());
  }

  JSValue QjsCandidate::get_type(JSContext* ctx, JSValueConst this_val) {
    auto candidate = Unwrap(ctx, this_val);
    if (!candidate) return JS_UNDEFINED;
    return JS_NewString(ctx, candidate->type().c_str());
  }

  JSValue QjsCandidate::get_start(JSContext* ctx, JSValueConst this_val) {
    auto candidate = Unwrap(ctx, this_val);
    if (!candidate) return JS_UNDEFINED;
    return JS_NewInt64(ctx, candidate->start());
  }

  JSValue QjsCandidate::get_end(JSContext* ctx, JSValueConst this_val) {
    auto candidate = Unwrap(ctx, this_val);
    if (!candidate) return JS_UNDEFINED;
    return JS_NewInt64(ctx, candidate->end());
  }

  JSValue QjsCandidate::get_quality(JSContext* ctx, JSValueConst this_val) {
    auto candidate = Unwrap(ctx, this_val);
    if (!candidate) return JS_UNDEFINED;
    return JS_NewInt32(ctx, candidate->quality());
  }

  JSValue QjsCandidate::get_preedit(JSContext* ctx, JSValueConst this_val) {
    auto candidate = Unwrap(ctx, this_val);
    if (!candidate) return JS_UNDEFINED;
    return JS_NewString(ctx, candidate->preedit().c_str());
  }

  JSValue QjsCandidate::set_text(JSContext* ctx, JSValueConst this_val, JSValue val) {
    auto candidate = Unwrap(ctx, this_val);
    if (!candidate) return JS_UNDEFINED;
    const char* text = JS_ToCString(ctx, val);
    if (text) {
      auto simpleCandidate = dynamic_cast<SimpleCandidate*>(candidate);
      if (simpleCandidate) {
        simpleCandidate->set_text(text);
      }
      JS_FreeCString(ctx, text);
    }
    return JS_UNDEFINED;
  }

  JSValue QjsCandidate::set_comment(JSContext* ctx, JSValueConst this_val, JSValue val) {
    auto candidate = Unwrap(ctx, this_val);
    if (!candidate) return JS_UNDEFINED;
    const char* comment = JS_ToCString(ctx, val);
    if (comment) {
      auto simpleCandidate = dynamic_cast<SimpleCandidate*>(candidate);
      if (simpleCandidate) {
        simpleCandidate->set_comment(comment);
      }
      JS_FreeCString(ctx, comment);
    }
    return JS_UNDEFINED;
  }

  JSValue QjsCandidate::set_type(JSContext* ctx, JSValueConst this_val, JSValue val) {
    auto candidate = Unwrap(ctx, this_val);
    if (!candidate) return JS_UNDEFINED;
    const char* type = JS_ToCString(ctx, val);
    if (type) {
      candidate->set_type(type);
      JS_FreeCString(ctx, type);
    }
    return JS_UNDEFINED;
  }

  JSValue QjsCandidate::set_start(JSContext* ctx, JSValueConst this_val, JSValue val) {
    auto candidate = Unwrap(ctx, this_val);
    if (!candidate) return JS_UNDEFINED;
    int64_t start;
    if (JS_ToInt64(ctx, &start, val) == 0) {
      candidate->set_start(start);
    }
    return JS_UNDEFINED;
  }

  JSValue QjsCandidate::set_end(JSContext* ctx, JSValueConst this_val, JSValue val) {
    auto candidate = Unwrap(ctx, this_val);
    if (!candidate) return JS_UNDEFINED;
    int64_t end;
    if (JS_ToInt64(ctx, &end, val) == 0) {
      candidate->set_end(end);
    }
    return JS_UNDEFINED;
  }

  JSValue QjsCandidate::set_quality(JSContext* ctx, JSValueConst this_val, JSValue val) {
    auto candidate = Unwrap(ctx, this_val);
    if (!candidate) return JS_UNDEFINED;
    int32_t quality;
    if (JS_ToInt32(ctx, &quality, val) == 0) {
      candidate->set_quality(quality);
    }
    return JS_UNDEFINED;
  }

  JSValue QjsCandidate::set_preedit(JSContext* ctx, JSValueConst this_val, JSValue val) {
    auto candidate = Unwrap(ctx, this_val);
    if (!candidate) return JS_UNDEFINED;
    const char* preedit = JS_ToCString(ctx, val);
    if (preedit) {
      auto simpleCandidate = dynamic_cast<SimpleCandidate*>(candidate);
      if (simpleCandidate) {
        simpleCandidate->set_preedit(preedit);
      }
      JS_FreeCString(ctx, preedit);
    }
    return JS_UNDEFINED;
  }
} // namespace rime
