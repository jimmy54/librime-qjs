#ifndef RIME_QJS_CONTEXT_H_
#define RIME_QJS_CONTEXT_H_

#include "qjs_macros.h"
#include "qjs_type_registry.h"
#include <rime/context.h>

DECLARE_JS_CLASS_WITH_RAW_POINTER(Context,
  DECLARE_PROPERTIES(input, caretPos, preedit, lastSegment),
  DECLARE_FUNCTIONS(commit, get_commit_text, clear, push_input, pop_input,
    has_menu, is_composing,
    clear_previous_segment, reopen_previous_segment,
    clear_non_confirmed_composition, refresh_non_confirmed_composition,
    set_option, get_option, clear_transient_options,
    set_property, get_property)
)

#endif // RIME_QJS_CONTEXT_H_
