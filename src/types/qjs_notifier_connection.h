#pragma once

#include <rime/common.h>

#include "qjs_macros.h"

using NotifierConnection = rime::connection;

constexpr const char* JS_LISTENER_PROPERTY_NAME = "jsListenerFunc";

DECLARE_JS_CLASS_WITH_SHARED_POINTER(NotifierConnection)
