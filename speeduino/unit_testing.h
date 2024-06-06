#pragma once

/**
 * @file
 * 
 * @brief Unit testing support
 * 
 */

/* @brief A tag for a static variable or function that can also be accessed by unit tests */
#if !defined(UNIT_TEST)
#define TESTABLE_STATIC static 
#else
#define TESTABLE_STATIC
#endif

/* @brief A tag for a static inline function that can also be accessed by unit tests */
#if !defined(UNIT_TEST)
#define TESTABLE_INLINE_STATIC static inline 
#else
#define TESTABLE_INLINE_STATIC
#endif
