#pragma once


#if !defined(UNIT_TEST)
#define TESTABLE_STATIC static 
#else
#define TESTABLE_STATIC
#endif

#if !defined(UNIT_TEST)
#define TESTABLE_INLINE_STATIC static inline 
#else
#define TESTABLE_INLINE_STATIC
#endif