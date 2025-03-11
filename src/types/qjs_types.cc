#include <cstddef>

#include "qjs_candidate.h"
#include "qjs_segment.h"
#include "qjs_trie.h"
// #include "qjs_translation.h"
#include "qjs_config.h"
#include "qjs_context.h"
#include "qjs_engine.h"
#include "qjs_key_event.h"
#include "qjs_preedit.h"
#include "qjs_schema.h"
// #include "qjs_menu.h"
// #include "qjs_memory.h"
// #include "qjs_dictionary.h"
// #include "qjs_composition.h"
// #include "qjs_code.h"
// #include "qjs_processor.h"
// #include "qjs_translation.h"
// #include "qjs_segmentation.h"
// #include "qjs_key_sequence.h"
#include "qjs_config_item.h"
#include "qjs_config_list.h"
#include "qjs_config_map.h"
#include "qjs_config_value.h"
// #include "qjs_translator.h"
// #include "qjs_notifier.h"
// #include "qjs_switcher.h"
// #include "qjs_reverse_db.h"
// #include "qjs_user_db.h"
// #include "qjs_dict_entry.h"
// #include "qjs_simple_candidate.h"
// Include other type headers

namespace rime {

void initQjsTypes(JSContext* ctx) {
  LOG(INFO) << "[qjs] registering rime types to the quickjs engine...";

  JS_SetModuleLoaderFunc(JS_GetRuntime(ctx), nullptr, QjsHelper::jsModuleLoader, nullptr);
  // Do not trigger GC when heap size is less than 16MB
  // default: rt->malloc_gc_threshold = 256 * 1024
  constexpr size_t SIXTEEN_MEGABYTES = 16L * 1024 * 1024;
  JS_SetGCThreshold(JS_GetRuntime(ctx), SIXTEEN_MEGABYTES);
  QjsHelper::exposeLogToJsConsole(ctx);

  // expose all types
  QjsTrie().expose(ctx);
  QjsSegment().expose(ctx);
  QjsCandidate().expose(ctx);
  // QjsTranslation().expose(ctx);
  QjsKeyEvent().expose(ctx);
  QjsContext().expose(ctx);
  QjsPreedit().expose(ctx);
  QjsSchema().expose(ctx);
  QjsConfig().expose(ctx);
  QjsEngine().expose(ctx);
  // QjsMenu().expose(ctx);
  // QjsMemory().expose(ctx);
  // QjsDictionary().expose(ctx);
  // QjsComposition().expose(ctx);
  // QjsCode().expose(ctx);
  // QjsProcessor().expose(ctx);
  // QjsFilter().expose(ctx);
  // QjsSegmentation().expose(ctx);
  // QjsKeySequence().expose(ctx);
  QjsConfigItem().expose(ctx);
  QjsConfigValue().expose(ctx);
  QjsConfigList().expose(ctx);
  QjsConfigMap().expose(ctx);
  // QjsSegmentor().expose(ctx);
  // QjsTranslator().expose(ctx);
  // QjsNotifier().expose(ctx);
  // QjsSwitcher().expose(ctx);
  // QjsReverseDb().expose(ctx);
  // QjsUserDb().expose(ctx);
  // QjsDictEntry().expose(ctx);
  // QjsSimpleCandidate().expose(ctx);
  // expose other types...
}

}  // namespace rime
