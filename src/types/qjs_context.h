#pragma once

#include <rime/context.h>
#include <memory>
#include "engines/engine_manager.h"
#include "engines/js_macros.h"
#include "js_wrapper.h"

using namespace rime;

template <typename T_JS_VALUE>
class JsWrapper<rime::Context, T_JS_VALUE> : public JsWrapperBase<T_JS_VALUE> {
  DEFINE_GETTER(Context, input, engine.toJsString(obj->input()))
  DEFINE_GETTER(Context, caretPos, engine.toJsInt(obj->caret_pos()))

  DEFINE_STRING_SETTER(Context, input, obj->set_input(str);)
  DEFINE_SETTER(Context, caretPos, engine.toInt, obj->set_caret_pos(value))

  DEFINE_GETTER(Context, preedit, engine.wrapShared(std::make_shared<Preedit>(obj->GetPreedit())))

  static JSValue getLastSegment(JSContext* ctx, JSValueConst thisVal) {
    auto& engine = getJsEngine<JSValue>();
    if (auto* obj = engine.unwrap<Context>(thisVal)) {
      if (obj->composition().empty()) {
        // The composition could be empty when there is not a menu listing.
        // In the javascript plugins, it should check `context.hasMenu()` before fetching the segment.
        return engine.null();
      }

      // must be set as reference [Segment&] here, otherwise fetching its prompt would crash the program
      Segment& segment = obj->composition().back();
      return engine.wrap<Segment>(&segment);
    }
    return engine.undefined();
  }

  DEFINE_CFUNCTION(commit, {
    auto obj = engine.unwrap<Context>(thisVal);
    obj->Commit();
    return engine.undefined();
  })

  DEFINE_CFUNCTION(getCommitText, {
    auto obj = engine.unwrap<Context>(thisVal);
    return engine.toJsString(obj->GetCommitText().c_str());
  })

  DEFINE_CFUNCTION(clear, {
    auto obj = engine.unwrap<Context>(thisVal);
    obj->Clear();
    return engine.undefined();
  })

  DEFINE_CFUNCTION(hasMenu, {
    auto obj = engine.unwrap<Context>(thisVal);
    return engine.toJsBool(obj->HasMenu());
  })

public:
  static const char* getTypeName() { return "Context"; }

  typename TypeMap<T_JS_VALUE>::ExposePropertyType* getProperties() override {
    auto& engine = getJsEngine<T_JS_VALUE>();
    static typename TypeMap<T_JS_VALUE>::ExposePropertyType properties[] = {
        engine.defineProperty("input", get_input, set_input),
        engine.defineProperty("caretPos", get_caretPos, set_caretPos),
    };
    this->setPropertyCount(countof(properties));

    return properties;
  }

  typename TypeMap<T_JS_VALUE>::ExposePropertyType* getGetters() override {
    auto& engine = getJsEngine<T_JS_VALUE>();
    static typename TypeMap<T_JS_VALUE>::ExposePropertyType getters[] = {
        engine.defineProperty("preedit", get_preedit, nullptr),
        engine.defineProperty("lastSegment", getLastSegment, nullptr),
    };
    this->setGetterCount(countof(getters));

    return getters;
  }

  typename TypeMap<T_JS_VALUE>::ExposeFunctionType* getFunctions() override {
    auto& engine = getJsEngine<T_JS_VALUE>();
    static typename TypeMap<T_JS_VALUE>::ExposeFunctionType functions[] = {
        engine.defineFunction("commit", 0, commit),
        engine.defineFunction("getCommitText", 0, getCommitText),
        engine.defineFunction("clear", 0, clear),
        engine.defineFunction("hasMenu", 0, hasMenu),
    };
    this->setFunctionCount(countof(functions));
    return functions;
  }
};
