#pragma once

#include <cstddef>
#include <cstdint>
#include <string>

#include "engines/type_map.h"
#include "types/js_wrapper.h"

enum class JsErrorType : std::uint8_t {
  SYNTAX,
  RANGE,
  REFERENCE,
  TYPE,
  EVAL,
  GENERIC,
  UNKNOWN,
};

template <typename T_JS_VALUE>
class JsEngine {
public:
  typename TypeMap<T_JS_VALUE>::ContextType& getContext();

  void setBaseFolderPath(const char* absolutePath);

  T_JS_VALUE loadJsFile(const char* fileName);
  T_JS_VALUE getJsClassHavingMethod(const T_JS_VALUE& container, const char* methodName);
  T_JS_VALUE getMethodOfClass(T_JS_VALUE jsClass, const char* methodName);

  template <typename T>
  void* getOpaque(T_JS_VALUE value);
  void setOpaque(T_JS_VALUE value, void* opaque);

  T_JS_VALUE null();
  T_JS_VALUE undefined();
  T_JS_VALUE jsTrue();
  T_JS_VALUE jsFalse();

  T_JS_VALUE newArray();
  size_t getArrayLength(const T_JS_VALUE& array);
  int insertItemToArray(T_JS_VALUE array, size_t index, const T_JS_VALUE& value);
  T_JS_VALUE getArrayItem(const T_JS_VALUE& array, size_t index);

  T_JS_VALUE newObject();
  T_JS_VALUE getObjectProperty(const T_JS_VALUE& obj, const char* propertyName);
  int setObjectProperty(T_JS_VALUE obj, const char* propertyName, const T_JS_VALUE& value);

  using ExposeFunction = T_JS_VALUE (*)(typename TypeMap<T_JS_VALUE>::ContextType ctx,
                                        T_JS_VALUE thisVal,
                                        int argc,
                                        T_JS_VALUE* argv);
  int setObjectFunction(T_JS_VALUE obj,
                        const char* functionName,
                        ExposeFunction cppFunction,
                        int expectingArgc);

  T_JS_VALUE toJsString(const char* str);
  T_JS_VALUE toJsString(const std::string& str);
  std::string toStdString(const T_JS_VALUE& value);

  T_JS_VALUE toJsBool(bool value);

  T_JS_VALUE toJsInt(size_t value);
  size_t toInt(const T_JS_VALUE& value);

  T_JS_VALUE toJsDouble(bool value);
  double toDouble(const T_JS_VALUE& value);

  T_JS_VALUE callFunction(T_JS_VALUE func, T_JS_VALUE thisArg, int argc, T_JS_VALUE* argv);
  T_JS_VALUE callConstructor(T_JS_VALUE func, int argc, T_JS_VALUE* argv);

  bool isObject(const T_JS_VALUE& value);
  bool isNull(const T_JS_VALUE& value);
  bool isUndefined(const T_JS_VALUE& value);
  bool isException(const T_JS_VALUE& value);
  T_JS_VALUE throwError(JsErrorType errorType, const char* format, ...);

  void logErrorStackTrace(const JsErrorType& exception);

  void freeValue(const T_JS_VALUE& value);

  template <typename T_RIME_TYPE>
  void registerType(JsWrapper<T_RIME_TYPE, T_JS_VALUE>& wrapper);

  typename TypeMap<T_JS_VALUE>::ExposeFunctionType defineFunction(const char* name,
                                                                  int avgc,
                                                                  ExposeFunction func);

  typename TypeMap<T_JS_VALUE>::ExposePropertyType defineProperty(
      const char* name,
      typename TypeMap<T_JS_VALUE>::GetterFunctionType getter,
      typename TypeMap<T_JS_VALUE>::SetterFunctionType setter);

  template <typename T>
  T_JS_VALUE wrap(T* ptrValue);
  template <typename T>
  T* unwrap(const T_JS_VALUE& value);

  template <typename T>
  T_JS_VALUE wrapShared(const std::shared_ptr<T>& value);
  template <typename T>
  std::shared_ptr<T> unwrapShared(const T_JS_VALUE& value);
};
