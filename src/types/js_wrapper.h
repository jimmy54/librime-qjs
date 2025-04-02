#pragma once

#include "engines/type_map.h"

template <typename T_JS_VALUE>
class JsWrapperBase {
public:
  virtual typename TypeMap<JSValue>::ConstructorFunctionPionterType getConstructor() {
    return nullptr;
  }
  int getConstructorArgc() { return constructorArgc_; }

  virtual typename TypeMap<JSValue>::FinalizerFunctionPionterType getFinalizer() { return nullptr; }

  virtual typename TypeMap<JSValue>::ExposeFunctionType* getFunctions() { return nullptr; }
  int getFunctionsCount() { return functionCount_; }

  virtual typename TypeMap<JSValue>::ExposePropertyType* getProperties() { return nullptr; }
  int getPropertiesCount() { return propertyCount_; }

  virtual typename TypeMap<JSValue>::ExposePropertyType* getGetters() { return nullptr; }
  int getGettersCount() { return getterCount_; }

#ifdef __APPLE__
  virtual typename TypeMap<JSValueRef>::ConstructorFunctionPionterType getConstructorJsc() {
    return nullptr;
  }

  virtual typename TypeMap<JSValueRef>::FinalizerFunctionPionterType getFinalizerJsc() {
    return nullptr;
  }

  virtual typename TypeMap<JSValueRef>::ExposeFunctionType* getFunctionsJsc() { return nullptr; }

  virtual typename TypeMap<JSValueRef>::ExposePropertyType* getPropertiesJsc() { return nullptr; }

  virtual typename TypeMap<JSValueRef>::ExposePropertyType* getGettersJsc() { return nullptr; }
#endif

protected:
  void setFunctionCount(int count) { functionCount_ = count; }
  void setPropertyCount(int count) { propertyCount_ = count; }
  void setGetterCount(int count) { getterCount_ = count; }
  void setConstructorArgc(int count) { constructorArgc_ = count; }

private:
  int functionCount_ = 0;
  int propertyCount_ = 0;
  int getterCount_ = 0;
  int constructorArgc_ = 0;
};

template <typename T_RIME_TYPE, typename T_JS_VALUE>
class JsWrapper {
public:
  static const char* getTypeName() { return "Unknown"; }
};
