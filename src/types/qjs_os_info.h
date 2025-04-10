#pragma once

#include "engines/js_macros.h"
#include "js_wrapper.h"
#include "misc/system_info.hpp"

template <typename T_JS_VALUE>
class JsWrapper<SystemInfo, T_JS_VALUE> {
  DEFINE_GETTER(SystemInfo, name, engine.toJsString(obj->getOSName()));
  DEFINE_GETTER(SystemInfo, version, engine.toJsString(obj->getOSVersion()));
  DEFINE_GETTER(SystemInfo, architecture, engine.toJsString(obj->getArchitecture()));

public:
  EXPORT_CLASS_WITH_RAW_POINTER(SystemInfo,
                                WITHOUT_CONSTRUCTOR,
                                WITHOUT_PROPERTIES,
                                WITH_GETTERS(name, version, architecture),
                                WITHOUT_FUNCTIONS);
};
