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
extern int16_t lookupFlexBoostCorrection(const statuses &current, const config2 &page2);
extern uint16_t getCLBoostTarget(const statuses &current, const config2 &page2, const config9 &page9);
extern bool isBoostControlEnabled(const statuses &current, const config15 &page15);
extern uint16_t boostTargetToDuty(uint16_t target, const statuses &current, const config2 &page2, const config6 &page6, const config10 &page10);
extern uint16_t calcCLBoostDuty(statuses &current, const config2 &page2, const config6 &page6, const config9 &page9, const config10 &page10, const config15 &page15);

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

static void test_isBoostControlEnabled_baro(void)
{
  statuses cur = {};
  config15 p15 = {};

  // Baro mode
  p15.boostControlEnable = EN_BOOST_CONTROL_BARO;

  // MAP >= baro -> enabled
  cur.baro = 50;
  cur.MAP = 50;
  TEST_ASSERT_TRUE(isBoostControlEnabled(cur, p15));

  // MAP < baro -> disabled
  cur.MAP = 49;
  TEST_ASSERT_FALSE(isBoostControlEnabled(cur, p15));
}

static void test_isBoostControlEnabled_fixed(void)
{
  statuses cur = {};
  config15 p15 = {};

  // Fixed threshold mode
  p15.boostControlEnable = EN_BOOST_CONTROL_FIXED;
  p15.boostControlEnableThreshold = 60; // threshold

  // MAP >= threshold -> enabled
  cur.MAP = 60;
  TEST_ASSERT_TRUE(isBoostControlEnabled(cur, p15));

  // MAP < threshold -> disabled
  cur.MAP = 59;
  TEST_ASSERT_FALSE(isBoostControlEnabled(cur, p15));
}

static void test_boostTargetToDuty_zero(void)
{
  statuses cur = {};
  config2 p2 = {};
  config6 p6 = {};
  config10 p10 = {};

  uint16_t res = boostTargetToDuty(0U, cur, p2, p6, p10);
  TEST_ASSERT_EQUAL(0, res);
}

