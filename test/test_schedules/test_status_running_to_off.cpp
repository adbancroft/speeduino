
#include <Arduino.h>
#include <unity.h>
#include "../test_utils.h"
#include "scheduler.h"

#define TIMEOUT 1000
#define DURATION 1000

static void test_status_running_to_off(FuelSchedule &schedule)
{
    resetFuelSchedulers();
    _setFuelScheduleDuration(schedule, TIMEOUT, DURATION);
    while( (schedule._status == PENDING) || (schedule._status == RUNNING) ) /*Wait*/ ;
    TEST_ASSERT_EQUAL(OFF, schedule._status);
}

static void test_status_running_to_off_fuel1(void)
{
    test_status_running_to_off(fuelSchedules[0]);
}

static void test_status_running_to_off_fuel2(void)
{
    test_status_running_to_off(fuelSchedules[1]);
}

static void test_status_running_to_off_fuel3(void)
{
    test_status_running_to_off(fuelSchedules[2]);
}

static void test_status_running_to_off_fuel4(void)
{
    test_status_running_to_off(fuelSchedules[3]);
}

static void test_status_running_to_off_fuel5(void)
{
#if INJ_CHANNELS >= 5
    test_status_running_to_off(fuelSchedules[4]);
#endif
}

static void test_status_running_to_off_fuel6(void)
{
#if INJ_CHANNELS >= 6
    test_status_running_to_off(fuelSchedules[5]);
#endif
}

static void test_status_running_to_off_fuel7(void)
{
#if INJ_CHANNELS >= 7
    test_status_running_to_off(fuelSchedules[6]);
#endif
}

static void test_status_running_to_off_fuel8(void)
{
#if INJ_CHANNELS >= 8
    test_status_running_to_off(fuelSchedules[7]);
#endif
}

static void test_status_running_to_off(IgnitionSchedule &schedule)
{
    resetIgnitionSchedulers();
    
    _setIgnitionScheduleDuration(schedule, TIMEOUT, DURATION);
    while( (schedule._status == PENDING) || (schedule._status == RUNNING) ) /*Wait*/ ;
    TEST_ASSERT_EQUAL(OFF, schedule._status);
}


static void test_status_running_to_off_ign1(void)
{
    test_status_running_to_off(ignitionSchedules[0]);
}

static void test_status_running_to_off_ign2(void)
{
    test_status_running_to_off(ignitionSchedules[1]);
}

static void test_status_running_to_off_ign3(void)
{
    test_status_running_to_off(ignitionSchedules[2]);
}

static void test_status_running_to_off_ign4(void)
{
    test_status_running_to_off(ignitionSchedules[3]);
}

static void test_status_running_to_off_ign5(void)
{
#if IGN_CHANNELS >= 5
    test_status_running_to_off(ignitionSchedules[4]);
#endif
}

static void test_status_running_to_off_ign6(void)
{
#if IGN_CHANNELS >= 6
    test_status_running_to_off(ignitionSchedules[5]);
#endif
}

static void test_status_running_to_off_ign7(void)
{
#if IGN_CHANNELS >= 7
    test_status_running_to_off(ignitionSchedules[6]);
#endif
}

static void test_status_running_to_off_ign8(void)
{
#if IGN_CHANNELS >= 8
    test_status_running_to_off(ignitionSchedules[7]);
#endif
}

void test_status_running_to_off(void)
{
  SET_UNITY_FILENAME() {

    RUN_TEST_P(test_status_running_to_off_fuel1);
    RUN_TEST_P(test_status_running_to_off_fuel2);
    RUN_TEST_P(test_status_running_to_off_fuel3);
    RUN_TEST_P(test_status_running_to_off_fuel4);
    RUN_TEST_P(test_status_running_to_off_fuel5);
    RUN_TEST_P(test_status_running_to_off_fuel6);
    RUN_TEST_P(test_status_running_to_off_fuel7);
    RUN_TEST_P(test_status_running_to_off_fuel8);

    RUN_TEST_P(test_status_running_to_off_ign1);
    RUN_TEST_P(test_status_running_to_off_ign2);
    RUN_TEST_P(test_status_running_to_off_ign3);
    RUN_TEST_P(test_status_running_to_off_ign4);
    RUN_TEST_P(test_status_running_to_off_ign5);
    RUN_TEST_P(test_status_running_to_off_ign6);
    RUN_TEST_P(test_status_running_to_off_ign7);
    RUN_TEST_P(test_status_running_to_off_ign8);
  }
}
