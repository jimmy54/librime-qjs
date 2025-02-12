#ifndef RIME_QJS_CANDIDATE_H_
#define RIME_QJS_CANDIDATE_H_

#include "qjs_type_registry.h"
#include <rime/candidate.h>

namespace rime {

class QjsCandidate : public QjsTypeRegistry {
public:
  void Register(JSContext* ctx) override;
  const char* GetClassName() const override { return "Candidate"; }

  static JSValue Wrap(JSContext* ctx, an<Candidate> candidate);
  static Candidate* Unwrap(JSContext* ctx, JSValue value);

private:
  // Instance methods
  static JSValue get_text(JSContext* ctx, JSValueConst this_val);
  static JSValue set_text(JSContext* ctx, JSValueConst this_val, JSValue val);
  static JSValue get_comment(JSContext* ctx, JSValueConst this_val);
  static JSValue set_comment(JSContext* ctx, JSValueConst this_val, JSValue val);
  static JSValue get_type(JSContext* ctx, JSValueConst this_val);
  static JSValue set_type(JSContext* ctx, JSValueConst this_val, JSValue val);
  static JSValue get_start(JSContext* ctx, JSValueConst this_val);
  static JSValue set_start(JSContext* ctx, JSValueConst this_val, JSValue val);
  static JSValue get_end(JSContext* ctx, JSValueConst this_val);
  static JSValue set_end(JSContext* ctx, JSValueConst this_val, JSValue val);
  static JSValue get_quality(JSContext* ctx, JSValueConst this_val);
  static JSValue set_quality(JSContext* ctx, JSValueConst this_val, JSValue val);
  static JSValue get_preedit(JSContext* ctx, JSValueConst this_val);
  static JSValue set_preedit(JSContext* ctx, JSValueConst this_val, JSValue val);
};

} // namespace rime

#endif // RIME_QJS_CANDIDATE_H_
