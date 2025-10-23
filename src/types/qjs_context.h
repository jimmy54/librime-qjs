#pragma once

#include <rime/context.h>
#include <memory>
#include "engines/js_macros.h"
#include "js_wrapper.h"

using namespace rime;

template <>
class JsWrapper<rime::Context> {
  DEFINE_GETTER(Context, input, obj->input())
  DEFINE_GETTER(Context, caretPos, obj->caret_pos())

  DEFINE_STRING_SETTER(Context, input, obj->set_input(str);)
  DEFINE_SETTER(Context, caretPos, engine.toInt, obj->set_caret_pos(value))

  DEFINE_GETTER(Context, preedit, std::make_shared<Preedit>(obj->GetPreedit()))

  DEFINE_GETTER(Context,
                lastSegment,
                obj->composition().empty() ? nullptr : &obj->composition().back());

  DEFINE_GETTER(Context, commitNotifier, &obj->commit_notifier())
  DEFINE_GETTER(Context, selectNotifier, &obj->select_notifier())
  DEFINE_GETTER(Context, updateNotifier, &obj->update_notifier())
  DEFINE_GETTER(Context, deleteNotifier, &obj->delete_notifier())

  DEFINE_GETTER(Context, commitHistory, &obj->commit_history())

  // External context getters
  DEFINE_GETTER(Context, externalPrecedingText, obj->external_preceding_text())
  DEFINE_GETTER(Context, externalFollowingText, obj->external_following_text())

  DEFINE_CFUNCTION(commit, {
    auto obj = engine.unwrap<Context>(thisVal);
    obj->Commit();
    return engine.undefined();
  })

  DEFINE_CFUNCTION(getCommitText, {
    auto obj = engine.unwrap<Context>(thisVal);
    return engine.wrap(obj->GetCommitText());
  })

  DEFINE_CFUNCTION(clear, {
    auto obj = engine.unwrap<Context>(thisVal);
    obj->Clear();
    return engine.undefined();
  })

  DEFINE_CFUNCTION(hasMenu, {
    auto obj = engine.unwrap<Context>(thisVal);
    return engine.wrap(obj->HasMenu());
  })

  DEFINE_CFUNCTION(clearExternalContext, {
    auto obj = engine.unwrap<Context>(thisVal);
    obj->clear_external_context();
    return engine.undefined();
  })

public:
  EXPORT_CLASS_WITH_RAW_POINTER(Context,
                                WITHOUT_CONSTRUCTOR,
                                WITH_PROPERTIES(input, caretPos),
                                WITH_GETTERS(preedit,
                                             lastSegment,
                                             commitNotifier,
                                             selectNotifier,
                                             updateNotifier,
                                             deleteNotifier,
                                             commitHistory,
                                             externalPrecedingText,
                                             externalFollowingText),
                                WITH_FUNCTIONS(commit, 0, getCommitText, 0, clear, 0, hasMenu, 0, clearExternalContext, 0));
};
