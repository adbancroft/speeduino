#include <unity.h>
#include "test_table2d.h"
#include "table2d.h"
#include "../test_utils.h"

typedef uint8_t byte;

static constexpr uint8_t TEST_TABLE2D_SIZE = 9;
static uint8_t table2d_data_u8[TEST_TABLE2D_SIZE] = {
    251, 211, 199, 167, 127, 101, 59, 23, 5
};
static int16_t table2d_data_s16[TEST_TABLE2D_SIZE] = {
    32029, 26357, 21323, 16363, 11329, 7537, 5531, 2539, 1237
};

static uint8_t table2d_axis_u8[TEST_TABLE2D_SIZE] {
    5, 23, 59, 101, 127, 167, 199, 211, 251,
};
static int16_t table2d_axis_s16[TEST_TABLE2D_SIZE] = {
    123, 2539, 5531, 7537, 11329, 16363, 21323, 26357, 32029,
};

static table2d<uint8_t, uint8_t, TEST_TABLE2D_SIZE> table2d_u8_u8(&table2d_axis_u8, &table2d_data_u8);
static table2d<int16_t, uint8_t, TEST_TABLE2D_SIZE> table2d_u8_s16(&table2d_axis_s16, &table2d_data_u8);
static table2d<uint8_t, int16_t, TEST_TABLE2D_SIZE> table2d_s16_u8(&table2d_axis_u8, &table2d_data_s16);
static table2d<int16_t, int16_t, TEST_TABLE2D_SIZE> table2d_s16_s16(&table2d_axis_s16, &table2d_data_s16);

void test_table2dLookup_50pct(void)
{
    uint8_t u8_u8_result = table2D_getValue(&table2d_u8_u8, intermediate(table2d_axis_u8[3], table2d_axis_u8[4], 50));
    TEST_ASSERT_EQUAL(147, u8_u8_result);
    TEST_ASSERT_EQUAL(4, table2d_u8_u8.cache.lastXMax);

    uint8_t u8_s16_result = table2D_getValue(&table2d_u8_s16, intermediate(table2d_axis_s16[6], table2d_axis_s16[7], 50));
    TEST_ASSERT_EQUAL(41, u8_s16_result);
    TEST_ASSERT_EQUAL(7, table2d_u8_s16.cache.lastXMax);

    int16_t s16_u8_result = table2D_getValue(&table2d_s16_u8, intermediate(table2d_axis_u8[3], table2d_axis_u8[4], 50));
    TEST_ASSERT_EQUAL(13846, s16_u8_result);
    TEST_ASSERT_EQUAL(4, table2d_s16_u8.cache.lastXMax);

    int16_t s16_s16_result = table2D_getValue(&table2d_s16_s16, intermediate(table2d_axis_s16[3], table2d_axis_s16[4], 50));
    TEST_ASSERT_EQUAL(13846, s16_s16_result);
    TEST_ASSERT_EQUAL(4, table2d_s16_s16.cache.lastXMax);
}


void test_table2dLookup_33pct(void)
{
    static constexpr uint8_t binPct = 33U;

    uint8_t u8_u8_result = table2D_getValue(&table2d_u8_u8, intermediate(table2d_axis_u8[3], table2d_axis_u8[4], binPct));
    TEST_ASSERT_EQUAL(154, u8_u8_result);
    TEST_ASSERT_EQUAL(4, table2d_u8_u8.cache.lastXMax);

    uint8_t u8_s16_result = table2D_getValue(&table2d_u8_s16, intermediate(table2d_axis_s16[6], table2d_axis_s16[7], binPct));
    TEST_ASSERT_EQUAL(48, u8_s16_result);
    TEST_ASSERT_EQUAL(7, table2d_u8_s16.cache.lastXMax);

    int16_t s16_u8_result = table2D_getValue(&table2d_s16_u8, intermediate(table2d_axis_u8[3], table2d_axis_u8[4], binPct));
    TEST_ASSERT_EQUAL(14621, s16_u8_result);
    TEST_ASSERT_EQUAL(4, table2d_s16_u8.cache.lastXMax);

    int16_t s16_s16_result = table2D_getValue(&table2d_s16_s16, intermediate(table2d_axis_s16[3], table2d_axis_s16[4], binPct));
    TEST_ASSERT_EQUAL(14703, s16_s16_result);
    TEST_ASSERT_EQUAL(4, table2d_s16_s16.cache.lastXMax);
}

