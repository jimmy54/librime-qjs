#include "qjs_config.h"  // IWYU pragma: keep

#include <filesystem>
#include <string>

#include "qjs_config_list.h"

namespace rime {

DEFINE_FUNCTION_ARGC(Config, loadFromFile, 1, {
  JSStringRAII param(JS_ToCString(ctx, argv[0]));
  return JS_NewBool(ctx, obj->LoadFromFile(std::filesystem::path(param.cStr())));
})
DEFINE_FUNCTION_ARGC(Config, saveToFile, 1, {
  JSStringRAII param(JS_ToCString(ctx, argv[0]));
  return JS_NewBool(ctx, obj->SaveToFile(std::filesystem::path(param.cStr())));
})
DEFINE_FUNCTION_ARGC(Config, getBool, 1, {
  JSStringRAII param(JS_ToCString(ctx, argv[0]));
  bool value = false;
  bool success = obj->GetBool(param, &value);
  return success ? JS_NewBool(ctx, value) : JS_NULL;
})
DEFINE_FUNCTION_ARGC(Config, getInt, 1, {
  JSStringRAII param(JS_ToCString(ctx, argv[0]));
  int value = 0;
  bool success = obj->GetInt(param, &value);
  return success ? JS_NewInt32(ctx, value) : JS_NULL;
})
DEFINE_FUNCTION_ARGC(Config, getDouble, 1, {
  JSStringRAII param(JS_ToCString(ctx, argv[0]));
  double value = 0.0;
  bool success = obj->GetDouble(param, &value);
  return success ? JS_NewFloat64(ctx, value) : JS_NULL;
})
DEFINE_FUNCTION_ARGC(Config, getString, 1, {
  JSStringRAII param(JS_ToCString(ctx, argv[0]));
  std::string value;
  bool success = obj->GetString(param, &value);
  return success ? JS_NewString(ctx, value.c_str()) : JS_NULL;
})
DEFINE_FUNCTION_ARGC(Config, getList, 1, {
  JSStringRAII param(JS_ToCString(ctx, argv[0]));
  auto list = obj->GetList(param);
  return QjsConfigList::wrap(ctx, list);
})

DEFINE_FUNCTION_ARGC(Config, setBool, 2, {
  JSStringRAII firstParam(JS_ToCString(ctx, argv[0]));
  bool value = JS_ToBool(ctx, argv[1]);
  bool success = obj->SetBool(firstParam, value);
  return JS_NewBool(ctx, success);
})
DEFINE_FUNCTION_ARGC(Config, setInt, 2, {
  JSStringRAII firstParam(JS_ToCString(ctx, argv[0]));
  int32_t value;
  JS_ToInt32(ctx, &value, argv[1]);
  bool success = obj->SetInt(firstParam, value);
  return JS_NewBool(ctx, success);
})
DEFINE_FUNCTION_ARGC(Config, setDouble, 2, {
  JSStringRAII firstParam(JS_ToCString(ctx, argv[0]));
  double value;
  JS_ToFloat64(ctx, &value, argv[1]);
  bool success = obj->SetDouble(firstParam, value);
  return JS_NewBool(ctx, success);
})
DEFINE_FUNCTION_ARGC(Config, setString, 2, {
  JSStringRAII firstParam(JS_ToCString(ctx, argv[0]));
  JSStringRAII value(JS_ToCString(ctx, argv[1]));
  // TODO: Config::SetString leaks memory, fix it in librime.
  // This issue could be reproduced by running: `ASAN_OPTIONS=detect_leaks=1 ctest`
  bool success = obj->SetString(firstParam, value);
  return JS_NewBool(ctx, success);
})

// Indirect leak of 64 byte(s) in 1 object(s) allocated from:
//     #0 0x0001012edf9d in _Znwm+0x7d (libclang_rt.asan_osx_dynamic.dylib:x86_64h+0x6af9d)
//     #1 0x000102031b84 in void* std::__1::__libcpp_operator_new[abi:ne200100]<unsigned long>(unsigned long) allocate.h:37
//     #2 0x0001020c6cde in std::__1::__shared_ptr_emplace<rime::ConfigValue, std::__1::allocator<rime::ConfigValue> >* std::__1::__libcpp_allocate[abi:ne200100]<std::__1::__shared_ptr_emplace<rime::ConfigValue, std::__1::allocator<rime::ConfigValue> > >(std::__1::__element_count, unsigned long) allocate.h:64
//     #3 0x0001020c6c5f in std::__1::allocator<std::__1::__shared_ptr_emplace<rime::ConfigValue, std::__1::allocator<rime::ConfigValue> > >::allocate[abi:ne200100](unsigned long) allocator.h:105
//     #4 0x0001020c6bdc in std::__1::allocator_traits<std::__1::allocator<std::__1::__shared_ptr_emplace<rime::ConfigValue, std::__1::allocator<rime::ConfigValue> > > >::allocate[abi:ne200100](std::__1::allocator<std::__1::__shared_ptr_emplace<rime::ConfigValue, std::__1::allocator<rime::ConfigValue> > >&, unsigned long) allocator_traits.h:270
//     #5 0x0001020c6b75 in std::__1::__allocation_guard<std::__1::allocator<std::__1::__shared_ptr_emplace<rime::ConfigValue, std::__1::allocator<rime::ConfigValue> > > >::__allocation_guard[abi:ne200100]<std::__1::allocator<rime::ConfigValue> >(std::__1::allocator<rime::ConfigValue>, unsigned long) allocation_guard.h:55
//     #6 0x0001020c69fc in std::__1::__allocation_guard<std::__1::allocator<std::__1::__shared_ptr_emplace<rime::ConfigValue, std::__1::allocator<rime::ConfigValue> > > >::__allocation_guard[abi:ne200100]<std::__1::allocator<rime::ConfigValue> >(std::__1::allocator<rime::ConfigValue>, unsigned long) allocation_guard.h:56
//     #7 0x0001020cf0c1 in _ZNSt3__115allocate_sharedB8ne200100IN4rime11ConfigValueENS_9allocatorIS2_EEJRKNS_12basic_stringIcNS_11char_traitsIcEENS3_IcEEEEETnNS_9enable_ifIXntsr8is_arrayIT_EE5valueEiE4typeELi0EEENS_10shared_ptrISD_EERKT0_DpOT1_ shared_ptr.h:732
//     #8 0x0001020cf074 in _ZNSt3__111make_sharedB8ne200100IN4rime11ConfigValueEJRKNS_12basic_stringIcNS_11char_traitsIcEENS_9allocatorIcEEEEETnNS_9enable_ifIXntsr8is_arrayIT_EE5valueEiE4typeELi0EEENS_10shared_ptrISC_EEDpOT0_ shared_ptr.h:741
//     #9 0x0001020cf02f in std::__1::shared_ptr<rime::ConfigValue> rime::New<rime::ConfigValue, std::__1::basic_string<char, std::__1::char_traits<char>, std::__1::allocator<char> > const&>(std::__1::basic_string<char, std::__1::char_traits<char>, std::__1::allocator<char> > const&) common.h:76
//     #10 0x0001020e0cd4 in rime::Config::SetString(std::__1::basic_string<char, std::__1::char_traits<char>, std::__1::allocator<char> > const&, std::__1::basic_string<char, std::__1::char_traits<char>, std::__1::allocator<char> > const&) config_component.cc:133
//     #11 0x0001008cee39 in rime::setString(JSContext*, JSValue, int, JSValue*) qjs_config.cc:68
//     #12 0x0001008e3c88 in js_call_c_function quickjs.c:14732
//     #13 0x00010092e5aa in JS_CallInternal quickjs.c:14947
//     #14 0x0001009325ec in JS_CallInternal quickjs.c:15361
//     #15 0x00010092dcb5 in JS_Call quickjs.c:17399
//     #16 0x0001008b415c in QuickJSTypesTest_WrapUnwrapRimeGears_Test::TestBody() types.test.cpp:72

DEFINE_JS_CLASS_WITH_RAW_POINTER(Config,
                                 NO_CONSTRUCTOR_TO_REGISTER,
                                 NO_PROPERTY_TO_REGISTER,
                                 NO_GETTER_TO_REGISTER,
                                 DEFINE_FUNCTIONS(JS_CFUNC_DEF("loadFromFile", 1, loadFromFile),
                                                  JS_CFUNC_DEF("saveToFile", 1, saveToFile),
                                                  JS_CFUNC_DEF("getBool", 1, getBool),
                                                  JS_CFUNC_DEF("getInt", 1, getInt),
                                                  JS_CFUNC_DEF("getDouble", 1, getDouble),
                                                  JS_CFUNC_DEF("getString", 1, getString),
                                                  JS_CFUNC_DEF("getList", 1, getList),
                                                  JS_CFUNC_DEF("setBool", 2, setBool),
                                                  JS_CFUNC_DEF("setInt", 2, setInt),
                                                  JS_CFUNC_DEF("setDouble", 2, setDouble),
                                                  JS_CFUNC_DEF("setString", 2, setString), ))

}  // namespace rime
