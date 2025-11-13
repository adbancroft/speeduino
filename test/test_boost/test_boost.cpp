#include <unity.h>
#include "../test_utils.h"
#include "config_pages.h"

extern bool isBoostByGear(const config2 &page2, const config9 &page9);

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

void testBoost(void) {
  SET_UNITY_FILENAME() {
    RUN_TEST(test_isBoostByGear_disabled);
    RUN_TEST(test_isBoostByGear_vssMode_too_low);
    RUN_TEST(test_isBoostByGear_enabled);
  }
}
