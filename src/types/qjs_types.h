#pragma once

#include "engines/js_engine.h"
#include "qjs_candidate.h"
#include "qjs_config.h"
#include "qjs_config_item.h"
#include "qjs_config_list.h"
#include "qjs_config_map.h"
#include "qjs_config_value.h"
#include "qjs_context.h"
#include "qjs_engine.h"
#include "qjs_environment.h"
#include "qjs_key_event.h"
#include "qjs_os_info.h"
#include "qjs_preedit.h"
#include "qjs_schema.h"
#include "qjs_segment.h"
#include "qjs_trie.h"

template <typename T_JS_VALUE>
void registerTypesToJsEngine(JsEngine<T_JS_VALUE>& engine) {
  TypeMap<T_JS_VALUE> typeMap;
  LOG(INFO) << "[qjs] registering rime types to the " << typeMap.engineName << " engine...";

  // expose all types
  auto trie = std::make_unique<JsWrapper<rime::Trie, T_JS_VALUE>>();
  engine.template registerType<rime::Trie>(*trie.get());

  auto segment = std::make_unique<JsWrapper<rime::Segment, T_JS_VALUE>>();
  engine.template registerType<rime::Segment>(*segment.get());

  auto candidate = std::make_unique<JsWrapper<rime::Candidate, T_JS_VALUE>>();
  engine.template registerType<rime::Candidate>(*candidate.get());

  auto keyEvt = std::make_unique<JsWrapper<rime::KeyEvent, T_JS_VALUE>>();
  engine.template registerType<rime::KeyEvent>(*keyEvt.get());

  auto context = std::make_unique<JsWrapper<rime::Context, T_JS_VALUE>>();
  engine.template registerType<rime::Context>(*context.get());

  auto preedit = std::make_unique<JsWrapper<rime::Preedit, T_JS_VALUE>>();
  engine.template registerType<rime::Preedit>(*preedit.get());

  auto schema = std::make_unique<JsWrapper<rime::Schema, T_JS_VALUE>>();
  engine.template registerType<rime::Schema>(*schema.get());

  auto config = std::make_unique<JsWrapper<rime::Config, T_JS_VALUE>>();
  engine.template registerType<rime::Config>(*config.get());

  auto engineWrapper = std::make_unique<JsWrapper<rime::Engine, T_JS_VALUE>>();
  engine.template registerType<rime::Engine>(*engineWrapper.get());

  auto configItem = std::make_unique<JsWrapper<rime::ConfigItem, T_JS_VALUE>>();
  engine.template registerType<rime::ConfigItem>(*configItem.get());

  auto configValue = std::make_unique<JsWrapper<rime::ConfigValue, T_JS_VALUE>>();
  engine.template registerType<rime::ConfigValue>(*configValue.get());

  auto configList = std::make_unique<JsWrapper<rime::ConfigList, T_JS_VALUE>>();
  engine.template registerType<rime::ConfigList>(*configList.get());

  auto configMap = std::make_unique<JsWrapper<rime::ConfigMap, T_JS_VALUE>>();
  engine.template registerType<rime::ConfigMap>(*configMap.get());

  auto environment = std::make_unique<JsWrapper<Environment, T_JS_VALUE>>();
  engine.template registerType<Environment>(*environment.get());

  auto osInfo = std::make_unique<JsWrapper<SystemInfo, T_JS_VALUE>>();
  engine.template registerType<SystemInfo>(*osInfo.get());
}
