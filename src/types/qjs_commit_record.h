#pragma once

#include <rime/commit_history.h>

#include "engines/js_macros.h"
#include "js_wrapper.h"

using namespace rime;

template <typename T_JS_VALUE>
class JsWrapper<rime::CommitRecord, T_JS_VALUE> {
  DEFINE_GETTER(CommitRecord, type, engine.toJsString(obj->type))
  DEFINE_GETTER(CommitRecord, text, engine.toJsString(obj->text))

  DEFINE_STRING_SETTER(CommitRecord, text, obj->text = str)
  DEFINE_STRING_SETTER(CommitRecord, type, obj->type = str)

public:
  EXPORT_CLASS_WITH_RAW_POINTER(CommitRecord,
                                WITHOUT_CONSTRUCTOR,
                                WITH_PROPERTIES(text, type),
                                WITHOUT_GETTERS,
                                WITHOUT_FUNCTIONS);
};
