#include "test_saturated_cast.h"
#include "math/saturated_cast.hpp"
#include <unity.h>


void test_signed_unsigned_narrow()
{
    TEST_ASSERT_EQUAL_UINT8(0, saturated_cast<uint8_t>((int16_t)-1));
    TEST_ASSERT_EQUAL_UINT8(UINT8_MAX, saturated_cast<uint8_t>((int16_t)UINT8_MAX+1));
    TEST_ASSERT_EQUAL_UINT8(0, saturated_cast<uint8_t>((int16_t)0));
}

void test_signed_unsigned_wider()
{
    TEST_ASSERT_EQUAL_UINT16(0, saturated_cast<uint16_t>((int8_t)INT8_MIN));
    TEST_ASSERT_EQUAL_UINT16(INT8_MAX, saturated_cast<uint16_t>((int8_t)INT8_MAX));
    TEST_ASSERT_EQUAL_UINT16(0, saturated_cast<uint16_t>((int8_t)0));
}

void test_signed_unsigned_samewidth()
{
    TEST_ASSERT_EQUAL_UINT8(0, saturated_cast<uint8_t>((int8_t)INT8_MIN));
    TEST_ASSERT_EQUAL_UINT8(INT8_MAX, saturated_cast<uint8_t>((int8_t)INT8_MAX));
    TEST_ASSERT_EQUAL_UINT8(0, saturated_cast<uint8_t>((int8_t)0));
}

void test_unsigned_signed_narrow()
{
    TEST_ASSERT_EQUAL_INT8(INT8_MAX, saturated_cast<int8_t>((uint16_t)INT8_MAX+1));
    TEST_ASSERT_EQUAL_INT8(0, saturated_cast<int8_t>((uint16_t)0));
}

void test_unsigned_signed_wider()
{
    TEST_ASSERT_EQUAL_INT16(INT8_MAX, saturated_cast<int16_t>((uint8_t)INT8_MAX));
    TEST_ASSERT_EQUAL_INT16(0, saturated_cast<int16_t>((uint8_t)0));
}

void test_unsigned_signed_samewidth()
{
    TEST_ASSERT_EQUAL_INT16(INT16_MAX, saturated_cast<int16_t>((uint16_t)UINT16_MAX));
    TEST_ASSERT_EQUAL_INT16(0, saturated_cast<int16_t>((uint16_t)0));
}


void test_signed_signed_narrow()
{
    TEST_ASSERT_EQUAL_INT8(INT8_MAX, saturated_cast<int8_t>((int16_t)INT16_MAX));
    TEST_ASSERT_EQUAL_INT8(INT8_MIN, saturated_cast<int8_t>((int16_t)INT16_MIN));
    TEST_ASSERT_EQUAL_INT8(0, saturated_cast<int8_t>((int16_t)0));
}


void test_unsigned_unsigned_narrow()
{
    TEST_ASSERT_EQUAL_UINT8(UINT8_MAX, saturated_cast<uint8_t>((uint16_t)UINT16_MAX));
    TEST_ASSERT_EQUAL_UINT8(0, saturated_cast<uint8_t>((uint16_t)0));
}

void test_saturated_cast()
{
    RUN_TEST(test_signed_unsigned_narrow);
    RUN_TEST(test_signed_unsigned_wider);
    RUN_TEST(test_signed_unsigned_samewidth);
    RUN_TEST(test_unsigned_signed_narrow);
    RUN_TEST(test_unsigned_signed_wider);
    RUN_TEST(test_unsigned_signed_samewidth);
    RUN_TEST(test_signed_signed_narrow);
    RUN_TEST(test_unsigned_unsigned_narrow);
}