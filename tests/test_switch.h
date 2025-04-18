#pragma once

#include <quickjs.h>
#include "types/qjs_types.h"

#ifdef _ENABLE_JAVASCRIPTCORE

#include <JavaScriptCore/JavaScript.h>

#define SETUP_JS_ENGINES(testSuite)                      \
  using JsTypes = ::testing::Types<JSValue, JSValueRef>; \
  TYPED_TEST_SUITE(testSuite, JsTypes);

#else

#define SETUP_JS_ENGINES(testSuite)          \
  using JsTypes = ::testing::Types<JSValue>; \
  TYPED_TEST_SUITE(testSuite, JsTypes);

#endif
