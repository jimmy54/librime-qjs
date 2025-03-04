#include "qjs_engine.h"
#include "qjs_schema.h"
#include "qjs_context.h"
#include <rime/key_event.h>

namespace rime {

DEFINE_GETTER(Engine, schema, QjsSchema::Wrap(ctx, obj->schema()))
DEFINE_GETTER(Engine, context, QjsContext::Wrap(ctx, obj->context()))
DEFINE_GETTER(Engine, activeEngine, QjsEngine::Wrap(ctx, obj->active_engine()))

DEFINE_FUNCTION_ARGC(Engine, commitText, 1,
  JSStringRAII param(JS_ToCString(ctx, argv[0]));
  obj->CommitText(param);
  return JS_UNDEFINED;
)

DEFINE_FUNCTION_ARGC(Engine, applySchema, 1,
  auto schema = QjsSchema::Unwrap(ctx, argv[0]);
  if (!schema) return JS_FALSE;

  obj->ApplySchema(schema);
  return JS_TRUE;
)

DEFINE_FUNCTION_ARGC(Engine, processKey, 1,
  JSStringRAII repr(JS_ToCString(ctx, argv[0]));
  return JS_NewBool(ctx, obj->ProcessKey(KeyEvent(repr)));
)

DEFINE_JS_CLASS_WITH_RAW_POINTER(
  Engine,
  NO_CONSTRUCTOR_TO_REGISTER,
  NO_PROPERTY_TO_REGISTER,
  DEFINE_GETTERS(schema, activeEngine, context),
  DEFINE_FUNCTIONS(
    JS_CFUNC_DEF("processKey", 1, processKey),
    JS_CFUNC_DEF("commitText", 1, commitText),
    JS_CFUNC_DEF("applySchema", 1, applySchema),
  )
)

} // namespace rime
