#ifndef RIME_QJS_TYPES_H_
#define RIME_QJS_TYPES_H_

#include <quickjs.h>

namespace rime {

// Initialize all QuickJS type bindings
void initQjsTypes(JSContext* ctx);

} // namespace rime

#endif // RIME_QJS_TYPES_H_
