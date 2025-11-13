#include <unity.h>
#include "../test_utils.h"
#include "config_pages.h"

extern bool isBoostByGear(const config2 &page2, const config9 &page9);
extern uint8_t gearToBoostFactor(uint8_t gear, const config9 &page9);

static void test_isBoostByGear_disabled(void)
{
  config2 p2 = {};
  config9 p9 = {};
  p9.boostByGearEnabled = BOOST_BY_GEAR_OFF;
  p2.vssMode = 2; // vssMode > 1
  TEST_ASSERT_FALSE(isBoostByGear(p2, p9));
}

static void test_isBoostByGear_vssMode_too_low(void)
{
  config2 p2 = {};
  config9 p9 = {};
  p9.boostByGearEnabled = BOOST_BY_GEAR_MULTIPLIED;
  p2.vssMode = 1; // not > 1
  TEST_ASSERT_FALSE(isBoostByGear(p2, p9));
}

static void test_isBoostByGear_enabled(void)
{
  config2 p2 = {};
  config9 p9 = {};
  p2.vssMode = 2;
  p9.boostByGearEnabled = BOOST_BY_GEAR_MULTIPLIED;
  TEST_ASSERT_TRUE(isBoostByGear(p2, p9));
  p9.boostByGearEnabled = BOOST_BY_GEAR_CONSTANT;
  TEST_ASSERT_TRUE(isBoostByGear(p2, p9));
}

static void test_gearToBoostFactor_basic(void)
{
  config9 p9 = {};
  // Populate gear factors
  p9.boostByGear1 = 11;
  p9.boostByGear2 = 22;
  p9.boostByGear3 = 33;
  p9.boostByGear4 = 44;
  p9.boostByGear5 = 55;
  p9.boostByGear6 = 66;

  TEST_ASSERT_EQUAL_UINT8(11, gearToBoostFactor(1, p9));
  TEST_ASSERT_EQUAL_UINT8(22, gearToBoostFactor(2, p9));
  TEST_ASSERT_EQUAL_UINT8(33, gearToBoostFactor(3, p9));
  TEST_ASSERT_EQUAL_UINT8(44, gearToBoostFactor(4, p9));
  TEST_ASSERT_EQUAL_UINT8(55, gearToBoostFactor(5, p9));
  TEST_ASSERT_EQUAL_UINT8(66, gearToBoostFactor(6, p9));
  // out of range
  TEST_ASSERT_EQUAL_UINT8(0, gearToBoostFactor(0, p9));
  TEST_ASSERT_EQUAL_UINT8(0, gearToBoostFactor(7, p9));
}

void testBoost(void) {
  SET_UNITY_FILENAME() {
    RUN_TEST(test_isBoostByGear_disabled);
    RUN_TEST(test_isBoostByGear_vssMode_too_low);
    RUN_TEST(test_isBoostByGear_enabled);
    RUN_TEST(test_gearToBoostFactor_basic);
  }
}
