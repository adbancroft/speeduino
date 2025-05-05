
#include <Arduino.h>
#include <unity.h>
#include "../test_utils.h"
#include "scheduler.h"

#define TIMEOUT 1000
#define DURATION 1000

static void test_status_off_to_pending(FuelSchedule &schedule)
{
    resetFuelSchedulers();
    _setFuelScheduleDuration(schedule, TIMEOUT, DURATION);
    TEST_ASSERT_EQUAL(PENDING, schedule._status);
}

static void test_status_off_to_pending_inj1(void)
{
    test_status_off_to_pending(fuelSchedules[0]);
}

static void test_status_off_to_pending_inj2(void)
{
    test_status_off_to_pending(fuelSchedules[1]);
}

static void test_status_off_to_pending_inj3(void)
{
    test_status_off_to_pending(fuelSchedules[2]);
}

static void test_status_off_to_pending_inj4(void)
{
    test_status_off_to_pending(fuelSchedules[3]);
}

static void test_status_off_to_pending_inj5(void)
{
#if INJ_CHANNELS >= 5
    test_status_off_to_pending(fuelSchedules[4]);
#endif
}

static void test_status_off_to_pending_inj6(void)
{
#if INJ_CHANNELS >= 6
    test_status_off_to_pending(fuelSchedules[5]);
#endif
}

static void test_status_off_to_pending_inj7(void)
{
#if INJ_CHANNELS >= 7
    test_status_off_to_pending(fuelSchedules[6]);
#endif
}

static void test_status_off_to_pending_inj8(void)
{
#if INJ_CHANNELS >= 8
    test_status_off_to_pending(fuelSchedules[7]);
#endif
}

static void test_status_off_to_pending(IgnitionSchedule &schedule)
{
    resetIgnitionSchedulers();
    _setIgnitionScheduleDuration(schedule, TIMEOUT, DURATION);
    TEST_ASSERT_EQUAL(PENDING, schedule._status);
}

static void test_status_off_to_pending_ign1(void)
{
    test_status_off_to_pending(ignitionSchedules[0]);
}

static void test_status_off_to_pending_ign2(void)
{
    test_status_off_to_pending(ignitionSchedules[1]);
}

static void test_status_off_to_pending_ign3(void)
{
    test_status_off_to_pending(ignitionSchedules[2]);
}

static void test_status_off_to_pending_ign4(void)
{
    test_status_off_to_pending(ignitionSchedules[3]);
}

static void test_status_off_to_pending_ign5(void)
{
#if IGN_CHANNELS >= 5
    test_status_off_to_pending(ignitionSchedules[4]);
#endif
}

static void test_status_off_to_pending_ign6(void)
{
#if IGN_CHANNELS >= 6
    test_status_off_to_pending(ignitionSchedules[5]);
#endif
}

static void test_status_off_to_pending_ign7(void)
{
#if IGN_CHANNELS >= 7
    test_status_off_to_pending(ignitionSchedules[6]);
#endif
}

static void test_status_off_to_pending_ign8(void)
{
#if IGN_CHANNELS >= 8
    test_status_off_to_pending(ignitionSchedules[7]);
#endif
}

void test_status_off_to_pending(void)
{
  SET_UNITY_FILENAME() {

    RUN_TEST_P(test_status_off_to_pending_inj1);
    RUN_TEST_P(test_status_off_to_pending_inj2);
    RUN_TEST_P(test_status_off_to_pending_inj3);
    RUN_TEST_P(test_status_off_to_pending_inj4);
    RUN_TEST_P(test_status_off_to_pending_inj5);
    RUN_TEST_P(test_status_off_to_pending_inj6);
    RUN_TEST_P(test_status_off_to_pending_inj7);
    RUN_TEST_P(test_status_off_to_pending_inj8);

    RUN_TEST_P(test_status_off_to_pending_ign1);
    RUN_TEST_P(test_status_off_to_pending_ign2);
    RUN_TEST_P(test_status_off_to_pending_ign3);
    RUN_TEST_P(test_status_off_to_pending_ign4);
    RUN_TEST_P(test_status_off_to_pending_ign5);
    RUN_TEST_P(test_status_off_to_pending_ign6);
    RUN_TEST_P(test_status_off_to_pending_ign7);
    RUN_TEST_P(test_status_off_to_pending_ign8);
  }
}
