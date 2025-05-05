#include <globals.h>
#include <speeduino.h>
#include <unity.h>
#include "../test_utils.h"
#include "scheduler.h"

extern void applyPWToSchedules(uint16_t primaryPW, uint16_t secondaryPW);

static void test_applyPWToSchedules_zero_zero(void)
{
  maxInjPrimaryOutputs = 0;
  maxInjSecondaryOutputs = 0;

  applyPWToSchedules(9000U, 1000U); //90% duty cycle at 6000rpm

  TEST_ASSERT_EQUAL(0, fuelSchedules[0].pw);
  TEST_ASSERT_EQUAL(0, fuelSchedules[1].pw);
  TEST_ASSERT_EQUAL(0, fuelSchedules[2].pw);
  TEST_ASSERT_EQUAL(0, fuelSchedules[3].pw);
}

static void test_applyPWToSchedules_two_zero(void)
{
  maxInjPrimaryOutputs = 2;
  maxInjSecondaryOutputs = 0;

  applyPWToSchedules(9000U, 1000U); //90% duty cycle at 6000rpm

  TEST_ASSERT_EQUAL(9000, fuelSchedules[0].pw);
  TEST_ASSERT_EQUAL(9000, fuelSchedules[1].pw);
  TEST_ASSERT_EQUAL(0, fuelSchedules[2].pw);
  TEST_ASSERT_EQUAL(0, fuelSchedules[3].pw);
}

static void test_applyPWToSchedules_zero_two(void)
{
  maxInjPrimaryOutputs = 0;
  maxInjSecondaryOutputs = 2;

  applyPWToSchedules(9000U, 1000U); //90% duty cycle at 6000rpm
  
  TEST_ASSERT_EQUAL(1000, fuelSchedules[0].pw);
  TEST_ASSERT_EQUAL(1000, fuelSchedules[1].pw);
  TEST_ASSERT_EQUAL(0, fuelSchedules[2].pw);
  TEST_ASSERT_EQUAL(0, fuelSchedules[3].pw);
}

void testStaging(void)
{
  SET_UNITY_FILENAME() {
    RUN_TEST(test_applyPWToSchedules_zero_zero);
    RUN_TEST(test_applyPWToSchedules_two_zero);
    RUN_TEST(test_applyPWToSchedules_zero_two);
  }
}
