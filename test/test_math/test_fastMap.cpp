#include <unity.h>
#include "fastMap.hpp"
#include "../timer.hpp"
#include "../test_utils.h"

template <typename T, typename U>
struct fast_map_test_data
{
  T inMin;
  T inMax;
  T inStep;
  bool invertT;
  U outMin;
  U outMax;
  bool invertU;
};

template <typename T, typename U>
static void test_fast_map(const fast_map_test_data<T, U> &test)
{
  for (T i = test.inMin; i <= test.inMax; i += test.inStep)
  {
    U expected = map(i, 
        test.invertT ? test.inMax : test.inMin, 
        test.invertT ? test.inMin : test.inMax, 
        test.invertU ? test.outMax : test.outMin,
        test.invertU ? test.outMin : test.outMax);
    U actual = fastMap(i, 
        test.invertT ? test.inMax : test.inMin, 
        test.invertT ? test.inMin : test.inMax, 
        test.invertU ? test.outMax : test.outMin,
        test.invertU ? test.outMin : test.outMax);
    TEST_ASSERT_EQUAL(expected, actual);
  }
}  

static void test_maths_fastMap_same_width_same_signedness_same_direction(void)
{
    const uint8_t inMin = 3;
    const uint8_t inMax = 233;
    const uint8_t outMin = 0;
    const uint8_t outMax = 255;

    {
      fast_map_test_data<uint8_t, uint8_t> test_params = {inMin, inMax, 1, false, outMin, outMax, false};
      test_fast_map(test_params);
    }
    {
      fast_map_test_data<uint8_t, uint8_t> test_params = {inMin, inMax, 1, true, outMin, outMax, true};
      test_fast_map(test_params);
    }
}

static void test_maths_fastMap_same_width_same_signedness_reverse_direction(void)
{
    const uint8_t inMin = 3;
    const uint8_t inMax = 233;
    const uint8_t outMin = 0;
    const uint8_t outMax = 255;

    {
        fast_map_test_data<uint8_t, uint8_t> test_params = {inMin, inMax, 1, false, outMin, outMax, true};
        test_fast_map(test_params);
    }

    {
        fast_map_test_data<uint8_t, uint8_t> test_params = {inMin, inMax, 1, true, outMin, outMax, false};
        test_fast_map(test_params);
    }
}

static void test_maths_fastMap_different_width_different_signedness_same_direction(void)
{
    const uint8_t inMin = 3;
    const uint8_t inMax = 233;
    const int16_t outMin = -23579;
    const int16_t outMax = -15973;

    {
      fast_map_test_data<uint8_t, int16_t> test_params = {inMin, inMax, 1, false, outMin, outMax, false};
      test_fast_map(test_params);
    }
    {
      fast_map_test_data<uint8_t, int16_t> test_params = {inMin, inMax, 1, true, outMin, outMax, true};
      test_fast_map(test_params);
    }
}

static void test_maths_fastMap_different_width_different_signedness_different_direction(void)
{
    const uint8_t inMin = 3;
    const uint8_t inMax = 233;
    const int16_t outMin = -23579;
    const int16_t outMax = -15973;

    {
        fast_map_test_data<uint8_t, int16_t> test_params = {inMin, inMax, 1, false, outMin, outMax, true};
        test_fast_map(test_params);
    }

    {
        fast_map_test_data<uint8_t, int16_t> test_params = {inMin, inMax, 1, true, outMin, outMax, false};
        test_fast_map(test_params);
    }
}

static void test_maths_fastMap_below_range(void)
{
  uint16_t actual = fastMap(1111, 1500, 11123, 1200, 5000);
  TEST_ASSERT_EQUAL(1200, actual);

  actual = fastMap(1111, 11123, 1500, 1200, 5000);
  TEST_ASSERT_EQUAL(5000, actual);
}

static void test_maths_fastMap_above_range(void)
{
  uint16_t actual = fastMap(15000, 1500, 11123, 1200, 5000);
  TEST_ASSERT_EQUAL(5000, actual);

  actual = fastMap(15000, 11123, 1500, 1200, 5000);
  TEST_ASSERT_EQUAL(1200, actual);
}

