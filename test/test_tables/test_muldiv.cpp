#include <unity.h>
#include <stdio.h>
#include "maths.h"
#include "timer.hpp"


void test_muldiv_u16()
{
    constexpr uint16_t step = 11;
    for (uint16_t a = 1; a < UINT16_MAX/3; a+=step)
    {
        uint16_t b = (UINT16_MAX/3) - a;
        uint16_t div = a + b;
        uint16_t d = muldiv(a, b, div);
        uint16_t expected = (float)a * (float)b / (float)div;
        if (expected!=d)
        {
            char buffer[256];
            sprintf(buffer, "a=%u, b=%u, div=%u, expected=%u, d=%u", a, b, div, expected, d);
            TEST_MESSAGE(buffer);
        }
        TEST_ASSERT_EQUAL(expected, d);
    }
}

void test_muldiv_s16()
{
    constexpr int16_t step = 23;
    for (int16_t a = -(INT16_MAX/3); a < INT16_MAX/3; a+=step)
    {
        int16_t b = -a;
        int16_t div = abs(a) + abs(b);
        int16_t d = muldiv(a, b, div);
        int16_t expected = (float)a * (float)b / (float)div;
        if (expected!=d)
        {
            char buffer[256];
            sprintf(buffer, "a=%d, b=%d, div=%d, expected=%d, d=%d", a, b, div, expected, d);
            TEST_MESSAGE(buffer);
        }
        TEST_ASSERT_EQUAL(expected, d);
    }
}

void test_muldiv_u16_perf()
{
    constexpr uint16_t step = 11;
    // We mostly use muldiv() for small numbers, which is where we gain all the performance
    // E.g. various axis bin widths which are small. RPM probably <500, Load<20, TPS<10, Coolant<75 etc.    
    constexpr uint16_t max = 256;
    constexpr uint16_t iters = 128;

    timer native_timer;
    uint32_t checkSumNative = 0;
    native_timer.start();
    for (uint16_t loop=0; loop<iters; ++loop)
    {
        for (uint16_t a = 1; a < max; a+=step)
        {
            uint16_t b = max - a;
            uint16_t div = a + b;
            checkSumNative += (uint32_t)a * (uint32_t)b / (uint32_t)div;
        }
    }
    native_timer.stop();

    timer mulDiv_timer;
    uint32_t checkSumMuldiv = 0;

    mulDiv_timer.start();
    for (uint16_t loop=0; loop<iters; ++loop)
    {
        for (uint16_t a = 1; a < max; a+=step)
        {
            uint16_t b = max - a;
            uint16_t div = a + b;
            checkSumMuldiv += muldiv(a, b, div);
        }
    }
    mulDiv_timer.stop();

    TEST_ASSERT_EQUAL(checkSumNative, checkSumMuldiv);
    char buffer[256];
    sprintf(buffer, "muldiv u16 timing: %lu, %lu", native_timer.duration_micros(), mulDiv_timer.duration_micros());
    TEST_MESSAGE(buffer);
    TEST_ASSERT_LESS_THAN(native_timer.duration_micros(), mulDiv_timer.duration_micros());
}

void test_muldiv_s16_perf()
{
    constexpr int16_t step = 11;
    // We mostly use muldiv() for small numbers, which is where we gain all the performance
    // E.g. various axis bin widths which are small. RPM probably <500, Load<20, TPS<10, Coolant<75 etc.  
    constexpr int16_t min = -128;  
    constexpr int16_t max = 128;
    constexpr int16_t iters = 128;

    timer native_timer;
    uint32_t checkSumNative = 0;
    native_timer.start();
    for (int16_t loop=0; loop<iters; ++loop)
    {
        for (int16_t a = min; a < max; a+=step)
        {
            int16_t b = -a;
            int16_t div = abs(a) + abs(b);
            checkSumNative += (int32_t)a * (int32_t)b / (int32_t)div;
        }
    }
    native_timer.stop();

    timer mulDiv_timer;
    uint32_t checkSumMuldiv = 0;

    mulDiv_timer.start();
    for (int16_t loop=0; loop<iters; ++loop)
    {
        for (int16_t a = min; a < max; a+=step)
        {
            int16_t b = -a;
            int16_t div = abs(a) + abs(b);
            checkSumMuldiv += muldiv(a, b, div);
        }
    }
    mulDiv_timer.stop();

    TEST_ASSERT_EQUAL(checkSumNative, checkSumMuldiv);
    char buffer[256];
    sprintf(buffer, "muldiv s16 timing: %lu, %lu", native_timer.duration_micros(), mulDiv_timer.duration_micros());
    TEST_MESSAGE(buffer);
    TEST_ASSERT_LESS_THAN(native_timer.duration_micros(), mulDiv_timer.duration_micros());
}

void testmuldiv()
{
    RUN_TEST(test_muldiv_u16);
    RUN_TEST(test_muldiv_s16);
#if defined(ARDUINO)    
    RUN_TEST(test_muldiv_u16_perf);
    RUN_TEST(test_muldiv_s16_perf);
#endif
}