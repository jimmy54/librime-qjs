#include "qjs_type_registry.h"

namespace rime {

void QjsTypeRegistry::RegisterClassMethods(JSContext* ctx, JSValue proto,
                                         const JSCFunctionListEntry* methods,
                                         int count) {
  JS_SetPropertyFunctionList(ctx, proto, methods, count);
}

void QjsTypeRegistry::RegisterClassProperties(JSContext* ctx, JSValue proto,
                                            const JSCFunctionListEntry* properties,
                                            int count) {
  JS_SetPropertyFunctionList(ctx, proto, properties, count);
}

} // namespace rime
