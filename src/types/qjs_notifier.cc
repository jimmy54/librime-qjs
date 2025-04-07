#include "qjs_notifier.h"
#include <memory>

#include "jsvalue_raii.hpp"
#include "qjs_context.h"
#include "qjs_macros.h"
#include "qjs_notifier_connection.h"

namespace rime {

DEFINE_FUNCTION_ARGC(Notifier, connect, 1, {
  auto jsListenerFunc = argv[0];
  if (!JS_IsFunction(ctx, jsListenerFunc)) {
    return JS_ThrowTypeError(ctx, "The argument of notifier.connect(arg) should be a function");
  }

  // IMPORTANT: jsListenerFunc should be duplicated before passing to JS_Call,
  // otherwise it will be released by the js engine and the function will not be called
  JS_DupValue(ctx, jsListenerFunc);

  auto connection = std::make_shared<NotifierConnection>(
      obj->connect([ctx, jsListenerFunc](Context* rimeContext) {
        JSValueRAII arg = QjsContext::wrap(ctx, rimeContext);
        JSValueRAII result = JS_Call(ctx, jsListenerFunc, JS_UNDEFINED, 1, arg.getPtr());
        if (JS_IsException(result)) {
          LOG(ERROR) << "Error in notifying the js connection";
        }
      }));

  auto jsConnection = QjsNotifierConnection::wrap(ctx, connection);
  // attach it to the connection to free it by the js engine when disconnecting
  JS_SetPropertyStr(ctx, jsConnection, JS_LISTENER_PROPERTY_NAME, jsListenerFunc);
  return jsConnection;
})

DEFINE_JS_CLASS_WITH_RAW_POINTER(Notifier,
                                 NO_CONSTRUCTOR_TO_REGISTER,
                                 NO_PROPERTY_TO_REGISTER,
                                 NO_GETTER_TO_REGISTER,
                                 DEFINE_FUNCTIONS(JS_CFUNC_DEF("connect", 1, connect)))

}  // namespace rime
