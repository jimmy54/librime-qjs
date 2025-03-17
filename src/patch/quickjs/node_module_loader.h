#include "quickjs.h"

#ifdef __cplusplus
extern "C" {
#endif

void setQjsBaseFolder(const char* path);

// NOLINTNEXTLINE(readability-identifier-naming)
JSModuleDef* js_module_loader(JSContext* ctx, const char* moduleName, void* opaque);

JSValue loadJsModule(JSContext* ctx, const char* fileName);

char* readJsCode(JSContext* ctx, const char* relativePath);

char* loadFile(const char* absolutePath);

#ifdef __cplusplus
}
#endif
