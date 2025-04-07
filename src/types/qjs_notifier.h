#pragma once

#include <rime/context.h>

#include "qjs_macros.h"

using Notifier = rime::signal<void(rime::Context* ctx)>;

DECLARE_JS_CLASS_WITH_RAW_POINTER(Notifier)
