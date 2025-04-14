#include "jsc_engine_impl.h"
#include "javascriptcore/jsc_string_raii.hpp"
#include "jsc_code_loader.h"

JscEngineImpl::JscEngineImpl() : ctx_(JSGlobalContextCreate(nullptr)) {
  exposeLogToJsConsole(ctx_);
}

JscEngineImpl::~JscEngineImpl() {
  for (auto& clazz : clazzes_) {
    auto& clazzDef = std::get<0>(clazz.second);
    JSClassRelease(clazzDef);
  }
  JSGlobalContextRelease(ctx_);
}

void JscEngineImpl::setBaseFolderPath(const char* absolutePath) {
  baseFolderPath_ = absolutePath;
}

JSValueRef JscEngineImpl::loadJsFile(const char* fileName) {
  lastException_ = nullptr;
  const auto* globalThis =
      JscCodeLoader::loadJsModuleToGlobalThis(ctx_, baseFolderPath_, fileName, &lastException_);
  if (lastException_ != nullptr) {
    logErrorStackTrace(lastException_, __FILE_NAME__, __LINE__);
    return JSValueMakeUndefined(ctx_);
  }
  return globalThis;
}

JSValueRef JscEngineImpl::eval(const char* code, const char* filename) {
  JscStringRAII jsCode = code;
  JscStringRAII filenameStr = filename;
  lastException_ = nullptr;
  JSValueRef result = JSEvaluateScript(ctx_, jsCode, nullptr, filenameStr, 0, &lastException_);
  logErrorStackTrace(lastException_, __FILE_NAME__, __LINE__);
  return result;
}

JSObjectRef JscEngineImpl::getGlobalObject() {
  return JSContextGetGlobalObject(ctx_);
}

size_t JscEngineImpl::getArrayLength(const JSValueRef& array) const {
  JSObjectRef arrayObj = JSValueToObject(ctx_, array, nullptr);
  JSValueRef lengthValue = JSObjectGetProperty(ctx_, arrayObj, JscStringRAII("length"), nullptr);
  return static_cast<size_t>(JSValueToNumber(ctx_, lengthValue, nullptr));
}

void JscEngineImpl::insertItemToArray(JSValueRef array,
                                      size_t index,
                                      const JSValueRef& value) const {
  JSObjectRef arrayObj = JSValueToObject(ctx_, array, nullptr);
  JSObjectSetPropertyAtIndex(ctx_, arrayObj, index, value, nullptr);
}

JSValueRef JscEngineImpl::getArrayItem(const JSValueRef& array, size_t index) const {
  JSObjectRef arrayObj = JSValueToObject(ctx_, array, nullptr);
  return JSObjectGetPropertyAtIndex(ctx_, arrayObj, index, nullptr);
}

JSValueRef JscEngineImpl::getObjectProperty(const JSObjectRef& obj,
                                            const char* propertyName) const {
  return JSObjectGetProperty(ctx_, obj, JscStringRAII(propertyName), nullptr);
}

int JscEngineImpl::setObjectProperty(const JSObjectRef& obj,
                                     const char* propertyName,
                                     const JSValueRef& value) {
  JSObjectSetProperty(ctx_, obj, JscStringRAII(propertyName), value, kJSPropertyAttributeNone,
                      nullptr);
  return 0;
}

int JscEngineImpl::setObjectFunction(JSObjectRef obj,
                                     const char* functionName,
                                     JSObjectCallAsFunctionCallback cppFunction,
                                     int expectingArgc) {
  JscStringRAII funcNameStr = functionName;
  JSObjectRef func = JSObjectMakeFunctionWithCallback(ctx_, funcNameStr, cppFunction);
  JSObjectSetProperty(ctx_, obj, funcNameStr, func, kJSPropertyAttributeNone, nullptr);
  return 0;
}

JSValueRef JscEngineImpl::callFunction(JSObjectRef func,
                                       JSObjectRef thisArg,
                                       int argc,
                                       JSValueRef* argv) {
  lastException_ = nullptr;
  auto* thisVal = JSValueIsUndefined(ctx_, thisArg) ? getGlobalObject() : thisArg;
  JSValueRef result = JSObjectCallAsFunction(ctx_, func, thisVal, argc, argv, &lastException_);
  logErrorStackTrace(lastException_, __FILE_NAME__, __LINE__);
  return result;
}

JSObjectRef JscEngineImpl::newClassInstance(const JSObjectRef& clazz, int argc, JSValueRef* argv) {
  lastException_ = nullptr;
  JSObjectRef result = JSObjectCallAsConstructor(ctx_, clazz, argc, argv, &lastException_);
  if (lastException_ != nullptr) {
    logErrorStackTrace(lastException_, __FILE_NAME__, __LINE__);
    return nullptr;
  }
  return result;
}

