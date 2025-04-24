#pragma once

#include "engines/common.h"

#ifdef _ENABLE_JAVASCRIPTCORE

#define SETUP_JS_ENGINES(testSuite)                      \
  using JsTypes = ::testing::Types<JSValue, JSValueRef>; \
  TYPED_TEST_SUITE(testSuite, JsTypes);

#else

#define SETUP_JS_ENGINES(testSuite)          \
  using JsTypes = ::testing::Types<JSValue>; \
  TYPED_TEST_SUITE(testSuite, JsTypes);

#endif
