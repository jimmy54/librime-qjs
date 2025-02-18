#include "qjs_schema.h"
#include "qjs_config.h"

namespace rime {

DEFINE_JS_CLASS_WITH_RAW_POINTER(
  Schema,
  NO_CONSTRUCTOR_TO_REGISTER,
  DEFINE_GETTERS(id, name, config, pageSize, selectKeys),
  NO_FUNCTION_TO_REGISTER
)

DEFINE_GETTER_2(Schema, id, schema_id, const string&, js_new_string_from_std)
DEFINE_GETTER_2(Schema, name, schema_name, const string&, js_new_string_from_std)
DEFINE_GETTER  (Schema, config, Config*, QjsConfig::Wrap)
DEFINE_GETTER_2(Schema, pageSize, page_size, int, JS_NewInt32)
DEFINE_GETTER_2(Schema, selectKeys, select_keys, const string&, js_new_string_from_std)

} // namespace rime