void test_maths_fastMap8_perf(void)
{
  const uint16_t iters = 50;
  const uint8_t inMin = 3;
  const uint8_t inMax = 233;
  const uint8_t step = 1;
  const uint8_t outMin = 0;
  const uint8_t outMax = 255;

  auto nativeTest = [] (uint8_t index, uint32_t &checkSum) { checkSum += map(index, inMin, inMax, outMin, outMax); };
  auto optimizedTest = [] (uint8_t index, uint32_t &checkSum) { checkSum += fastMap(index, inMin, inMax, outMin, outMax); };
  auto comparison = compare_executiontime<uint8_t, uint32_t>(iters, inMin, inMax, step, nativeTest, optimizedTest);
  
  // This also forces the compiler to run the loops above
  TEST_ASSERT_EQUAL(comparison.timeA.result, comparison.timeB.result);

  TEST_ASSERT_LESS_THAN(comparison.timeA.durationMicros, comparison.timeB.durationMicros);
}

void test_maths_fastMap8to16_perf(void)
{
  const uint16_t iters = 50;
  const uint8_t inMin = 3;
  const uint8_t inMax = 233;
  const uint8_t step = 1;
  const uint16_t outMin = 1521;
  const uint16_t outMax = 53333;

  auto nativeTest = [] (uint8_t index, uint32_t &checkSum) { checkSum += map(index, inMin, inMax, outMin, outMax); };
  auto optimizedTest = [] (uint8_t index, uint32_t &checkSum) { checkSum += fastMap(index, inMin, inMax, outMin, outMax); };
  auto comparison = compare_executiontime<uint8_t, uint32_t>(iters, inMin, inMax, step, nativeTest, optimizedTest);
  
  // This also forces the compiler to run the loops above
  TEST_ASSERT_EQUAL(comparison.timeA.result, comparison.timeB.result);

  TEST_ASSERT_LESS_THAN(comparison.timeA.durationMicros, comparison.timeB.durationMicros);
}

void test_maths_fastMap8_v_16perf(void)
{
  const uint16_t iters = 50;
  const uint8_t inMin = 3;
  const uint8_t inMax = 233;
  const uint8_t step = 1;
  const uint8_t outMin = 0;
  const uint8_t outMax = 255;

  auto nativeTest = [] (uint8_t index, uint32_t &checkSum) { checkSum += fastMap((uint16_t)index, (uint16_t)inMin, (uint16_t)inMax, (uint16_t)outMin, (uint16_t)outMax); };
  auto optimizedTest = [] (uint8_t index, uint32_t &checkSum) { checkSum += fastMap(index, inMin, inMax, outMin, outMax); };
  auto comparison = compare_executiontime<uint8_t, uint32_t>(iters, inMin, inMax, step, nativeTest, optimizedTest);
  
  // This also forces the compiler to run the loops above
  TEST_ASSERT_EQUAL(comparison.timeA.result, comparison.timeB.result);

  TEST_ASSERT_LESS_THAN(comparison.timeA.durationMicros, comparison.timeB.durationMicros);
}

void test_maths_fastMap16_perf(void)
{
  const uint16_t iters = 50;
  const uint16_t inMin = 1521;
  const uint16_t inMax = 53333;
  const uint16_t step = 331;
  const uint16_t outMin = 0;
  const uint16_t outMax = 1111;

  auto nativeTest = [] (uint16_t index, uint32_t &checkSum) { checkSum += map(index, inMin, inMax, outMin, outMax); };
  auto optimizedTest = [] (uint16_t index, uint32_t &checkSum) { checkSum += fastMap(index, inMin, inMax, outMin, outMax); };
  auto comparison = compare_executiontime<uint16_t, uint32_t>(iters, inMin, inMax, step, nativeTest, optimizedTest);
  
  // This also forces the compiler to run the loops above
  TEST_ASSERT_EQUAL(comparison.timeA.result, comparison.timeB.result);

  TEST_ASSERT_LESS_THAN(comparison.timeA.durationMicros, comparison.timeB.durationMicros);
}

void testFastMap(void) {
  SET_UNITY_FILENAME() {
    RUN_TEST(test_maths_fastMap_same_width_same_signedness_same_direction);
    RUN_TEST(test_maths_fastMap_same_width_same_signedness_reverse_direction);
    RUN_TEST(test_maths_fastMap_different_width_different_signedness_same_direction);
    RUN_TEST(test_maths_fastMap_different_width_different_signedness_different_direction);
    RUN_TEST(test_maths_fastMap_below_range);
    RUN_TEST(test_maths_fastMap_above_range);
    
    RUN_TEST(test_maths_fastMap8_perf);
    RUN_TEST(test_maths_fastMap16_perf);
    RUN_TEST(test_maths_fastMap8to16_perf);
    RUN_TEST(test_maths_fastMap8_v_16perf);
  }
}