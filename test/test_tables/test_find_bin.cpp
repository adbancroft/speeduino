#include "find_bin.h"
#include <unity.h>

#define _countof(x) (sizeof(x) / sizeof (x[0]))

static uint8_t test_data_u8[] = {
    5, 23, 59, 101, 127, 167, 199, 211, 251,
};
static int16_t test_data_s16[] = {
    123, 2539, 5531, 7537, 11329, 16363, 21323, 26357, 32029,
};

void test_findBin_clamplower(void)
{
    uint8_t find_u8 = test_data_u8[0]-5;
    uint8_t result = find_bin(find_u8, test_data_u8, (uint8_t)0U, (uint8_t)(_countof(test_data_u8)-1), 2U);
    TEST_ASSERT_EQUAL(1, result);

    int16_t find_s16 = test_data_s16[0]-5;
    result = find_bin(find_s16, test_data_s16, (uint8_t)0U, (uint8_t)(_countof(test_data_s16)-1), 2U);
    TEST_ASSERT_EQUAL(1, result);
}


void test_findBin_clampupper(void)
{
    uint8_t find_u8 = test_data_u8[_countof(test_data_u8)-1]+1;
    uint8_t result = find_bin(find_u8, test_data_u8, (uint8_t)0U, (uint8_t)(_countof(test_data_u8)-1), 2U);
    TEST_ASSERT_EQUAL(_countof(test_data_u8)-1, result);

    int16_t find_s16 = test_data_s16[_countof(test_data_s16)-1]+5;
    result = find_bin(find_s16, test_data_s16, (uint8_t)0U, (uint8_t)(_countof(test_data_s16)-1), 2U);
    TEST_ASSERT_EQUAL(_countof(test_data_s16)-1, result);
}

void test_findBin_bin_edges(void)
{
    uint8_t find_u8 = test_data_u8[0];
    uint8_t result = find_bin(find_u8, test_data_u8, (uint8_t)0U, (uint8_t)(_countof(test_data_u8)-1), _countof(test_data_u8)-1);
    TEST_ASSERT_EQUAL(1U, result);

    for (uint8_t loop=1; loop!=_countof(test_data_u8)-2; ++loop) {
        result = find_bin(test_data_u8[loop], test_data_u8, (uint8_t)0U, (uint8_t)(_countof(test_data_u8)-1), _countof(test_data_u8)-1);
        TEST_ASSERT_EQUAL(loop, result);
    }

    find_u8 = test_data_u8[_countof(test_data_u8)-1];
    result = find_bin(find_u8, test_data_u8, (uint8_t)0U, (uint8_t)(_countof(test_data_u8)-1), 1U);
    TEST_ASSERT_EQUAL(_countof(test_data_u8)-1, result);
}

void test_findBin_bin_midpoints(void)
{
    for (uint8_t loop=0; loop!=_countof(test_data_u8)-2; ++loop) {
        uint8_t midPoint = test_data_u8[loop]+((test_data_u8[loop+1] - test_data_u8[loop])/2);
        uint8_t result = find_bin(midPoint, test_data_u8, (uint8_t)0U, (uint8_t)(_countof(test_data_u8)-1), _countof(test_data_u8)-1);
        TEST_ASSERT_EQUAL(loop+1, result);
    }
}

void testFindBin()
{
    RUN_TEST(test_findBin_clamplower);
    RUN_TEST(test_findBin_clampupper);
    RUN_TEST(test_findBin_bin_edges);
    RUN_TEST(test_findBin_bin_midpoints);

    // Note - no test against bad last index. find_bin() doesn't defend against this as it's too
    // large a performance overhead
}