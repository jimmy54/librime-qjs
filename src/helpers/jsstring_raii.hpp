#ifndef RIME_QJS_JSSTRING_RAII_H_
#define RIME_QJS_JSSTRING_RAII_H_

#include <glog/logging.h>
#include <quickjs.h>

#include "qjs_helper.h"

namespace rime {

class JSStringRAII {
public:
  JSStringRAII(const char* str) : str_(str) {}

  JSStringRAII(const JSStringRAII&) = delete;
  JSStringRAII(JSStringRAII&&) = delete;
  JSStringRAII& operator=(const JSStringRAII&) = delete;
  JSStringRAII& operator=(JSStringRAII&&) = delete;

  ~JSStringRAII() {
    auto* ctx = QjsHelper::getInstance().getContext();
    if (str_ != nullptr) {
      JS_FreeCString(ctx, str_);
    }
  }

  operator std::string() const { return {str_}; }

  [[nodiscard]] const char* cStr() const { return str_; }

private:
  const char* str_{nullptr};
};

}  // namespace rime

#endif  // RIME_QJS_JSSTRING_RAII_H_
