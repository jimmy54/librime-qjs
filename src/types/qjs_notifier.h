#pragma once

#include <rime/context.h>

#include "engines/js_macros.h"
#include "js_exception.h"
#include "js_wrapper.h"
#include "qjs_notifier_connection.h"

using Notifier = rime::signal<void(rime::Context* ctx)>;
using NotifierConnection = rime::connection;

template <typename T_JS_VALUE>
class JsWrapper<Notifier, T_JS_VALUE> {
  DEFINE_CFUNCTION_ARGC(connect, 1, {
    auto jsListenerFunc = argv[0];
    if (!engine.isFunction(jsListenerFunc)) {
      const char* msg = "The argument of notifier.connect(arg) should be a function";
      throw new JsException(JsErrorType::TYPE, msg);
    }

    // IMPORTANT: jsListenerFunc should be duplicated before passing to JS_Call,
    // otherwise it will be released by the js engine and the function will not be called
    auto duplicatedFunc = engine.duplicateValue(jsListenerFunc);

    auto obj = engine.unwrap<Notifier>(thisVal);
    auto connection = std::make_shared<NotifierConnection>(
        obj->connect([ctx, duplicatedFunc](rime::Context* rimeContext) {
          auto& engine = JsEngine<T_JS_VALUE>::getEngineByContext(ctx);
          auto undefined = engine.toObject(engine.undefined());
          auto arg = engine.wrap(rimeContext);
          T_JS_VALUE args[] = {arg};
          auto result = engine.callFunction(engine.toObject(duplicatedFunc), undefined, 1, args);
          if (engine.isException(result)) {
            LOG(ERROR) << "Error in notifying the js connection";
          }
          engine.freeValue(result, arg);
        }));

    auto jsConnection = engine.wrapShared(connection);
    // attach it to the connection to free it by the js engine when disconnecting
    engine.setObjectProperty(jsConnection, JS_LISTENER_PROPERTY_NAME, duplicatedFunc);
    return jsConnection;
  })

public:
  EXPORT_CLASS_WITH_RAW_POINTER(Notifier,
                                WITHOUT_CONSTRUCTOR,
                                WITHOUT_PROPERTIES,
                                WITHOUT_GETTERS,
                                WITH_FUNCTIONS(connect, 1));
};
