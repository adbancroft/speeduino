#include <unity.h>
#include <stdio.h>
#include "maths.h"
#include "timer.hpp"

template <typename T> inline T get_b(T a, T max)
{
    return (max - a)/3;
}
template <typename T> inline T get_div(T, T b)
{
    T div = b+3;
#if !defined(ARDUINO)
    if (div==0) { div = 1; }
#endif
    return div;
}


template <typename T>
void test_muldiv_t(T min, T max, T step)
{
    for (T a = min; a < max; a+= step)
    {
        T b = get_b(a, max);
        T div = get_div(a, b);
        T d = muldiv(a, b, div);
        T expected = (float)a * (float)b / (float)div;
        if (expected!=d)
        {
            char buffer[256];
            if (std::is_unsigned<T>::value)
            {
                sprintf(buffer, "a=%u, b=%u, div=%u, expected=%u, d=%u", a, b, div, expected, d);
            } else {
                sprintf(buffer, "a=%d, b=%d, div=%d, expected=%d, d=%d", a, b, div, expected, d);
            }
            TEST_MESSAGE(buffer);
        }
        TEST_ASSERT_EQUAL(expected, d);
    }
}

void test_muldiv_u8()
{
    test_muldiv_t<uint8_t>(1, UINT8_MAX, 1);
}

void test_muldiv_s8()
{
    test_muldiv_t<int8_t>(INT8_MIN, INT8_MAX, 1);
}

void test_muldiv_u16()
{
    test_muldiv_t<uint16_t>(1, UINT16_MAX/3, 11);
}

void test_muldiv_s16()
{
    test_muldiv_t<int16_t>(-(INT16_MAX/3), INT16_MAX/3, 23);
}


template <typename T, typename TPromote>
void test_muldiv_perf_t(uint16_t iters, const char *msgTag, T min, T max, T step)
{
    timer native_timer;
    int32_t checkSumNative = 0;
    native_timer.start();
    for (uint16_t loop=0; loop<iters; ++loop)
    {
        for (T a = min; a < max; a+= step)
        {
            T b = get_b(a, max);
            T div = get_div(a, b);
            checkSumNative += muldiv_simple<T, TPromote>(a, b, div);
        }
    }
    native_timer.stop();

    timer mulDiv_timer;
    int32_t checkSumMuldiv = 0;
    mulDiv_timer.start();
    for (uint16_t loop=0; loop<iters; ++loop)
    {
        for (T a = min; a < max; a+= step)
        {
            T b = get_b(a, max);
            T div = get_div(a, b);
            checkSumMuldiv += muldiv(a, b, div);
        }
    }
    mulDiv_timer.stop();

    TEST_ASSERT_EQUAL(checkSumNative, checkSumMuldiv);
    char buffer[256];
    sprintf(buffer, "muldiv %s timing: %lu, %lu", msgTag, native_timer.duration_micros(), mulDiv_timer.duration_micros());
    TEST_MESSAGE(buffer);
    TEST_ASSERT_LESS_THAN(native_timer.duration_micros(), mulDiv_timer.duration_micros());

}

void test_muldiv_u16_perf()
{
    // We mostly use muldiv() for small numbers, which is where we gain all the performance
    // E.g. various axis bin widths which are small. RPM probably <500, Load<20, TPS<10, Coolant<75 etc.    
    test_muldiv_perf_t<uint16_t, uint32_t>(128, "u16", 0, 512, 11);
}

void test_muldiv_s16_perf()
{
    // We mostly use muldiv() for small numbers, which is where we gain all the performance
    // E.g. various axis bin widths which are small. RPM probably <500, Load<20, TPS<10, Coolant<75 etc.    
    test_muldiv_perf_t<int16_t, int32_t>(128, "s16", -256, 256, 11);
}

void testmuldiv()
{
    RUN_TEST(test_muldiv_u8);
    RUN_TEST(test_muldiv_s8);
    RUN_TEST(test_muldiv_u16);
    RUN_TEST(test_muldiv_s16);
#if defined(ARDUINO)    
    RUN_TEST(test_muldiv_u16_perf);
    RUN_TEST(test_muldiv_s16_perf);
#endif
}