void test_table2dLookup_66pct(void)
{
    static constexpr uint8_t binPct = 66U;

    uint8_t u8_u8_result = table2D_getValue(&table2d_u8_u8, intermediate(table2d_axis_u8[3], table2d_axis_u8[4], binPct));
    TEST_ASSERT_EQUAL(141, u8_u8_result);
    TEST_ASSERT_EQUAL(4, table2d_u8_u8.cache.lastXMax);

    uint8_t u8_s16_result = table2D_getValue(&table2d_u8_s16, intermediate(table2d_axis_s16[6], table2d_axis_s16[7], binPct));
    TEST_ASSERT_EQUAL(36, u8_s16_result);
    TEST_ASSERT_EQUAL(7, table2d_u8_s16.cache.lastXMax);

    int16_t s16_u8_result = table2D_getValue(&table2d_s16_u8, intermediate(table2d_axis_u8[3], table2d_axis_u8[4], binPct));
    TEST_ASSERT_EQUAL(13072, s16_u8_result);
    TEST_ASSERT_EQUAL(4, table2d_s16_u8.cache.lastXMax);

    int16_t s16_s16_result = table2D_getValue(&table2d_s16_s16, intermediate(table2d_axis_s16[3], table2d_axis_s16[4], binPct));
    TEST_ASSERT_EQUAL(13041, s16_s16_result);
    TEST_ASSERT_EQUAL(4, table2d_s16_s16.cache.lastXMax);
}

void test_table2dLookup_exactAxis(void)
{
    uint8_t u8_u8_result = table2D_getValue(&table2d_u8_u8, table2d_axis_u8[7]);
    TEST_ASSERT_EQUAL(23, u8_u8_result);
    TEST_ASSERT_EQUAL(7, table2d_u8_u8.cache.lastXMax);

    uint8_t u8_s16_result = table2D_getValue(&table2d_u8_s16, table2d_axis_s16[1]);
    TEST_ASSERT_EQUAL(211, u8_s16_result);
    TEST_ASSERT_EQUAL(1, table2d_u8_s16.cache.lastXMax);

    int16_t s16_u8_result = table2D_getValue(&table2d_s16_u8, table2d_axis_u8[2]);
    TEST_ASSERT_EQUAL(21323, s16_u8_result);
    TEST_ASSERT_EQUAL(2, table2d_s16_u8.cache.lastXMax);

    int16_t s16_s16_result = table2D_getValue(&table2d_s16_s16, table2d_axis_s16[5]);
    TEST_ASSERT_EQUAL(7537, s16_s16_result);
    TEST_ASSERT_EQUAL(5, table2d_s16_s16.cache.lastXMax);    
}

void test_table2dLookup_overMax(void)
{
    uint8_t u8_u8_result = table2D_getValue(&table2d_u8_u8, (uint8_t)(table2d_axis_u8[TEST_TABLE2D_SIZE-1]+1));
    TEST_ASSERT_EQUAL(5, u8_u8_result);
    TEST_ASSERT_EQUAL(8, table2d_u8_u8.cache.lastXMax);

    uint8_t u8_s16_result = table2D_getValue(&table2d_u8_s16, (int16_t)(table2d_axis_s16[TEST_TABLE2D_SIZE-1]+1));
    TEST_ASSERT_EQUAL(5, u8_s16_result);
    TEST_ASSERT_EQUAL(8, table2d_u8_s16.cache.lastXMax);

    int16_t s16_u8_result = table2D_getValue(&table2d_s16_u8, (uint8_t)(table2d_axis_u8[TEST_TABLE2D_SIZE-1]+1));
    TEST_ASSERT_EQUAL(1237, s16_u8_result);
    TEST_ASSERT_EQUAL(8, table2d_s16_u8.cache.lastXMax);

    int16_t s16_s16_result = table2D_getValue(&table2d_s16_s16, (int16_t)(table2d_axis_s16[TEST_TABLE2D_SIZE-1]+1));
    TEST_ASSERT_EQUAL(1237, s16_s16_result);
    TEST_ASSERT_EQUAL(8, table2d_s16_s16.cache.lastXMax);    
}

