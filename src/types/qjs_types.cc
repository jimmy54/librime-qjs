#include "qjs_types.h"
// #include "qjs_segment.h"
#include "qjs_candidate.h"
// #include "qjs_translation.h"
// #include "qjs_key_event.h"
#include "qjs_context.h"
#include "qjs_preedit.h"
#include "qjs_schema.h"
#include "qjs_config.h"
#include "qjs_engine.h"
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
#include "qjs_config_value.h"
#include "qjs_config_list.h"
#include "qjs_config_map.h"
// #include "qjs_segmentor.h"
// #include "qjs_translator.h"
// #include "qjs_notifier.h"
// #include "qjs_switcher.h"
// #include "qjs_reverse_db.h"
// #include "qjs_user_db.h"
// #include "qjs_dict_entry.h"
// #include "qjs_simple_candidate.h"
// Include other type headers

namespace rime {

void init_qjs_types(JSContext* ctx) {
  LOG(INFO) << "registering rime types to the quickjs engine...";

  JS_SetModuleLoaderFunc(JS_GetRuntime(ctx), nullptr, QjsHelper::jsModuleLoader, nullptr);
  QjsHelper::exposeLogToJsConsole(ctx);

  // Register all types
  // QjsSegment().Register(ctx);
  QjsCandidate().Register(ctx);
  // QjsTranslation().Register(ctx);
  // QjsKeyEvent().Register(ctx);
  QjsContext().Register(ctx);
  QjsPreedit().Register(ctx);
  QjsSchema().Register(ctx);
  QjsConfig().Register(ctx);
  QjsEngine().Register(ctx);
  // QjsMenu().Register(ctx);
  // QjsMemory().Register(ctx);
  // QjsDictionary().Register(ctx);
  // QjsComposition().Register(ctx);
  // QjsCode().Register(ctx);
  // QjsProcessor().Register(ctx);
  // QjsFilter().Register(ctx);
  // QjsSegmentation().Register(ctx);
  // QjsKeySequence().Register(ctx);
  QjsConfigItem().Register(ctx);
  QjsConfigValue().Register(ctx);
  QjsConfigList().Register(ctx);
  QjsConfigMap().Register(ctx);
  // QjsSegmentor().Register(ctx);
  // QjsTranslator().Register(ctx);
  // QjsNotifier().Register(ctx);
  // QjsSwitcher().Register(ctx);
  // QjsReverseDb().Register(ctx);
  // QjsUserDb().Register(ctx);
  // QjsDictEntry().Register(ctx);
  // QjsSimpleCandidate().Register(ctx);
  // Register other types...
}

} // namespace rime
