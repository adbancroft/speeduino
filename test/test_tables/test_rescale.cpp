#include "maths.h"
#include <unity.h>
#include <stdio.h>

template <typename T, typename U>
void test_rescale(T fromValue, T fromMin, T fromMax, U toMin, U toMax)
{
    // This is the floating point version of the rescale function
    float f_expected = (((float)fromValue - (float)fromMin) / ((float)fromMax - (float)fromMin))
          * ((float)toMax - (float)toMin) + (float)toMin;

    U actual = rescale(fromValue, fromMin, fromMax, toMin, toMax);
    U expected = f_expected;
    if (expected!=actual)
    {
        char buffer[96];
        char *pBuffer = buffer;

        char formatT_U[] = "in=%u, fmin=%u, fmax=%u, ";
        char formatT_S[] = "in=%d, fmin=%d, fmax=%d, ";
        pBuffer += sprintf(pBuffer, std::is_unsigned<T>::value ? formatT_U : formatT_S, fromValue, fromMin, fromMax);

        char formatU_U[] = "tmin=%u, tmax=%u";
        char formatU_S[] = "tmin=%d, tmax=%d";
        sprintf(pBuffer, std::is_unsigned<U>::value ? formatU_U : formatU_S, toMin, toMax);

        TEST_MESSAGE(buffer);
    }
    // We do not apply rounding, so expect +/- 1 from 'real' result.
    TEST_ASSERT_INT_WITHIN(1, expected, actual);
}

void test_rescale_s8_s8()
{  
    // Ascending
    test_rescale((int8_t)3, (int8_t)0, (int8_t)4, (int8_t)0, (int8_t)8);
    test_rescale((int8_t)-1, (int8_t)-5, (int8_t)10, (int8_t)-1, (int8_t)7);
    test_rescale((int8_t)0, (int8_t)-5, (int8_t)10, (int8_t)-1, (int8_t)7);

    // Descending
    test_rescale((int8_t)3, (int8_t)0, (int8_t)4, (int8_t)8, (int8_t)0);
    test_rescale((int8_t)-1, (int8_t)-5, (int8_t)10, (int8_t)7, (int8_t)-1);
    test_rescale((int8_t)0, (int8_t)-5, (int8_t)10, (int8_t)7, (int8_t)-1);
}


void test_rescale_s8_u16()
{  
    // Ascending
    test_rescale((int8_t)3, (int8_t)0, (int8_t)4, (uint16_t)1234, (uint16_t)33214);
    test_rescale((int8_t)-1, (int8_t)-5, (int8_t)10, (uint16_t)100, (uint16_t)1000);
    test_rescale((int8_t)0, (int8_t)-5, (int8_t)10, (uint16_t)0, (uint16_t)7);

    // Descending
    test_rescale((int8_t)3, (int8_t)0, (int8_t)4, (uint16_t)33214, (uint16_t)1234);
    test_rescale((int8_t)-1, (int8_t)-5, (int8_t)10, (uint16_t)1000, (uint16_t)100);
    test_rescale((int8_t)0, (int8_t)-5, (int8_t)10, (uint16_t)7, (uint16_t)1);
}

void test_rescale_clamp_min()
{
    TEST_ASSERT_EQUAL(33214, rescale((int8_t)-3, (int8_t)0, (int8_t)4, (uint16_t)33214, (uint16_t)1234));
}

void test_rescale_clamp_max()
{
    TEST_ASSERT_EQUAL(1234, rescale((int8_t)5, (int8_t)0, (int8_t)4, (uint16_t)33214, (uint16_t)1234));
}

void testrescale()
{
    RUN_TEST(test_rescale_s8_s8);
    RUN_TEST(test_rescale_s8_u16);
    RUN_TEST(test_rescale_clamp_min);
    RUN_TEST(test_rescale_clamp_max);
#if defined(ARDUINO)    
#endif
}