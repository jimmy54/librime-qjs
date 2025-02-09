#include "qjs_candidate.h"

namespace rime {

static JSClassID js_candidate_class_id;

static void js_candidate_finalizer(JSRuntime* rt, JSValue val) {
  auto candidate = static_cast<an<Candidate>*>(JS_GetOpaque(val, js_candidate_class_id));
  delete candidate;
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

  // Register methods
  const JSCFunctionListEntry methods[] = {
    JS_CFUNC_DEF("getText", 0, get_text),
    JS_CFUNC_DEF("getComment", 0, get_comment),
    JS_CFUNC_DEF("getType", 0, get_type),
    JS_CFUNC_DEF("getStart", 0, get_start),
    JS_CFUNC_DEF("getEnd", 0, get_end),
    JS_CFUNC_DEF("getQuality", 0, get_quality),
    JS_CFUNC_DEF("getPreedit", 0, get_preedit),
  };
  JS_SetPropertyFunctionList(ctx, proto, methods, countof(methods));

  // No constructor - Candidate objects are created by other components
  JS_SetClassProto(ctx, js_candidate_class_id, proto);

  JS_FreeValue(ctx, proto);
}

// JSValue QjsCandidate::Wrap(JSContext* ctx, an<Candidate> candidate) {
//   if (!candidate) return JS_NULL;
//   JSValue obj = JS_NewObjectClass(ctx, js_candidate_class_id);
//   void *ptr = static_cast<void*>(&candidate);
//   JS_SetOpaque(obj, ptr);
//   // JS_SetOpaque(obj, candidate.get());
//   return obj;
// }

// an<Candidate> QjsCandidate::Unwrap(JSContext* ctx, JSValue value) {
//   void *ptr = JS_GetOpaque(value, js_candidate_class_id);
//   // LOG(INFO) << "[qjs] Unwrap ptr: " << (ptr ? ptr : nullptr) ;
//   an<Candidate> *candidate = static_cast<an<Candidate>*>(ptr);
//   return *candidate;
// }


JSValue QjsCandidate::Wrap(JSContext* ctx, an<Candidate> candidate) {
  if (!candidate) return JS_NULL;
  JSValue obj = JS_NewObjectClass(ctx, js_candidate_class_id);
  JS_SetOpaque(obj, candidate.get());
  return obj;
}

Candidate* QjsCandidate::Unwrap(JSContext* ctx, JSValue value) {
  void *ptr = JS_GetOpaque(value, js_candidate_class_id);
  return static_cast<Candidate*>(ptr);
}

JSValue QjsCandidate::get_text(JSContext* ctx, JSValueConst this_val, int argc, JSValueConst* argv) {
  auto candidate = Unwrap(ctx, this_val);
  if (!candidate) {
    LOG(ERROR) << "[qjs] get_text: Candidate is null";
    return JS_UNDEFINED;
  } else {
    LOG(INFO) << "[qjs] get_text: " << candidate->text();
  }
  return JS_NewString(ctx, candidate->text().c_str());
}

JSValue QjsCandidate::get_comment(JSContext* ctx, JSValueConst this_val, int argc, JSValueConst* argv) {
  auto candidate = Unwrap(ctx, this_val);
  if (!candidate) return JS_UNDEFINED;
  return JS_NewString(ctx, candidate->comment().c_str());
}

JSValue QjsCandidate::get_type(JSContext* ctx, JSValueConst this_val, int argc, JSValueConst* argv) {
  auto candidate = Unwrap(ctx, this_val);
  if (!candidate) return JS_UNDEFINED;
  return JS_NewString(ctx, candidate->type().c_str());
}

JSValue QjsCandidate::get_start(JSContext* ctx, JSValueConst this_val, int argc, JSValueConst* argv) {
  auto candidate = Unwrap(ctx, this_val);
  if (!candidate) return JS_UNDEFINED;
  return JS_NewInt64(ctx, candidate->start());
}

JSValue QjsCandidate::get_end(JSContext* ctx, JSValueConst this_val, int argc, JSValueConst* argv) {
  auto candidate = Unwrap(ctx, this_val);
  if (!candidate) return JS_UNDEFINED;
  return JS_NewInt64(ctx, candidate->end());
}

JSValue QjsCandidate::get_quality(JSContext* ctx, JSValueConst this_val, int argc, JSValueConst* argv) {
  auto candidate = Unwrap(ctx, this_val);
  if (!candidate) return JS_UNDEFINED;
  return JS_NewInt32(ctx, candidate->quality());
}

JSValue QjsCandidate::get_preedit(JSContext* ctx, JSValueConst this_val, int argc, JSValueConst* argv) {
  auto candidate = Unwrap(ctx, this_val);
  if (!candidate) return JS_UNDEFINED;
  return JS_NewString(ctx, candidate->preedit().c_str());
}

} // namespace rime
