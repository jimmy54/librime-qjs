#pragma once

#include <cstdarg>
#include <string>

template <typename T_JS_VALUE>
class JsEngine {};

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
