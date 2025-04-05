#pragma once

#include <JavaScriptCore/JavaScriptCore.h>

class JscStringRAII {
public:
  JscStringRAII(const char* str) : str_(JSStringCreateWithUTF8CString(str)) {}
  JscStringRAII(JSStringRef str) : str_(str) {}

  JscStringRAII(const JscStringRAII&) = delete;
  JscStringRAII(JscStringRAII&&) = delete;
  JscStringRAII& operator=(const JscStringRAII&) = delete;
  JscStringRAII& operator=(JscStringRAII&&) = delete;

  ~JscStringRAII() { JSStringRelease(str_); }

  operator JSStringRef() const { return str_; }

private:
  JSStringRef str_;
};
