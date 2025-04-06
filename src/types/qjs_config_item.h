#pragma once

#include <rime/config/config_types.h>
#include "engines/js_macros.h"
#include "js_wrapper.h"

template <typename T_JS_VALUE>
class JsWrapper<rime::ConfigItem, T_JS_VALUE> : public JsWrapperBase<T_JS_VALUE> {
  DEFINE_CFUNCTION(getType, {
    auto obj = engine.unwrapShared<rime::ConfigItem>(thisVal);
    const char* strType;
    switch (obj->type()) {
      case rime::ConfigItem::kNull:
        strType = "null";
        break;
      case rime::ConfigItem::kScalar:
        strType = "scalar";
        break;
      case rime::ConfigItem::kList:
        strType = "list";
        break;
      case rime::ConfigItem::kMap:
        strType = "map";
        break;
      default:
        strType = "unknown";
    }
    return engine.toJsBool(strType);
  })

public:
  EXPORT_CLASS(ConfigItem);

  EXPORT_FINALIZER(rime::ConfigItem, finalizer);
  EXPORT_FUNCTIONS(getType, 0);
};
