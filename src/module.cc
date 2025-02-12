#include <rime_api.h>
#include <rime/common.h>
#include <rime/registry.h>

#include "qjs_helpers.h"
#include "qjs_component.h"
#include "qjs_types.h"
#include "qjs_filter.h"

using namespace rime;

static void rime_qjs_initialize() {
  LOG(INFO) << "[qjs] registering components from module 'qjs'.";
  Registry& r = Registry::instance();

  static JSRuntime *rt = JS_NewRuntime();
  static JSContext *ctx = JS_NewContext(rt);
  init_qjs_types(ctx);
  registerLogToJsConsole(ctx);

  string jsDirectory = string(rime_get_api()->get_user_data_dir()) + "/js";
  r.Register("qjs_filter", new QuickJSComponent<QuickJSFilter>(ctx, jsDirectory));
}

static void rime_qjs_finalize() {}

RIME_REGISTER_MODULE(qjs)
