#pragma once

#include <cstdarg>
#include <exception>
#include <string>

enum class JsErrorType : std::uint8_t {
  SYNTAX,
  RANGE,
  REFERENCE,
  TYPE,
  EVAL,
  GENERIC,
  UNKNOWN,
};

class JsException : public std::exception {
private:
  std::string message_;
  JsErrorType type_;

public:
  // Constructor that takes an error message
  explicit JsException(JsErrorType type, const char* format, ...) : type_(type) {
    va_list args;
    va_start(args, format);
    char buffer[1024];
    vsnprintf(buffer, sizeof(buffer), format, args);
    va_end(args);
    message_ = buffer;
  }

  // Override what() method from std::exception
  [[nodiscard]] const char* what() const noexcept override { return message_.c_str(); }
  [[nodiscard]] JsErrorType getType() const noexcept { return type_; }
};
