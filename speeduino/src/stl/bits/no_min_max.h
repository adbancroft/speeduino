// Because Arduino defines min & max as macros, we need to undefine them in some cases

#if defined(max)
#pragma push_macro("max")
#undef max
#define POP_MAX() _Pragma("pop_macro(\"max\")")
#else
#define POP_MAX()
#endif

#if defined(min)
#pragma push_macro("min")
#undef min
#define POP_MIN() _Pragma("pop_macro(\"min\")")
#else
#define POP_MIN()
#endif

#define POP_MINMAX() \
    POP_MAX(); \
    POP_MIN()
