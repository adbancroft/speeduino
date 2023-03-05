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
