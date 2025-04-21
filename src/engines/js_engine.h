#pragma once

#include <cstdint>
#include <string>
#include <vector>

#include "engines/js_exception.h"
#include "engines/type_map.h"

template <typename T_RIME_TYPE, typename T_JS_VALUE>
class JsWrapper;

template <typename T_JS_VALUE>
class JsEngine {
  using T_JS_OBJECT = typename TypeMap<T_JS_VALUE>::ObjectType;

public:
  void setBaseFolderPath(const char* absolutePath);

  // return the memory usaged by the js engine in bytes, -1 if not available
  int64_t getMemoryUsage();

  T_JS_OBJECT createInstanceOfModule(const char* moduleName,
                                     std::vector<T_JS_VALUE>& args,
                                     const std::string& mainFuncName);
  T_JS_VALUE loadJsFile(const char* fileName);
  T_JS_VALUE getJsClassHavingMethod(const T_JS_VALUE& container, const char* methodName);
  T_JS_VALUE getMethodOfClassOrInstance(T_JS_VALUE jsClass,
                                        T_JS_VALUE instance,
                                        const char* methodName);

  T_JS_VALUE null();
  T_JS_VALUE undefined();
  T_JS_VALUE jsTrue();
  T_JS_VALUE jsFalse();

  T_JS_VALUE newArray();
  size_t getArrayLength(const T_JS_VALUE& array);
  void insertItemToArray(T_JS_VALUE array, size_t index, const T_JS_VALUE& value);
  T_JS_VALUE getArrayItem(const T_JS_VALUE& array, size_t index);

  T_JS_OBJECT newObject();
  T_JS_VALUE getObjectProperty(const T_JS_OBJECT& obj, const char* propertyName);
  int setObjectProperty(const T_JS_OBJECT& obj, const char* propertyName, const T_JS_VALUE& value);

  using ExposeFunction = T_JS_VALUE (*)(typename TypeMap<T_JS_VALUE>::ContextType ctx,
                                        T_JS_VALUE thisVal,
                                        int argc,
                                        T_JS_VALUE* argv);
  int setObjectFunction(T_JS_OBJECT obj,
                        const char* functionName,
                        ExposeFunction cppFunction,
                        int expectingArgc);

  T_JS_OBJECT toObject(const T_JS_VALUE& value);
  T_JS_VALUE toJsString(const char* str);
  T_JS_VALUE toJsString(const std::string& str);
  std::string toStdString(const T_JS_VALUE& value);

  T_JS_VALUE toJsBool(bool value);
  bool toBool(T_JS_VALUE value);

  T_JS_VALUE toJsInt(size_t value);
  size_t toInt(const T_JS_VALUE& value);

  T_JS_VALUE toJsDouble(bool value);
  double toDouble(const T_JS_VALUE& value);

  T_JS_VALUE callFunction(T_JS_VALUE func, T_JS_VALUE thisArg, int argc, T_JS_VALUE* argv);
  T_JS_VALUE newClassInstance(T_JS_VALUE clazz, int argc, T_JS_VALUE* argv);

  bool isFunction(const T_JS_VALUE& value);
  bool isArray(const T_JS_VALUE& value);
  bool isObject(const T_JS_VALUE& value);
  bool isNull(const T_JS_VALUE& value);
  bool isUndefined(const T_JS_VALUE& value);
  bool isException(const T_JS_VALUE& value);
  T_JS_OBJECT throwError(JsErrorType errorType, const std::string& message);

  T_JS_VALUE duplicateValue(const T_JS_VALUE& value);

  template <typename... Args>
  void freeValue(const Args&... args) const;

  template <typename... Args>
  void protectFromGC(const Args&... args) const;

  template <typename... Args>
  void unprotectFromGC(const Args&... args) const;

  template <typename T_RIME_TYPE>
  void registerType();

  template <typename T>
  T_JS_OBJECT wrap(T* ptrValue);
  template <typename T>
  T* unwrap(const T_JS_VALUE& value);

  template <typename T>
  T_JS_OBJECT wrapShared(std::shared_ptr<T> value);  //pass by value to copy the shared_ptr
  template <typename T>
  std::shared_ptr<T> unwrapShared(const T_JS_VALUE& value);

  virtual T_JS_VALUE eval(const char* code, const char* filename = "<eval>");

  virtual T_JS_OBJECT getGlobalObject();
};

static std::string formatString(const char* format, ...) {
  va_list args;
  // NOLINTBEGIN(cppcoreguidelines-pro-bounds-array-to-pointer-decay)
  va_start(args, format);
  constexpr size_t K_MAX_BUFFER_SIZE = 1024;
  char buffer[K_MAX_BUFFER_SIZE];
  char* ptr = static_cast<char*>(buffer);
  vsnprintf(ptr, sizeof(buffer), format, args);
  va_end(args);
  // NOLINTEND(cppcoreguidelines-pro-bounds-array-to-pointer-decay)
  return {ptr};
}