JSValueRef JscEngineImpl::getJsClassHavingMethod(const JSValueRef& module,
                                                 const char* methodName) const {
  JSObjectRef globalObj = JSContextGetGlobalObject(ctx_);
  JSPropertyNameArrayRef propertyNames = JSObjectCopyPropertyNames(ctx_, globalObj);
  size_t count = JSPropertyNameArrayGetCount(propertyNames);

  for (size_t i = count - 1; i >= 0; i--) {
    JSStringRef propertyName = JSPropertyNameArrayGetNameAtIndex(propertyNames, i);
    JSValueRef value = JSObjectGetProperty(ctx_, globalObj, propertyName, nullptr);

    if (JSValueIsObject(ctx_, value)) {
      JSObjectRef obj = JSValueToObject(ctx_, value, nullptr);
      JSValueRef prototype = JSObjectGetProperty(ctx_, obj, JscStringRAII("prototype"), nullptr);

      if (!JSValueIsUndefined(ctx_, prototype) && JSValueIsObject(ctx_, prototype)) {
        JSObjectRef prototypeObj = JSValueToObject(ctx_, prototype, nullptr);
        JSValueRef method =
            JSObjectGetProperty(ctx_, prototypeObj, JscStringRAII(methodName), nullptr);

        if (!JSValueIsUndefined(ctx_, method) && JSValueIsObject(ctx_, method)) {
          JSPropertyNameArrayRelease(propertyNames);
          return value;
        }
      }
    }
  }

  JSPropertyNameArrayRelease(propertyNames);
  return JSValueMakeUndefined(ctx_);
}

JSObjectRef JscEngineImpl::getMethodOfClassOrInstance(JSObjectRef jsClass,
                                                      JSObjectRef instance,
                                                      const char* methodName) {
  lastException_ = nullptr;
  JSValueRef method =
      JSObjectGetProperty(ctx_, instance, JscStringRAII(methodName), &lastException_);
  logErrorStackTrace(lastException_, __FILE_NAME__, __LINE__);
  return JSValueToObject(ctx_, method, nullptr);
}

void JscEngineImpl::logErrorStackTrace(const JSValueRef& exception,
                                       const char* file,
                                       int line) const {
  if (exception == nullptr) {
    return;
  }
  JscStringRAII exceptionStr = JSValueToStringCopy(ctx_, exception, nullptr);
  size_t bufferSize = JSStringGetMaximumUTF8CStringSize(exceptionStr);
  std::vector<char> buffer(bufferSize);
  JSStringGetUTF8CString(exceptionStr, buffer.data(), bufferSize);
  LOG(ERROR) << "[qjs] JS exception at " << file << ':' << line << " => " << buffer.data();
}

std::string JscEngineImpl::toStdString(const JSValueRef& value) const {
  if ((ctx_ == nullptr) || (value == nullptr)) {
    return {};
  }

  JSStringRef jsString = JSValueToStringCopy(ctx_, value, nullptr);
  if (jsString == nullptr) {
    return {};
  }

  size_t maxBufferSize = JSStringGetMaximumUTF8CStringSize(jsString);
  std::string result;
  result.resize(maxBufferSize);

  size_t actualSize = JSStringGetUTF8CString(jsString, result.data(), maxBufferSize);
  result.resize(actualSize > 0 ? actualSize - 1 : 0);  // Remove null terminator if present

  JSStringRelease(jsString);
  return result;
}

