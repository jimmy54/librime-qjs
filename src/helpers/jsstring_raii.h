#ifndef RIME_QJS_JSSTRING_RAII_H_
#define RIME_QJS_JSSTRING_RAII_H_

#include <glog/logging.h>
#include <quickjs.h>
#include "qjs_helper.h"

namespace rime {

class JSStringRAII {
public:
    explicit JSStringRAII(const char *str) : str_(str) {}

    ~JSStringRAII() {
        auto ctx = QjsHelper::getInstance().getContext();
        if (ctx && str_) {
            JS_FreeCString(ctx, str_);
        }
    }

    operator std::string() const { return std::string(str_); }
private:
    const char *str_{nullptr};
};

} // namespace rime

#endif // RIME_QJS_JSSTRING_RAII_H_
