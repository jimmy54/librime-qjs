#pragma once

#include "quickjs.h"

template <typename T>
struct TypeMap {  // Default mapping
  using RuntimeType = void*;
  using ContextType = void*;
  using ObjectType = void*;
  using FunctionPionterType = void*;
  using ConstructorFunctionPionterType = void*;
  using ExposeFunctionType = void*;
  using ExposePropertyType = void*;
  using GetterFunctionType = void*;
  using SetterFunctionType = void*;

  inline static const char* engineName = "Unsupported";  // Engine name for logging and debugging
};

// Specializations
template <>
struct TypeMap<JSValue> {
  using RuntimeType = JSRuntime;
  using ContextType = JSContext*;
  using ObjectType = JSValue;
  using FunctionPionterType = JSCFunction*;
  using ConstructorFunctionPionterType = JSCFunction*;
  using FinalizerFunctionPionterType = JSClassFinalizer*;
  using ExposeFunctionType = const JSCFunctionListEntry;
  using ExposePropertyType = const JSCFunctionListEntry;
  using GetterFunctionType = JSValue (*)(JSContext*, JSValueConst);
  using SetterFunctionType = JSValue (*)(JSContext*, JSValueConst, JSValue);
  inline static const char* engineName = "QuickJS-NG";
};

#ifdef __APPLE__

#include <JavaScriptCore/JavaScript.h>
#include <JavaScriptCore/JavaScriptCore.h>

// Specializations
template <>
struct TypeMap<JSValueRef> {
  using RuntimeType = void*;
  using ContextType = JSContextRef;
  using ObjectType = JSObjectRef;
  using FunctionPionterType = JSObjectCallAsFunctionCallback;
  using ConstructorFunctionPionterType = JSObjectCallAsConstructorCallback;
  using FinalizerFunctionPionterType = void (*)(JSObjectRef);
  using ExposeFunctionType = JSStaticFunction;
  using ExposePropertyType = JSStaticValue;
  using GetterFunctionType = JSValueRef (*)(JSContextRef, JSObjectRef, JSStringRef, JSValueRef*);
  using SetterFunctionType =
      bool (*)(JSContextRef, JSObjectRef, JSStringRef, JSValueRef, JSValueRef*);
  inline static const char* engineName = "JavaScriptCore";
};
#endif
