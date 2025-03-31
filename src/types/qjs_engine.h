#pragma once

#include <rime/engine.h>
#include "js_wrapper.h"

#include <rime/key_event.h>

#include "engines/engine_manager.h"
#include "engines/js_macros.h"

using namespace rime;

template <typename T_JS_VALUE>
class JsWrapper<rime::Engine, T_JS_VALUE> : public JsWrapperBase<T_JS_VALUE> {
  DEFINE_GETTER(Engine, schema, engine.wrap(obj->schema()))
  DEFINE_GETTER(Engine, context, engine.wrap(obj->context()))
  DEFINE_GETTER(Engine, activeEngine, engine.wrap(obj->active_engine()))

  DEFINE_CFUNCTION_ARGC(commitText, 1, {
    std::string text = engine.toStdString(argv[0]);
    auto* obj = engine.unwrap<Engine>(thisVal);
    obj->CommitText(text);
    return engine.undefined();
  })

  DEFINE_CFUNCTION_ARGC(applySchema, 1, {
    auto schema = engine.unwrap<Schema>(argv[0]);
    if (!schema) {
      return engine.jsFalse();
    }
    auto* obj = engine.unwrap<Engine>(thisVal);
    obj->ApplySchema(schema);
    return engine.jsTrue();
  })

  DEFINE_CFUNCTION_ARGC(processKey, 1, {
    std::string keyRepr = engine.toStdString(argv[0]);
    auto* obj = engine.unwrap<Engine>(thisVal);
    return engine.toJsBool(obj->ProcessKey(KeyEvent(keyRepr)));
  })

public:
  static const char* getTypeName() { return "Engine"; }

  typename TypeMap<T_JS_VALUE>::ExposePropertyType* getGetters() override {
    auto& engine = getJsEngine<T_JS_VALUE>();
    static typename TypeMap<T_JS_VALUE>::ExposePropertyType getters[] = {
        engine.defineProperty("schema", get_schema, nullptr),
        engine.defineProperty("activeEngine", get_activeEngine, nullptr),
        engine.defineProperty("context", get_context, nullptr),
    };
    this->setGetterCount(countof(getters));

    return getters;
  }

  typename TypeMap<T_JS_VALUE>::ExposeFunctionType* getFunctions() override {
    auto& engine = getJsEngine<T_JS_VALUE>();
    static typename TypeMap<T_JS_VALUE>::ExposeFunctionType functions[] = {
        engine.defineFunction("processKey", 1, processKey),
        engine.defineFunction("commitText", 1, commitText),
        engine.defineFunction("applySchema", 1, applySchema),
    };
    this->setFunctionCount(countof(functions));
    return functions;
  }
};
