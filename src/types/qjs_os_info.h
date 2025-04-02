#pragma once

#include "engines/js_macros.h"
#include "js_wrapper.h"
#include "system_info.hpp"

template <typename T_JS_VALUE>
class JsWrapper<SystemInfo, T_JS_VALUE> : public JsWrapperBase<T_JS_VALUE> {
  DEFINE_GETTER(SystemInfo, name, engine.toJsString(obj->getOSName()));
  DEFINE_GETTER(SystemInfo, version, engine.toJsString(obj->getOSVersion()));
  DEFINE_GETTER(SystemInfo, architecture, engine.toJsString(obj->getArchitecture()));

public:
  static const char* getTypeName() { return "OperationSystemInfo"; }

  EXPORT_GETTERS(name, version, architecture);
};
