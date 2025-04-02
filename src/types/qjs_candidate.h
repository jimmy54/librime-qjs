#pragma once

#include <rime/candidate.h>
#include <rime/gear/translator_commons.h>
#include <memory>
#include "engines/js_exception.h"
#include "engines/js_macros.h"
#include "js_wrapper.h"

using namespace rime;

template <typename T_JS_VALUE>
class JsWrapper<rime::Candidate, T_JS_VALUE> : public JsWrapperBase<T_JS_VALUE> {
  DEFINE_GETTER_2(Candidate, text, engine.toJsString(obj->text()))
  DEFINE_GETTER_2(Candidate, comment, engine.toJsString(obj->comment()))
  DEFINE_GETTER_2(Candidate, type, engine.toJsString(obj->type()))
  DEFINE_GETTER_2(Candidate, start, engine.toJsInt(obj->start()))
  DEFINE_GETTER_2(Candidate, end, engine.toJsInt(obj->end()))
  DEFINE_GETTER_2(Candidate, quality, engine.toJsInt(obj->quality()))
  DEFINE_GETTER_2(Candidate, preedit, engine.toJsString(obj->preedit()))

  DEFINE_STRING_SETTER_2(Candidate, text, {
    if (auto simpleCandidate = dynamic_cast<rime::SimpleCandidate*>(obj.get())) {
      simpleCandidate->set_text(str);
    }
  })

  DEFINE_STRING_SETTER_2(Candidate, comment, {
    if (auto simpleCandidate = dynamic_cast<rime::SimpleCandidate*>(obj.get())) {
      simpleCandidate->set_comment(str);
    } else if (auto phrase = dynamic_cast<rime::Phrase*>(obj.get())) {
      phrase->set_comment(str);
    }
  })

  DEFINE_STRING_SETTER_2(Candidate, type, obj->set_type(str);)

  DEFINE_SETTER_2(Candidate, start, engine.toInt, obj->set_start(value))
  DEFINE_SETTER_2(Candidate, quality, engine.toDouble, obj->set_quality(value))
  DEFINE_SETTER_2(Candidate, end, engine.toInt, obj->set_end(value))

  DEFINE_STRING_SETTER_2(Candidate, preedit, {
    if (auto simpleCandidate = dynamic_cast<rime::SimpleCandidate*>(obj.get())) {
      simpleCandidate->set_preedit(str);
    } else if (auto phrase = dynamic_cast<rime::Phrase*>(obj.get())) {
      phrase->set_preedit(str);
    }
  })

public:
  static const char* getTypeName() { return "Candidate"; }

  static constexpr int MIN_ARGC_NEW_CANDIDATE = 5;

  JsWrapper<rime::Candidate, T_JS_VALUE>() { this->setConstructorArgc(MIN_ARGC_NEW_CANDIDATE); }

  EXPORT_CONSTRUCTOR(makeCandidate, {
    if (argc < MIN_ARGC_NEW_CANDIDATE) {
      throw new JsException(JsErrorType::SYNTAX, "new Candidate(...) expects 5 or 6 arguments");
    }

    auto obj = std::make_shared<rime::SimpleCandidate>();
    obj->set_type(engine.toStdString(argv[0]));
    obj->set_start(engine.toInt(argv[1]));
    obj->set_end(engine.toInt(argv[2]));
    obj->set_text(engine.toStdString(argv[3]));
    obj->set_comment(engine.toStdString(argv[4]));
    if (argc > MIN_ARGC_NEW_CANDIDATE) {
      obj->set_quality(engine.toDouble(argv[5]));
    }
    return engine.wrapShared<rime::Candidate>(obj);
  });

  EXPORT_SHARED_FINALIZER(rime::Candidate, finalizer);
  EXPORT_PROPERTIES(text, comment, type, start, end, quality, preedit);
};
