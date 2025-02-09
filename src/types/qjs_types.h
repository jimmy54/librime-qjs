#ifndef RIME_QJS_TYPES_H_
#define RIME_QJS_TYPES_H_

#include <quickjs.h>

namespace rime {

// Initialize all QuickJS type bindings
void init_qjs_types(JSContext* ctx);

} // namespace rime

#endif // RIME_QJS_TYPES_H_