void JscEngineImpl::registerType(const char* typeName,
                                 JSClassRef& jsClass,
                                 JSObjectCallAsConstructorCallback constructor,
                                 TypeMap<JSValueRef>::FinalizerFunctionPionterType finalizer,
                                 JSStaticFunction* functions,
                                 int numFunctions,
                                 JSStaticValue* properties,
                                 int numProperties,
                                 JSStaticValue* getters,
                                 int numGetters) {
  if (clazzes_.find(typeName) != clazzes_.end()) {
    DLOG(INFO) << "[jsc] type: " << typeName << " has already been registered.";
    return;
  }
  DLOG(INFO) << "[jsc] registering type: " << typeName;

  std::vector<JSStaticValue> staticValues;
  staticValues.reserve(numProperties + numGetters + 1);
  for (int i = 0; i < numProperties; i++) {
    staticValues.push_back(properties[i]);
  }
  for (int i = 0; i < numGetters; i++) {
    staticValues.push_back(getters[i]);
  }
  staticValues.push_back({nullptr, nullptr, nullptr, 0});

  JSClassDefinition classDef = {.version = 0,
                                .attributes = kJSClassAttributeNone,
                                .className = typeName,
                                .parentClass = nullptr,
                                .staticValues = staticValues.data(),
                                .staticFunctions = functions,
                                .initialize = nullptr,
                                .finalize = finalizer,
                                .hasProperty = nullptr,
                                .getProperty = nullptr,
                                .setProperty = nullptr,
                                .deleteProperty = nullptr,
                                .getPropertyNames = nullptr,
                                .callAsFunction = nullptr,
                                .callAsConstructor = constructor,
                                .hasInstance = nullptr,
                                .convertToType = nullptr};

  DLOG(INFO) << "[jsc] registering type: " << typeName << " with " << (staticValues.size() - 1)
             << " properties and " << numFunctions << " functions";

  jsClass = JSClassCreate(&classDef);
  clazzes_[typeName] = std::make_tuple(jsClass, classDef, staticValues);

  // Add the constructor to the global object
  JSObjectRef globalObj = JSContextGetGlobalObject(ctx_);
  JSObjectRef constructorObj = JSObjectMake(ctx_, jsClass, nullptr);
  JSObjectSetProperty(ctx_, globalObj, JscStringRAII(typeName), constructorObj,
                      kJSPropertyAttributeNone, nullptr);
}

bool JscEngineImpl::isTypeRegistered(const std::string& typeName) const {
  if (clazzes_.find(typeName) == clazzes_.end()) {
    LOG(ERROR) << "type: " << typeName << " has not been registered.";
    return false;
  }
  return true;
}

const JSClassRef& JscEngineImpl::getRegisteredClass(const std::string& typeName) const {
  if (!isTypeRegistered(typeName)) {
    throw std::runtime_error("type: " + typeName + " has not been registered.");
  }
  return std::get<0>(clazzes_.at(typeName));
}

void JscEngineImpl::exposeLogToJsConsole(JSContextRef ctx) {
  JSObjectRef globalObject = JSContextGetGlobalObject(ctx);

  // Create console object
  JSObjectRef consoleObj = JSObjectMake(ctx, nullptr, nullptr);

  // Create log function
  JscStringRAII logStr = "log";
  JSObjectRef logFunc = JSObjectMakeFunctionWithCallback(ctx, logStr, jsLog);

  // Create error function
  JscStringRAII errorStr = "error";
  JSObjectRef errorFunc = JSObjectMakeFunctionWithCallback(ctx, errorStr, jsError);

  // Add functions to console object
  JSObjectSetProperty(ctx, consoleObj, logStr, logFunc, kJSPropertyAttributeNone, nullptr);
  JSObjectSetProperty(ctx, consoleObj, errorStr, errorFunc, kJSPropertyAttributeNone, nullptr);

  // Add console object to global scope
  JSObjectSetProperty(ctx, globalObject, JscStringRAII("console"), consoleObj,
                      kJSPropertyAttributeNone, nullptr);
}

static std::string processJsArguments(JSContextRef ctx,
                                      size_t argumentCount,
                                      const JSValueRef arguments[],
                                      JSValueRef* exception) {
  std::stringstream ss;
  for (size_t i = 0; i < argumentCount; i++) {
    JscStringRAII strRef = JSValueToStringCopy(ctx, arguments[i], exception);
    size_t bufferSize = JSStringGetMaximumUTF8CStringSize(strRef);
    char* buffer = new char[bufferSize];
    JSStringGetUTF8CString(strRef, buffer, bufferSize);
    ss << buffer;
    if (i < argumentCount - 1) {
      ss << " ";
    }
    delete[] buffer;
  }
  return ss.str();
}

JSValueRef JscEngineImpl::jsLog(JSContextRef ctx,
                                JSObjectRef function,
                                JSObjectRef thisObject,
                                size_t argumentCount,
                                const JSValueRef arguments[],
                                JSValueRef* exception) {
  std::string message = processJsArguments(ctx, argumentCount, arguments, exception);
  google::LogMessage("$jsc$", 0, google::GLOG_INFO).stream() << message;
  return JSValueMakeUndefined(ctx);
}

JSValueRef JscEngineImpl::jsError(JSContextRef ctx,
                                  JSObjectRef function,
                                  JSObjectRef thisObject,
                                  size_t argumentCount,
                                  const JSValueRef arguments[],
                                  JSValueRef* exception) {
  std::string message = processJsArguments(ctx, argumentCount, arguments, exception);
  google::LogMessage("$jsc$", 0, google::GLOG_ERROR).stream() << message;
  return JSValueMakeUndefined(ctx);
}
