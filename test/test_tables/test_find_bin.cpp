#include "find_bin.h"
#include <unity.h>

#define _countof(x) (sizeof(x) / sizeof (x[0]))

static uint8_t test_data_u8[] = {
    5, 23, 59, 101, 127, 167, 199, 211, 251,
};
static const uint8_t test_data_u8_start = 0U;
static const uint8_t test_data_u8_end = _countof(test_data_u8)-1;

static uint8_t test_data_u8_r[] = {
    251, 211, 199, 167, 127, 101, 59, 23, 5,
};
static const uint8_t test_data_u8_r_start = _countof(test_data_u8_r)-1;
static const uint8_t test_data_u8_r_end = 0U;

static int16_t test_data_s16[] = {
    123, 2539, 5531, 7537, 11329, 16363, 21323, 26357, 32029,
};
static const uint8_t test_data_s16_start = 0U;
static const uint8_t test_data_s16_end = _countof(test_data_s16)-1;
static int16_t test_data_s16_r[] = {
    32029, 26357, 21323, 16363, 11329, 7357, 5531, 2539, 123,
};
static const uint8_t test_data_s16_r_start = _countof(test_data_s16_r)-1;
static const uint8_t test_data_s16_r_end = 0U;

void test_findBin_clamplower(void)
{
    uint8_t find_u8 = test_data_u8[test_data_u8_start]-5;
    uint8_t result = find_bin(find_u8, test_data_u8, test_data_u8_start, test_data_u8_end, 2U);
    TEST_ASSERT_EQUAL_UINT8(1, result);

    int16_t find_s16 = test_data_s16[test_data_s16_start]-5;
    result = find_bin(find_s16, test_data_s16, test_data_s16_start, test_data_s16_end, 2U);
    TEST_ASSERT_EQUAL_UINT8(1, result);
}

void test_findBin_clamplower_reverse(void)
{
    uint8_t find_u8 = test_data_u8_r[test_data_u8_r_start]-5;
    uint8_t result = find_bin(find_u8, test_data_u8_r, test_data_u8_r_start, test_data_u8_r_end, 2U);
    TEST_ASSERT_EQUAL_UINT8(test_data_u8_r_start-1, result);

    int16_t find_s16 = test_data_s16_r[test_data_s16_r_start]-5;
    result = find_bin(find_s16, test_data_s16_r, test_data_s16_r_start, test_data_s16_r_end,  2U);
    TEST_ASSERT_EQUAL_UINT8(7, result);
}

void test_findBin_clampupper(void)
{
    uint8_t find_u8 = test_data_u8[test_data_u8_end]+1;
    uint8_t result = find_bin(find_u8, test_data_u8, test_data_u8_start, test_data_u8_end, 2U);
    TEST_ASSERT_EQUAL_UINT8(test_data_u8_end, result);

    int16_t find_s16 = test_data_s16[test_data_s16_end]+5;
    result = find_bin(find_s16, test_data_s16, test_data_s16_start, test_data_s16_end, 2U);
    TEST_ASSERT_EQUAL_UINT8(test_data_s16_end, result);    
}

void test_findBin_clampupper_reverse(void)
{
    uint8_t find_u8 = test_data_u8_r[test_data_u8_r_end]+1;
    uint8_t result = find_bin(find_u8, test_data_u8_r, test_data_u8_r_start, test_data_u8_r_end, 2U);
    TEST_ASSERT_EQUAL_UINT8(test_data_u8_r_end, result);    

    int16_t find_s16 = test_data_s16_r[test_data_s16_r_end]+5;
    result = find_bin(find_s16, test_data_s16_r, test_data_s16_r_start, test_data_s16_r_end, 2U);
    TEST_ASSERT_EQUAL_UINT8(test_data_s16_r_end, result);
}

void test_findBin_bin_edges(void)
{
    uint8_t find_u8 = test_data_u8[test_data_u8_start];
    uint8_t result = find_bin(find_u8, test_data_u8, test_data_u8_start, test_data_u8_end, 2U);
    TEST_ASSERT_EQUAL_UINT8(1U, result);

    for (uint8_t loop=test_data_u8_start+1; loop!=test_data_u8_end+1; ++loop) 
    {
        find_u8 = test_data_u8[loop];
        result = find_bin(find_u8, test_data_u8, test_data_u8_start, test_data_u8_end, 2U);
        TEST_ASSERT_EQUAL_UINT8(loop, result);
    } 
}

void test_findBin_bin_edges_reverse(void)
{
    uint8_t find_u8 = test_data_u8_r[test_data_u8_r_end];
    uint8_t result = find_bin(find_u8, test_data_u8_r, test_data_u8_r_start, test_data_u8_r_end, 2U);
    TEST_ASSERT_EQUAL_UINT8(test_data_u8_r_end, result);

    for (uint8_t loop=test_data_u8_r_start-1; loop!=test_data_u8_r_end; --loop) 
    {
        find_u8 = test_data_u8_r[loop];
        result = find_bin(find_u8, test_data_u8_r, test_data_u8_r_start, test_data_u8_r_end, 2U);
        TEST_ASSERT_EQUAL_UINT8(loop, result);
    }    
}

void test_findBin_bin_midpoints(void)
{
    for (uint8_t loop=0; loop!=_countof(test_data_u8)-2; ++loop) 
    {
        uint8_t midPoint = test_data_u8[loop]+((test_data_u8[loop+1] - test_data_u8[loop])/2);
        uint8_t result = find_bin(midPoint, test_data_u8, test_data_u8_start, test_data_u8_end, 2U);
        TEST_ASSERT_EQUAL_UINT8(loop+1, result);
    }
}

void testFindBin()
{
    RUN_TEST(test_findBin_clamplower);
    RUN_TEST(test_findBin_clamplower_reverse);
    RUN_TEST(test_findBin_clampupper);
    RUN_TEST(test_findBin_clampupper_reverse);
    RUN_TEST(test_findBin_bin_edges);
    RUN_TEST(test_findBin_bin_edges_reverse);
    RUN_TEST(test_findBin_bin_midpoints);

    // Note - no test against bad last index. find_bin() doesn't defend against this as it's too
    // large a performance overhead
}