#pragma once

#include <rime/context.h>
#include <memory>
#include "engines/js_macros.h"
#include "js_wrapper.h"

using namespace rime;

template <typename T_JS_VALUE>
class JsWrapper<rime::Context, T_JS_VALUE> : public JsWrapperBase<T_JS_VALUE> {
  DEFINE_GETTER(Context, input, engine.toJsString(obj->input()))
  DEFINE_GETTER(Context, caretPos, engine.toJsInt(obj->caret_pos()))

  DEFINE_STRING_SETTER(Context, input, obj->set_input(str);)
  DEFINE_SETTER(Context, caretPos, engine.toInt, obj->set_caret_pos(value))

  DEFINE_GETTER(Context, preedit, engine.wrapShared(std::make_shared<Preedit>(obj->GetPreedit())))

  DEFINE_GETTER(Context,
                lastSegment,
                obj->composition().empty()
                    ? engine.null()
                    : engine.wrap<Segment>(
                          &obj->composition().back()));  // <-- & is required to pass the reference!

  DEFINE_CFUNCTION(commit, {
    auto obj = engine.unwrap<Context>(thisVal);
    obj->Commit();
    return engine.undefined();
  })

  DEFINE_CFUNCTION(getCommitText, {
    auto obj = engine.unwrap<Context>(thisVal);
    return engine.toJsString(obj->GetCommitText().c_str());
  })

  DEFINE_CFUNCTION(clear, {
    auto obj = engine.unwrap<Context>(thisVal);
    obj->Clear();
    return engine.undefined();
  })

  DEFINE_CFUNCTION(hasMenu, {
    auto obj = engine.unwrap<Context>(thisVal);
    return engine.toJsBool(obj->HasMenu());
  })

public:
  static const char* getTypeName() { return "Context"; }

  EXPORT_PROPERTIES(input, caretPos);
  EXPORT_GETTERS(preedit, lastSegment);
  EXPORT_FUNCTIONS(commit, 0, getCommitText, 0, clear, 0, hasMenu, 0);
};
