#include "qjs_engine.h"
#include "qjs_schema.h"
#include "qjs_context.h"
// #include "qjs_key_event.h"

namespace rime {

DEFINE_JS_CLASS_WITH_RAW_POINTER(Engine,
  DEFINE_GETTERS(schema, activeEngine, context),
  DEFINE_FUNCTIONS(
    // JS_CFUNC_DEF("processKey", 1, process_key), // TODO: key_event
    // JS_CFUNC_DEF("compose", 0, compose), // TODO: context
    JS_CFUNC_DEF("commitText", 1, commit_text),
    JS_CFUNC_DEF("applySchema", 1, apply_schema),
  )
)

DEFINE_GETTER(Engine, schema, Schema*, QjsSchema::Wrap)
DEFINE_GETTER(Engine, context, Context*, QjsContext::Wrap)
DEFINE_GETTER_2(Engine, activeEngine, active_engine, Engine*, QjsEngine::Wrap)

DEF_FUNC_WITH_SINGLE_STRING_PARAM(Engine, commit_text,
  obj->CommitText(param);
  return JS_UNDEFINED;
)

DEF_FUNC(Engine, apply_schema,
  if (argc < 1) return JS_FALSE;
  auto schema = QjsSchema::Unwrap(ctx, argv[0]);
  if (!schema) return JS_FALSE;

  obj->ApplySchema(schema);
  return JS_TRUE;
)

// JSValue QjsEngine::process_key(JSContext* ctx, JSValueConst this_val, int argc, JSValueConst* argv) {
//   if (argc < 1) return JS_FALSE;

//   auto engine = Unwrap(ctx, this_val);
//   if (!engine) return JS_FALSE;

//   auto keyEvent = QjsKeyEvent::Unwrap(ctx, argv[0]);
//   if (!keyEvent) return JS_FALSE;

//   return JS_NewBool(ctx, engine->ProcessKey(*keyEvent));
// }


} // namespace rime
