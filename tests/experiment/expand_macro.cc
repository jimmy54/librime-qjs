// to check the macro output
// usage: $ g++ -E ./expand_macro.cc -o macro_output.i


#define EXPAND(...) __VA_ARGS__
#define FOR_EACH_1(macro, x) macro(x)
#define FOR_EACH_2(macro, x, ...) macro(x) EXPAND(FOR_EACH_1(macro, __VA_ARGS__))
#define FOR_EACH_3(macro, x, ...) macro(x) EXPAND(FOR_EACH_2(macro, __VA_ARGS__))
#define FOR_EACH_4(macro, x, ...) macro(x) EXPAND(FOR_EACH_3(macro, __VA_ARGS__))
#define FOR_EACH_5(macro, x, ...) macro(x) EXPAND(FOR_EACH_4(macro, __VA_ARGS__))
#define FOR_EACH_6(macro, x, ...) macro(x) EXPAND(FOR_EACH_5(macro, __VA_ARGS__))
#define FOR_EACH_7(macro, x, ...) macro(x) EXPAND(FOR_EACH_6(macro, __VA_ARGS__))
#define FOR_EACH_8(macro, x, ...) macro(x) EXPAND(FOR_EACH_7(macro, __VA_ARGS__))
#define FOR_EACH_9(macro, x, ...) macro(x) EXPAND(FOR_EACH_8(macro, __VA_ARGS__))
#define FOR_EACH_10(macro, x, ...) macro(x) EXPAND(FOR_EACH_9(macro, __VA_ARGS__))
#define FOR_EACH_11(macro, x, ...) macro(x) EXPAND(FOR_EACH_10(macro, __VA_ARGS__))
#define FOR_EACH_12(macro, x, ...) macro(x) EXPAND(FOR_EACH_11(macro, __VA_ARGS__))
#define FOR_EACH_13(macro, x, ...) macro(x) EXPAND(FOR_EACH_12(macro, __VA_ARGS__))
#define FOR_EACH_14(macro, x, ...) macro(x) EXPAND(FOR_EACH_13(macro, __VA_ARGS__))
#define FOR_EACH_15(macro, x, ...) macro(x) EXPAND(FOR_EACH_14(macro, __VA_ARGS__))
#define FOR_EACH_16(macro, x, ...) macro(x) EXPAND(FOR_EACH_15(macro, __VA_ARGS__))
#define FOR_EACH_17(macro, x, ...) macro(x) EXPAND(FOR_EACH_16(macro, __VA_ARGS__))
#define FOR_EACH_18(macro, x, ...) macro(x) EXPAND(FOR_EACH_17(macro, __VA_ARGS__))
#define FOR_EACH_19(macro, x, ...) macro(x) EXPAND(FOR_EACH_18(macro, __VA_ARGS__))
#define FOR_EACH_20(macro, x, ...) macro(x) EXPAND(FOR_EACH_19(macro, __VA_ARGS__))

// Get number of arguments
#define COUNT_ARGS_(_1, _2, _3, _4, _5, _6, _7, _8, _9, _10, \
                    _11, _12, _13, _14, _15, _16, _17, _18, _19, _20,  N, ...) N
#define COUNT_ARGS(...) COUNT_ARGS_(__VA_ARGS__, \
  20, 19, 18, 17, 16, 15, 14, 13, 12, 11, \
  10, 9, 8, 7, 6, 5, 4, 3, 2, 1)

// Select the appropriate FOR_EACH macro based on argument count
#define _FOR_EACH_N(N, macro, ...) FOR_EACH_##N(macro, __VA_ARGS__)
#define FOR_EACH_N(N, macro, ...) _FOR_EACH_N(N, macro, __VA_ARGS__)
#define FOR_EACH(macro, ...) FOR_EACH_N(COUNT_ARGS(__VA_ARGS__), macro, __VA_ARGS__)

#define DEFINE_PROPERTIES(...)                                                 \
    static struct { \
        FOR_EACH(DEFINE_PROPERTY, __VA_ARGS__) \
    } obj;

#define DEFINE_PROPERTY(name) int name;

#define NO_PROPERTY_TO_REGISTER {}
#define NO_FUNCTION_TO_REGISTER {}
#define NO_CONSTRUCTOR_TO_REGISTER {}

#define GET_MACRO(_1, _2, _3, NAME, ...) NAME

#define TEST_1(x) EXPAND(x)
#define TEST_2(x, y) \
  EXPAND(x); \
  EXPAND(y);
#define TEST_3(x, y, z) \
  EXPAND(x); \
  EXPAND(y); \
  EXPAND(z);


#define DEFINE_JS_CLASS_WITH_RAW_POINTER(class_name, ...)   \
  DEFINE_JS_CLASS_IMPL(class_name, EXPANDWRAP_UNWRAP_WITH_RAW_POINTER(class_name), REGISTER_IMPL(__VA_ARGS__))

#define DEFINE_JS_CLASS_IMPL(class_name, wrap_unwrap_func, register_func) \
  static void register_##class_name() { \
    register_func; \
  } \
  wrap_unwrap_func

#define EXPANDWRAP_UNWRAP_WITH_RAW_POINTER(class_name) \
  static void js_wrap_##class_name() { \
  } \
  static void js_unwrap_##class_name() { \
  }

#define REGISTER_IMPL(...) \
  GET_MACRO(__VA_ARGS__, TEST_3, TEST_2, TEST_1)(__VA_ARGS__)

DEFINE_JS_CLASS_WITH_RAW_POINTER(Candidate,
  DEFINE_PROPERTIES(text, var),
  NO_FUNCTION_TO_REGISTER,
  NO_CONSTRUCTOR_TO_REGISTER
)
