#pragma once

#include <glog/logging.h>
#include <quickjs.h>
#include <string>

class TypeConverter {
public:
  static JSValue toJsString(JSContext* ctx, const char* str) { return JS_NewString(ctx, str); }

  static JSValue toJsString(JSContext* ctx, const std::string& str) {
    return JS_NewString(ctx, str.c_str());
  }

  static std::string toStdString(JSContext* ctx, const JSValue& value) {
    const char* str = JS_ToCString(ctx, value);
    if (!str)
      return std::string();
    std::string ret(str);
    JS_FreeCString(ctx, str);
    return ret;
  }

  static JSValue toJsNumber(JSContext* ctx, double value) { return JS_NewFloat64(ctx, value); }

  static JSValue toJsNumber(JSContext* ctx, int64_t value) { return JS_NewInt64(ctx, value); }

  static double toDouble(JSContext* ctx, const JSValue& value) {
    double ret = 0;
    JS_ToFloat64(ctx, &ret, value);
    return ret;
  }

  static uint32_t toUint32(JSContext* ctx, const JSValue& value) {
    uint32_t ret = 0;
    JS_ToUint32(ctx, &ret, value);
    return ret;
  }
};

class ErrorHandler {
public:
  static void logErrorStackTrace(JSContext* ctx,
                                 const JSValue& exception,
                                 const char* file,
                                 int line) {
    JSValue actualException = JS_GetException(ctx);
    std::string message = TypeConverter::toStdString(ctx, actualException);
    LOG(ERROR) << "[qjs] JS exception at " << file << ':' << line << " => " << message;

    JSValue stack = JS_GetPropertyStr(ctx, actualException, "stack");
    std::string stackTrace = TypeConverter::toStdString(ctx, stack);
    if (stackTrace.empty()) {
      LOG(ERROR) << "[qjs] JS stack trace is null.";
    } else {
      LOG(ERROR) << "[qjs] JS stack trace: " << stackTrace;
    }

    JS_FreeValue(ctx, stack);
    JS_FreeValue(ctx, exception);
  }

  static JSValue throwError(JSContext* ctx, JsErrorType errorType, const char* format, ...) {
    JSValue ret;
    va_list args;
    va_start(args, format);

    switch (errorType) {
      case JsErrorType::SYNTAX:
      case JsErrorType::EVAL:
        ret = JS_ThrowSyntaxError(ctx, format, args);
        break;
      case JsErrorType::RANGE:
        ret = JS_ThrowRangeError(ctx, format, args);
        break;
      case JsErrorType::REFERENCE:
        ret = JS_ThrowReferenceError(ctx, format, args);
        break;
      case JsErrorType::TYPE:
        ret = JS_ThrowTypeError(ctx, format, args);
        break;
      case JsErrorType::GENERIC:
      case JsErrorType::UNKNOWN:
        ret = JS_ThrowPlainError(ctx, format, args);
        break;
    }

    va_end(args);
    return ret;
  }
};
