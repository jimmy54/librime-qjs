#include <rime/gear/translator_commons.h>
#include "qjs_candidate.h"
#include <memory>

namespace {
  JSValue js_new_string_from_std(JSContext* ctx, const std::string& str) {
    return JS_NewString(ctx, str.c_str());
  }
}

#define DEFINE_CANDIDATE_GETTER(name, type, converter)                     \
  [[nodiscard]]                                                           \
  JSValue QjsCandidate::get_##name(JSContext* ctx, JSValueConst this_val) { \
    if (auto candidate = Unwrap(ctx, this_val)) {                         \
      return converter(ctx, candidate->name());                           \
    }                                                                     \
    return JS_UNDEFINED;                                                  \
  }

#define DEFINE_CANDIDATE_STRING_SETTER(name, assignment)                   \
  JSValue QjsCandidate::set_##name(JSContext* ctx, JSValueConst this_val, JSValue val) { \
    if (auto candidate = Unwrap(ctx, this_val)) {                         \
      if (const char* str = JS_ToCString(ctx, val)) {                     \
        assignment                                                        \
        JS_FreeCString(ctx, str);                                        \
      }                                                                   \
    }                                                                     \
    return JS_UNDEFINED;                                                  \
  }

#define DEFINE_CANDIDATE_NUMERIC_SETTER(name, type, converter)            \
  JSValue QjsCandidate::set_##name(JSContext* ctx, JSValueConst this_val, JSValue val) { \
    if (auto candidate = Unwrap(ctx, this_val)) {                         \
      type value;                                                         \
      if (converter(ctx, &value, val) == 0) {                            \
        candidate->set_##name(value);                                     \
      }                                                                   \
    }                                                                     \
    return JS_UNDEFINED;                                                  \
  }

namespace rime {

  static JSClassID js_candidate_class_id;

  static void js_candidate_finalizer(JSRuntime* rt, JSValue val) {
    if (auto ptr = JS_GetOpaque(val, js_candidate_class_id)) {
      delete static_cast<std::shared_ptr<Candidate>*>(ptr);
    }
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

  [[nodiscard]]
  JSValue QjsCandidate::Wrap(JSContext* ctx, an<Candidate> candidate) {
    if (!candidate) return JS_NULL;

    auto obj = JS_NewObjectClass(ctx, js_candidate_class_id);
    if (JS_IsException(obj)) {
      return obj;
    }

    auto ptr = std::make_unique<std::shared_ptr<Candidate>>(candidate);
    if (JS_SetOpaque(obj, ptr.release()) < 0) {
      JS_FreeValue(ctx, obj);
      return JS_EXCEPTION;
    }

    return obj;
  }

  [[nodiscard]]
  an<Candidate> QjsCandidate::Unwrap(JSContext* ctx, JSValue value) {
    if (auto ptr = JS_GetOpaque(value, js_candidate_class_id)) {
      if (auto sharedPtr = static_cast<std::shared_ptr<Candidate>*>(ptr)) {
        return *sharedPtr;
      }
    }
    return nullptr;
  }

  DEFINE_CANDIDATE_GETTER(text, const string&, js_new_string_from_std)
  DEFINE_CANDIDATE_GETTER(comment, const string&, js_new_string_from_std)
  DEFINE_CANDIDATE_GETTER(type, const string&, js_new_string_from_std)
  DEFINE_CANDIDATE_GETTER(start, size_t, JS_NewInt64)
  DEFINE_CANDIDATE_GETTER(end, size_t, JS_NewInt64)
  DEFINE_CANDIDATE_GETTER(quality, int, JS_NewInt32)
  DEFINE_CANDIDATE_GETTER(preedit, const string&, js_new_string_from_std)

  DEFINE_CANDIDATE_STRING_SETTER(text,
    if (auto simpleCandidate = dynamic_cast<SimpleCandidate*>(candidate.get())) {
      simpleCandidate->set_text(str);
    }
  )

  DEFINE_CANDIDATE_STRING_SETTER(comment,
    if (auto simpleCandidate = dynamic_cast<SimpleCandidate*>(candidate.get())) {
      simpleCandidate->set_comment(str);
    } else if (auto phrase = dynamic_cast<Phrase*>(candidate.get())) {
      phrase->set_comment(str);
    }
  )

  DEFINE_CANDIDATE_STRING_SETTER(type,
    candidate->set_type(str);
  )

  DEFINE_CANDIDATE_NUMERIC_SETTER(start, int64_t, JS_ToInt64)
  DEFINE_CANDIDATE_NUMERIC_SETTER(end, int64_t, JS_ToInt64)
  DEFINE_CANDIDATE_NUMERIC_SETTER(quality, int32_t, JS_ToInt32)

  DEFINE_CANDIDATE_STRING_SETTER(preedit,
    if (auto simpleCandidate = dynamic_cast<SimpleCandidate*>(candidate.get())) {
      simpleCandidate->set_preedit(str);
    } else if (auto phrase = dynamic_cast<Phrase*>(candidate.get())) {
      phrase->set_preedit(str);
    }
  )

} // namespace rime

#undef DEFINE_CANDIDATE_GETTER
#undef DEFINE_CANDIDATE_STRING_SETTER
#undef DEFINE_CANDIDATE_NUMERIC_SETTER
