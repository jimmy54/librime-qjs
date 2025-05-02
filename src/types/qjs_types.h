#pragma once

#include "engines/common.h"
#include "qjs_candidate.h"
#include "qjs_candidate_iterator.h"
#include "qjs_commit_history.h"
#include "qjs_commit_record.h"
#include "qjs_config.h"
#include "qjs_config_item.h"
#include "qjs_config_list.h"
#include "qjs_config_map.h"
#include "qjs_config_value.h"
#include "qjs_context.h"
#include "qjs_engine.h"
#include "qjs_environment.h"
#include "qjs_key_event.h"
#include "qjs_leveldb.h"
#include "qjs_notifier.h"
#include "qjs_notifier_connection.h"
#include "qjs_os_info.h"
#include "qjs_preedit.h"
#include "qjs_schema.h"
#include "qjs_segment.h"
#include "qjs_trie.h"

template <typename T_JS_VALUE>
void registerTypesToJsEngine() {
  JsEngine<T_JS_VALUE>& engine = JsEngine<T_JS_VALUE>::instance();
  DLOG(INFO) << "[qjs] registering rime types to the " << engine.engineName << " engine...";

  // expose all types
  engine.template registerType<rime::Candidate>();
  engine.template registerType<rime::Translation>();
  engine.template registerType<rime::Trie>();
  engine.template registerType<LevelDb>();
  engine.template registerType<rime::Segment>();
  engine.template registerType<rime::KeyEvent>();
  engine.template registerType<rime::Context>();
  engine.template registerType<rime::Preedit>();
  engine.template registerType<rime::Schema>();
  engine.template registerType<rime::Config>();
  engine.template registerType<rime::Engine>();
  engine.template registerType<rime::ConfigItem>();
  engine.template registerType<rime::ConfigValue>();
  engine.template registerType<rime::ConfigList>();
  engine.template registerType<rime::ConfigMap>();
  engine.template registerType<Environment>();
  engine.template registerType<SystemInfo>();
  engine.template registerType<CommitRecord>();
  engine.template registerType<CommitHistory>();
  engine.template registerType<NotifierConnection>();
  engine.template registerType<Notifier>();
}
