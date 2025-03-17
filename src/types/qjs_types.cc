#include <cstddef>

#include "qjs_candidate.h"
#include "qjs_config.h"
#include "qjs_config_item.h"
#include "qjs_config_list.h"
#include "qjs_config_map.h"
#include "qjs_config_value.h"
#include "qjs_context.h"
#include "qjs_engine.h"
#include "qjs_key_event.h"
#include "qjs_preedit.h"
#include "qjs_schema.h"
#include "qjs_segment.h"
#include "qjs_trie.h"

#include "node_module_loader.h"

namespace rime {

void initQjsTypes(JSContext* ctx) {
  LOG(INFO) << "[qjs] registering rime types to the quickjs engine...";

  JS_SetModuleLoaderFunc(JS_GetRuntime(ctx), nullptr, js_module_loader, nullptr);
  // Do not trigger GC when heap size is less than 16MB
  // default: rt->malloc_gc_threshold = 256 * 1024
  constexpr size_t SIXTEEN_MEGABYTES = 16L * 1024 * 1024;
  JS_SetGCThreshold(JS_GetRuntime(ctx), SIXTEEN_MEGABYTES);
  QjsHelper::exposeLogToJsConsole(ctx);

  // expose all types
  QjsTrie().expose(ctx);
  QjsSegment().expose(ctx);
  QjsCandidate().expose(ctx);
  QjsKeyEvent().expose(ctx);
  QjsContext().expose(ctx);
  QjsPreedit().expose(ctx);
  QjsSchema().expose(ctx);
  QjsConfig().expose(ctx);
  QjsEngine().expose(ctx);
  QjsConfigItem().expose(ctx);
  QjsConfigValue().expose(ctx);
  QjsConfigList().expose(ctx);
  QjsConfigMap().expose(ctx);
}

}  // namespace rime