static void test_boostTargetToDuty_positive(void)
{
  statuses cur = {};
  config2 p2 = {};
  config6 p6 = {};
  config10 p10 = {};

  // Give current a known boostDuty that would be used as fallback by the PID path if Compute() does not run
  cur.boostDuty = 1234U;
  // Call with a small positive target; exact output depends on PID internals, so assert general bounds
  uint16_t res = boostTargetToDuty(5U, cur, p2, p6, p10);
  // Result should be a valid duty between 0 and 10000 (0% - 100% with 2 decimal places)
  TEST_ASSERT_TRUE(res <= 10000U);
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

static void test_lookupFlexBoostCorrection_disabled(void)
{
  statuses cur = {};
  config2 p2 = {};

  // flex disabled -> correction should be zero
  p2.flexEnabled = 0;
  cur.ethanolPct = 40; // arbitrary
  TEST_ASSERT_EQUAL(0, lookupFlexBoostCorrection(cur, p2));
}

static void test_lookupFlexBoostCorrection_enabled(void)
{
  statuses cur = {};
  config2 p2 = {};

  // Enable flex correction
  p2.flexEnabled = 1;

  // Set bins so that ethanolPct == 40 maps exactly to bin index 2
  configPage10.flexBoostBins[0] = 0;
  configPage10.flexBoostBins[1] = 20;
  configPage10.flexBoostBins[2] = 40;
  configPage10.flexBoostBins[3] = 60;
  configPage10.flexBoostBins[4] = 80;
  configPage10.flexBoostBins[5] = 100;

  // Populate corresponding adj values and pick index 2 value
  for(int i=0;i<6;i++) { configPage10.flexBoostAdj[i] = (int16_t)((i+1)*10); }

  cur.ethanolPct = 40; // exact match to bin 2 -> value should be flexBoostAdj[2] == 30

  TEST_ASSERT_EQUAL_INT16((int16_t)30, lookupFlexBoostCorrection(cur, p2));
}


static void test_getCLBoostTarget_boostByGear(void)
{
  statuses cur = {};
  config2 p2 = {};
  config9 p9 = {};

  // Make boost-by-gear active
  p2.vssMode = 2;
  p9.boostByGearEnabled = BOOST_BY_GEAR_CONSTANT;
  p9.boostByGear1 = 7; // factor

  cur.gear = 1;

  // For constant mode, calcBoostByGearTarget returns factor * 2
  TEST_ASSERT_EQUAL(14, getCLBoostTarget(cur, p2, p9));
}

static void test_getCLBoostTarget_openLoop(void)
{
  statuses cur = {};
  config2 p2 = {};
  config9 p9 = {};

  // Ensure boost-by-gear disabled
  p2.vssMode = 0;
  p9.boostByGearEnabled = BOOST_BY_GEAR_OFF;

  // Populate boostTable lookup value
  fill_table_values(boostTable, (table3d_value_t)9);

  cur.TPS = 0;
  cur.RPM = 0;

  // Expect lookup << 1 = 9 * 2 = 18
  TEST_ASSERT_EQUAL(18, getCLBoostTarget(cur, p2, p9));
}

static void test_calcCLBoostDuty_disabled(void)
{
  statuses cur = {};
  config2 p2 = {};
  config6 p6 = {};
  config9 p9 = {};
  config10 p10 = {};
  config15 p15 = {};

  // Make boost control disabled by baro: MAP < baro
  cur.MAP = 10;
  cur.baro = 20;
  p15.boostControlEnable = EN_BOOST_CONTROL_BARO;

  // Set global disabled duty
  configPage15.boostDCWhenDisabled = 77;

  TEST_ASSERT_EQUAL( (uint16_t)(configPage15.boostDCWhenDisabled * 100U), calcCLBoostDuty(cur, p2, p6, p9, p10, p15));
}

static void test_calcCLBoostDuty_enabled_zero_target(void)
{
  statuses cur = {};
  config2 p2 = {};
  config6 p6 = {};
  config9 p9 = {};
  config10 p10 = {};
  config15 p15 = {};

  // Enable boost control (baro mode) and ensure MAP >= baro
  p15.boostControlEnable = EN_BOOST_CONTROL_BARO;
  cur.baro = 50;
  cur.MAP = 50;

  // Make open-loop lookup return zero
  p2.vssMode = 0; // ensure not boost-by-gear
  p9.boostByGearEnabled = BOOST_BY_GEAR_OFF;
  fill_table_values(boostTable, (table3d_value_t)0);

  uint16_t res = calcCLBoostDuty(cur, p2, p6, p9, p10, p15);

  // With zero target, boostTargetToDuty should return 0
  TEST_ASSERT_EQUAL(0, res);
  TEST_ASSERT_EQUAL(0, cur.flexBoostCorrection);
  TEST_ASSERT_EQUAL(0, cur.boostTarget);
}

static void test_calcCLBoostDuty_enabled_with_flex(void)
{
  statuses cur = {};
  config2 p2 = {};
  config6 p6 = {};
  config9 p9 = {};
  config10 p10 = {};
  config15 p15 = {};

  // Enable boost control (baro mode)
  p15.boostControlEnable = EN_BOOST_CONTROL_BARO;
  cur.baro = 50;
  cur.MAP = 50;

  // Ensure open-loop lookup returns a known base target (5 -> <<1 == 10)
  p2.vssMode = 0;
  p9.boostByGearEnabled = BOOST_BY_GEAR_OFF;
  fill_table_values(boostTable, (table3d_value_t)5);

  // Enable flex and set bins/adj so ethanolPct maps to a small correction
  p2.flexEnabled = 1;
  configPage10.flexBoostBins[0] = 0;
  configPage10.flexBoostAdj[0] = -15;
  configPage10.flexBoostBins[1] = 20;
  configPage10.flexBoostAdj[1] = 0;
  configPage10.flexBoostBins[2] = 40;
  configPage10.flexBoostAdj[2] = 30;
  configPage10.flexBoostBins[3] = 60;
  configPage10.flexBoostAdj[3] = 30;
  configPage10.flexBoostBins[4] = 80;
  configPage10.flexBoostAdj[4] = 30;
  configPage10.flexBoostBins[5] = 100;
  configPage10.flexBoostAdj[5] = 30;

  cur.ethanolPct = 40; // matches index 2 -> adj == 30

  uint16_t res = calcCLBoostDuty(cur, p2, p6, p9, p10, p15);

  // Verify flex correction and computed boost target are set as expected
  TEST_ASSERT_EQUAL(30, cur.flexBoostCorrection);
  // base lookup <<1 = 5<<1 = 10, plus flex 30 => 40
  TEST_ASSERT_EQUAL(40, cur.boostTarget);
  TEST_ASSERT_EQUAL(2000, res);

  cur.ethanolPct = 0; // matches index 0 -> adj == -15
  res = calcCLBoostDuty(cur, p2, p6, p9, p10, p15);
  TEST_ASSERT_EQUAL(-15, cur.flexBoostCorrection);
  TEST_ASSERT_EQUAL(0, cur.boostTarget); // Clamped to 0
  TEST_ASSERT_EQUAL(0, res);
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
    RUN_TEST(test_lookupFlexBoostCorrection_disabled);
    RUN_TEST(test_lookupFlexBoostCorrection_enabled);
    RUN_TEST(test_getCLBoostTarget_boostByGear);
    RUN_TEST(test_getCLBoostTarget_openLoop);
    RUN_TEST(test_isBoostControlEnabled_baro);
    RUN_TEST(test_isBoostControlEnabled_fixed);
    RUN_TEST(test_boostTargetToDuty_zero);
    RUN_TEST(test_boostTargetToDuty_positive);
    RUN_TEST(test_calcCLBoostDuty_disabled);
    RUN_TEST(test_calcCLBoostDuty_enabled_zero_target);
    RUN_TEST(test_calcCLBoostDuty_enabled_with_flex);
  }
}
