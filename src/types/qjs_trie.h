#pragma once

#include <glog/logging.h>
#include "engines/engine_manager.h"
#include "engines/js_engine.h"
#include "engines/js_macros.h"
#include "js_wrapper.h"
#include "trie.h"

using namespace rime;

template <typename T_JS_VALUE>
class JsWrapper<rime::Trie, T_JS_VALUE> : public JsWrapperBase<T_JS_VALUE> {
  DEFINE_CFUNCTION(makeTrie, { return engine.wrapShared(std::make_shared<Trie>()); })

  DEFINE_CFUNCTION_ARGC(loadTextFile, 1, {
    std::string absolutePath = engine.toStdString(argv[0]);
    size_t size = 0;
    if (argc > 0) {
      size = engine.toInt(argv[1]);
    }
    auto obj = engine.unwrapShared<Trie>(thisVal);

    try {
      obj->loadTextFile(absolutePath, size);
    } catch (const std::exception& e) {
      LOG(ERROR) << "loadTextFile of " << absolutePath << " failed: " << e.what();
      return JS_ThrowPlainError(ctx, "%s", e.what());
    }

    return JS_UNDEFINED;
  })

  DEFINE_CFUNCTION_ARGC(loadBinaryFile, 1, {
    std::string absolutePath = engine.toStdString(argv[0]);
    auto obj = engine.unwrapShared<Trie>(thisVal);
    try {
      obj->loadBinaryFileMmap(absolutePath);
    } catch (const std::exception& e) {
      LOG(ERROR) << "loadBinaryFileMmap of " << absolutePath << " failed: " << e.what();
      return engine.throwError(JsErrorType::GENERIC, "%s", e.what());
    }
    return JS_UNDEFINED;
  })

  DEFINE_CFUNCTION_ARGC(saveToBinaryFile, 1, {
    std::string absolutePath = engine.toStdString(argv[0]);
    auto obj = engine.unwrapShared<Trie>(thisVal);
    try {
      obj->saveToBinaryFile(absolutePath);
    } catch (const std::exception& e) {
      LOG(ERROR) << "saveToBinaryFile of " << absolutePath << " failed: " << e.what();
      return engine.throwError(JsErrorType::GENERIC, "%s", e.what());
    }
    return JS_UNDEFINED;
  })

  DEFINE_CFUNCTION_ARGC(find, 1, {
    std::string key = engine.toStdString(argv[0]);
    auto obj = engine.unwrapShared<Trie>(thisVal);
    auto result = obj->find(key);
    return result.has_value() ? engine.toJsString(result.value().c_str()) : engine.null();
  })

  DEFINE_CFUNCTION_ARGC(prefixSearch, 1, {
    std::string prefix = engine.toStdString(argv[0]);
    auto obj = engine.unwrapShared<Trie>(thisVal);
    auto matches = obj->prefixSearch(prefix);

    auto jsArray = engine.newArray();
    for (size_t i = 0; i < matches.size(); ++i) {
      auto jsObject = engine.newObject();
      engine.setObjectProperty(jsObject, "text", engine.toJsString(matches[i].first.c_str()));
      engine.setObjectProperty(jsObject, "info", engine.toJsString(matches[i].second.c_str()));
      engine.insertItemToArray(jsArray, i, jsObject);
    }
    return jsArray;
  })

public:
  static const char* getTypeName() { return "Trie"; }

  typename TypeMap<T_JS_VALUE>::FunctionPionterType getConstructor() override { return makeTrie; }

  typename TypeMap<T_JS_VALUE>::ExposeFunctionType* getFunctions() override {
    auto& engine = getJsEngine<T_JS_VALUE>();
    static typename TypeMap<T_JS_VALUE>::ExposeFunctionType functions[] = {
        engine.defineFunction("loadTextFile", 1, loadTextFile),
        engine.defineFunction("loadBinaryFile", 1, loadBinaryFile),
        engine.defineFunction("saveToBinaryFile", 1, saveToBinaryFile),
        engine.defineFunction("find", 1, find),
        engine.defineFunction("prefixSearch", 1, prefixSearch),
    };
    this->setFunctionCount(countof(functions));
    return functions;
  }
};