void test_table2dLookup_underMin(void)
{
    uint8_t u8_u8_result = table2D_getValue(&table2d_u8_u8, (uint8_t)(table2d_axis_u8[0]-1));
    TEST_ASSERT_EQUAL(251, u8_u8_result);
    TEST_ASSERT_EQUAL(1, table2d_u8_u8.cache.lastXMax);

    uint8_t u8_s16_result = table2D_getValue(&table2d_u8_s16, (int16_t)(table2d_axis_s16[0]-1));
    TEST_ASSERT_EQUAL(251, u8_s16_result);
    TEST_ASSERT_EQUAL(1, table2d_u8_s16.cache.lastXMax);

    int16_t s16_u8_result = table2D_getValue(&table2d_s16_u8, (uint8_t)(table2d_axis_u8[0]-1));
    TEST_ASSERT_EQUAL(32029, s16_u8_result);
    TEST_ASSERT_EQUAL(1, table2d_s16_u8.cache.lastXMax);

    int16_t s16_s16_result = table2D_getValue(&table2d_s16_s16, (int16_t)(table2d_axis_s16[0]-1));
    TEST_ASSERT_EQUAL(32029, s16_s16_result);
    TEST_ASSERT_EQUAL(1, table2d_s16_s16.cache.lastXMax);       
}


void test_table2dLookup_mismatch_lookup_type(void)
{
    {
        int16_t lookupValue_s16 = INT16_MIN/2;
        uint8_t u8_u8_result = table2D_getValue(&table2d_u8_u8, lookupValue_s16);
        TEST_ASSERT_EQUAL(table2d_u8_u8.values[0], u8_u8_result);
        TEST_ASSERT_EQUAL(1, table2d_u8_u8.cache.lastXMax);
    }

    {
        uint16_t lookupValue_u16 = UINT16_MAX;
        int16_t s16_s16_result = table2D_getValue(&table2d_s16_s16, lookupValue_u16);
        TEST_ASSERT_EQUAL(table2d_s16_s16.values[TEST_TABLE2D_SIZE-1], s16_s16_result);
        TEST_ASSERT_EQUAL(TEST_TABLE2D_SIZE-1, table2d_s16_s16.cache.lastXMax);      
    }
}

void test_table2d_all_decrementing(void)
{
    {
        uint8_t u8_u8_result_last = UINT8_MAX;
        for (uint8_t loop=table2d_axis_u8[0]; loop<=table2d_axis_u8[TEST_TABLE2D_SIZE-1]; ++loop)
        {
            uint8_t u8_u8_result = table2D_getValue(&table2d_u8_u8, loop);
            TEST_ASSERT_LESS_OR_EQUAL(u8_u8_result_last, u8_u8_result);
            u8_u8_result_last = u8_u8_result;
        }
    }

    {
        int16_t s16_s16_result_last = INT16_MAX;
        for (int16_t loop=table2d_axis_s16[0]; loop<=table2d_axis_s16[TEST_TABLE2D_SIZE-1]; loop+=3)
        {
            int16_t s16_s16_result = table2D_getValue(&table2d_s16_s16, loop);
            TEST_ASSERT_LESS_OR_EQUAL(s16_s16_result_last, s16_s16_result);
            s16_s16_result_last = s16_s16_result;
        }
    }
}

#include "../timer.hpp"


static void test_lookup_perf(void) {
    uint16_t iters = 32;
    uint8_t start_index = 3;
    uint8_t end_index = 255;
    uint8_t step = 1;

    timer timerA;
    uint32_t paramA = 0;
    auto nativeTest = [] (uint8_t index, uint32_t &checkSum) { 
        checkSum += table2D_getValue(&table2d_u8_u8, (uint8_t)(table2d_axis_u8[TEST_TABLE2D_SIZE-1]+1));
    };
    measure_executiontime<uint8_t, uint32_t&>(iters, start_index, end_index, step, timerA, paramA, nativeTest);

    // The checksums will be different due to rounding. This is only
    // here to force the compiler to run the loops above
    TEST_ASSERT_INT32_WITHIN(UINT32_MAX/2, UINT32_MAX/2, paramA);

    char buffer[128];
    sprintf(buffer, "Timing: %" PRIu32, timerA.duration_micros());
    TEST_MESSAGE(buffer);

}

void testTable2d()
{
  SET_UNITY_FILENAME() {

    RUN_TEST(test_table2dLookup_50pct);
    RUN_TEST(test_table2dLookup_33pct);
    RUN_TEST(test_table2dLookup_66pct);
    RUN_TEST(test_table2dLookup_exactAxis);
    RUN_TEST(test_table2dLookup_overMax);
    RUN_TEST(test_table2dLookup_underMin);
    RUN_TEST(test_table2d_all_decrementing); 
    RUN_TEST(test_table2dLookup_mismatch_lookup_type);
    RUN_TEST(test_lookup_perf);
  }
}