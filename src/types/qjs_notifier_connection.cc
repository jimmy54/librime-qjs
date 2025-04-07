#include "qjs_notifier_connection.h"

#include "qjs_macros.h"

namespace rime {

DEFINE_FUNCTION(NotifierConnection, disconnect, {
  obj->disconnect();
  JSValue jsListenerFunc = JS_GetPropertyStr(ctx, thisVal, JS_LISTENER_PROPERTY_NAME);
  JS_FreeValue(ctx, jsListenerFunc);
  return JS_UNDEFINED;
})

DEFINE_GETTER(NotifierConnection, isConnected, JS_NewBool(ctx, obj->connected()))

DEFINE_JS_CLASS_WITH_SHARED_POINTER(NotifierConnection,
                                    NO_CONSTRUCTOR_TO_REGISTER,
                                    NO_PROPERTY_TO_REGISTER,
                                    DEFINE_GETTERS(isConnected),
                                    DEFINE_FUNCTIONS(JS_CFUNC_DEF("disconnect", 0, disconnect)))

}  // namespace rime
