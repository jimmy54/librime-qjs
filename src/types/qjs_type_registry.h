#ifndef RIME_QJS_TYPE_REGISTRY_H_
#define RIME_QJS_TYPE_REGISTRY_H_

#include <quickjs.h>

#include <string>

namespace rime {

// Base class for type registration
class QjsTypeRegistry {
public:
  virtual ~QjsTypeRegistry() = default;

  // expose the type to QuickJS runtime
  virtual void expose(JSContext* ctx) = 0;

  // Get the class name for this type
  [[nodiscard]] virtual const char* getClassName() const = 0;
};

static inline JSValue jsNewStringFromStd(JSContext* ctx, const std::string& str) {
  return JS_NewString(ctx, str.c_str());
}

}  // namespace rime

#endif  // RIME_QJS_TYPE_REGISTRY_H_
