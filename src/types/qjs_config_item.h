#pragma once

#include <rime/config/config_types.h>
#include "engines/engine_manager.h"
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
    return JS_NewString(ctx, strType);
  })

public:
  static const char* getTypeName() { return "ConfigItem"; }

  typename TypeMap<T_JS_VALUE>::ExposeFunctionType* getFunctions() override {
    auto& engine = getJsEngine<T_JS_VALUE>();
    static typename TypeMap<T_JS_VALUE>::ExposeFunctionType functions[] = {
        engine.defineFunction("getType", 0, getType),
    };
    this->setFunctionCount(countof(functions));
    return functions;
  }
};
