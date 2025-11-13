#include <unity.h>
#include "../test_utils.h"
#include "config_pages.h"
#include "statuses.h"
#include "globals.h"

extern bool isBoostByGear(const config2 &page2, const config9 &page9);
extern uint8_t gearToBoostFactor(uint8_t gear, const config9 &page9);
extern uint16_t calcBoostByGearDuty(const statuses &current, const config9 &page9);
extern uint16_t calcBoostByGearTarget(const statuses &current, const config9 &page9);
extern uint16_t calcOLBoostDuty(const statuses &current, const config2 &page2, const config9 &page9);

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

  TEST_ASSERT_EQUAL(11, gearToBoostFactor(1, p9));
  TEST_ASSERT_EQUAL(22, gearToBoostFactor(2, p9));
  TEST_ASSERT_EQUAL(33, gearToBoostFactor(3, p9));
  TEST_ASSERT_EQUAL(44, gearToBoostFactor(4, p9));
  TEST_ASSERT_EQUAL(55, gearToBoostFactor(5, p9));
  TEST_ASSERT_EQUAL(66, gearToBoostFactor(6, p9));
  // out of range
  TEST_ASSERT_EQUAL(0, gearToBoostFactor(0, p9));
  TEST_ASSERT_EQUAL(0, gearToBoostFactor(7, p9));
}

static void test_calcBoostByGearDuty_constant(void)
{
  statuses cur = {};
  config9 p9 = {};

  p9.boostByGearEnabled = BOOST_BY_GEAR_CONSTANT;
  p9.boostByGear1 = 5; // factor

  cur.gear = 1;
  TEST_ASSERT_EQUAL(1000, calcBoostByGearDuty(cur, p9)); // 5 * 2 * 100 = 1000

  cur.gear = 7; // out of range
  TEST_ASSERT_EQUAL(0, calcBoostByGearDuty(cur, p9));
}

static void test_calcBoostByGearDuty_multiplied(void)
{
    // Set table to known value and verify multiplied calculation
    fill_table_values(boostTable, (table3d_value_t)7); // every cell = 7

    statuses cur = {};
    config9 p9 = {};
    p9.boostByGearEnabled = BOOST_BY_GEAR_MULTIPLIED;
    p9.boostByGear1 = 3; // factor

    cur.gear = 1;
    cur.TPS = 0; // lookup uses current.TPS * 2 -> 0
    cur.RPM = 0;

    // expected = gearFactor * lookupBoostTarget * 4 = 3 * 7 * 4 = 84
    TEST_ASSERT_EQUAL(84, calcBoostByGearDuty(cur, p9));

    // if lookup is zero, result should be zero as well
    fill_table_values(boostTable, (table3d_value_t)0);
    TEST_ASSERT_EQUAL(0, calcBoostByGearDuty(cur, p9));
}


static void test_calcBoostByGearTarget_constant(void)
{
  statuses cur = {};
  config9 p9 = {};

  p9.boostByGearEnabled = BOOST_BY_GEAR_CONSTANT;
  p9.boostByGear1 = 5; // factor

  cur.gear = 1;
  TEST_ASSERT_EQUAL(10, calcBoostByGearTarget(cur, p9)); // 5 * 2 = 10
}

static void test_calcBoostByGearTarget_multiplied(void)
{
  // Choose values so (gearFactor * lookup) is divisible by 100 to avoid truncation
  fill_table_values(boostTable, (table3d_value_t)25); // every cell = 25

  statuses cur = {};
  config9 p9 = {};
  p9.boostByGearEnabled = BOOST_BY_GEAR_MULTIPLIED;
  p9.boostByGear1 = 4; // factor

  cur.gear = 1;
  cur.TPS = 0;
  cur.RPM = 0;

  // expected = ((4*25)/100) * 4 = (100/100)*4 = 4
  TEST_ASSERT_EQUAL(4, calcBoostByGearTarget(cur, p9));
}


static void test_calcOLBoostDuty_boostByGear(void)
{
    statuses cur = {};
    config2 p2 = {};
    config9 p9 = {};

    // make isBoostByGear() true
    p2.vssMode = 2;
    p9.boostByGearEnabled = BOOST_BY_GEAR_CONSTANT;
    p9.boostByGear1 = 6;

    cur.gear = 1;

    // calcOLBoostDuty should delegate to calcBoostByGearDuty in this case
    TEST_ASSERT_EQUAL(1200, calcOLBoostDuty(cur, p2, p9)); // 6 * 2 * 100 = 1200
}

static void test_calcOLBoostDuty_openLoop(void)
{
    statuses cur = {};
    config2 p2 = {};
    config9 p9 = {};

    // make isBoostByGear() false
    p2.vssMode = 0;
    p9.boostByGearEnabled = BOOST_BY_GEAR_OFF;

    // populate boostTable with known value
    fill_table_values(boostTable, (table3d_value_t)9);

    cur.TPS = 0;
    cur.RPM = 0;

    // expected = lookup * 2 * 100 = 9 * 2 * 100 = 1800
    TEST_ASSERT_EQUAL(1800, calcOLBoostDuty(cur, p2, p9));
}



void testBoost(void) {
  SET_UNITY_FILENAME() {
    RUN_TEST(test_isBoostByGear_disabled);
    RUN_TEST(test_isBoostByGear_vssMode_too_low);
    RUN_TEST(test_isBoostByGear_enabled);
    RUN_TEST(test_gearToBoostFactor_basic);
    RUN_TEST(test_calcBoostByGearDuty_constant);
    RUN_TEST(test_calcBoostByGearDuty_multiplied);
    RUN_TEST(test_calcBoostByGearTarget_constant);
    RUN_TEST(test_calcBoostByGearTarget_multiplied);
    RUN_TEST(test_calcOLBoostDuty_boostByGear);
    RUN_TEST(test_calcOLBoostDuty_openLoop);
  }
}
