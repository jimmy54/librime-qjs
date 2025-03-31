#pragma once

#include <rime/menu.h>
#include <rime/segmentation.h>
#include "engines/engine_manager.h"
#include "engines/js_macros.h"
#include "js_wrapper.h"

using namespace rime;

template <typename T_JS_VALUE>
class JsWrapper<rime::Segment, T_JS_VALUE> : public JsWrapperBase<T_JS_VALUE> {
  DEFINE_GETTER(Segment, selectedIndex, engine.toJsInt(obj->selected_index))
  DEFINE_SETTER(Segment, selectedIndex, engine.toInt, obj->selected_index = value)

  DEFINE_GETTER(Segment, prompt, engine.toJsString(obj->prompt.c_str()))
  DEFINE_STRING_SETTER(Segment, prompt, obj->prompt = str)

  DEFINE_GETTER(Segment, start, engine.toJsInt(obj->start))
  DEFINE_GETTER(Segment, end, engine.toJsInt(obj->end))
  DEFINE_GETTER(Segment,
                selectedCandidate,
                engine.wrapShared<Candidate>(obj->GetSelectedCandidate()))
  DEFINE_GETTER(Segment, candidateSize, engine.toJsInt(obj->menu->candidate_count()))

  DEFINE_CFUNCTION_ARGC(getCandidateAt, 1, {
    auto obj = engine.unwrap<Segment>(thisVal);
    int32_t index = engine.toInt(argv[0]);
    if (index < 0 || size_t(index) >= obj->menu->candidate_count()) {
      return engine.null();
    }
    return engine.wrapShared(obj->menu->GetCandidateAt(index));
  })

  DEFINE_CFUNCTION_ARGC(hasTag, 1, {
    auto obj = engine.unwrap<Segment>(thisVal);
    std::string tag = engine.toStdString(argv[0]);
    return engine.toJsBool(obj->HasTag(tag));
  })

public:
  static const char* getTypeName() { return "Segment"; }

  typename TypeMap<T_JS_VALUE>::ExposePropertyType* getProperties() override {
    auto& engine = getJsEngine<T_JS_VALUE>();
    static typename TypeMap<T_JS_VALUE>::ExposePropertyType properties[] = {
        engine.defineProperty("selectedIndex", get_selectedIndex, set_selectedIndex),
        engine.defineProperty("prompt", get_prompt, set_prompt),
    };
    this->setPropertyCount(countof(properties));

    return properties;
  }

  typename TypeMap<T_JS_VALUE>::ExposePropertyType* getGetters() override {
    auto& engine = getJsEngine<T_JS_VALUE>();
    static typename TypeMap<T_JS_VALUE>::ExposePropertyType getters[] = {
        engine.defineProperty("start", get_start, nullptr),
        engine.defineProperty("end", get_end, nullptr),
        engine.defineProperty("selectedCandidate", get_selectedCandidate, nullptr),
        engine.defineProperty("candidateSize", get_candidateSize, nullptr),
    };
    this->setGetterCount(countof(getters));

    return getters;
  }

  typename TypeMap<T_JS_VALUE>::ExposeFunctionType* getFunctions() override {
    auto& engine = getJsEngine<T_JS_VALUE>();
    static typename TypeMap<T_JS_VALUE>::ExposeFunctionType functions[] = {
        engine.defineFunction("getCandidateAt", 1, getCandidateAt),
        engine.defineFunction("hasTag", 1, hasTag),
    };
    this->setFunctionCount(countof(functions));
    return functions;
  }
};
