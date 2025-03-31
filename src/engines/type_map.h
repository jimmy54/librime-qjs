#pragma once

#include "quickjs.h"

template <typename T>
struct TypeMap {  // Default mapping
  using RuntimeType = void;
  using ContextType = void;
  using FunctionPionterType = void*;
  using FinalizerFunctionPionterType = void*;
  using ExposeFunctionType = void;
  using ExposePropertyType = void;
  using GetterFunctionType = void;
  using SetterFunctionType = void;

  const char* engineName = "unknown";  // Engine name for logging and debugging
};

// Specializations
template <>
struct TypeMap<JSValue> {
  using RuntimeType = JSRuntime;
  using ContextType = JSContext*;
  using FunctionPionterType = JSCFunction*;
  using FinalizerFunctionPionterType = JSClassFinalizer*;
  using ExposeFunctionType = const JSCFunctionListEntry;
  using ExposePropertyType = const JSCFunctionListEntry;
  using GetterFunctionType = JSValue (*)(JSContext*, JSValueConst);
  using SetterFunctionType = JSValue (*)(JSContext*, JSValueConst, JSValue);
  const char* engineName = "QuickJS-NG";
};
