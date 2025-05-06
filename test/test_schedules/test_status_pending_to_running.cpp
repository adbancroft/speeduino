
#include <Arduino.h>
#include <unity.h>
#include "../test_utils.h"
#include "scheduler.h"

#define TIMEOUT 1000
#define DURATION 1000

static void test_status_pending_to_running(FuelSchedule &schedule)
{
    extern void resetFuelSchedulers(void); 
    resetFuelSchedulers();
    TEST_ASSERT_EQUAL(OFF, schedule._status);
    _setFuelScheduleDuration(schedule, TIMEOUT, DURATION);
    extern void startFuelSchedulers(void); 
    startFuelSchedulers();    
    while(schedule._status == PENDING) /*Wait*/ ;
    TEST_ASSERT_EQUAL(RUNNING, schedule._status);
}

static void test_status_pending_to_running_fuel1(void)
{
    test_status_pending_to_running(fuelSchedules[0]);
}

static void test_status_pending_to_running_fuel2(void)
{
    test_status_pending_to_running(fuelSchedules[1]);
}

static void test_status_pending_to_running_fuel3(void)
{
    test_status_pending_to_running(fuelSchedules[2]);
}

static void test_status_pending_to_running_fuel4(void)
{
    test_status_pending_to_running(fuelSchedules[3]);
}

static void test_status_pending_to_running_fuel5(void)
{
#if INJ_CHANNELS >= 5
    test_status_pending_to_running(fuelSchedules[4]);
#endif
}

static void test_status_pending_to_running_fuel6(void)
{
#if INJ_CHANNELS >= 6
    test_status_pending_to_running(fuelSchedules[5]);
#endif
}

static void test_status_pending_to_running_fuel7(void)
{
#if INJ_CHANNELS >= 7
    test_status_pending_to_running(fuelSchedules[6]);
#endif
}

static void test_status_pending_to_running_fuel8(void)
{
#if INJ_CHANNELS >= 8
    test_status_pending_to_running(fuelSchedules[7]);
#endif
}

static void test_status_pending_to_running(IgnitionSchedule &schedule)
{
    extern void resetIgnitionSchedulers(void);
    resetIgnitionSchedulers();
    TEST_ASSERT_EQUAL(OFF, schedule._status);
    _setIgnitionScheduleDuration(schedule, TIMEOUT, DURATION);
    extern void startIgnitionSchedulers(void);
    startIgnitionSchedulers();
    while(schedule._status == PENDING) /*Wait*/ ;
    TEST_ASSERT_EQUAL(RUNNING, schedule._status);
}

static void test_status_pending_to_running_ign1(void)
{
    test_status_pending_to_running(ignitionSchedules[0]);
}

static void test_status_pending_to_running_ign2(void)
{
    test_status_pending_to_running(ignitionSchedules[1]);
}

static void test_status_pending_to_running_ign3(void)
{
    test_status_pending_to_running(ignitionSchedules[2]);
}

static void test_status_pending_to_running_ign4(void)
{
    test_status_pending_to_running(ignitionSchedules[3]);
}

static void test_status_pending_to_running_ign5(void)
{
#if IGN_CHANNELS >= 5
    test_status_pending_to_running(ignitionSchedules[4]);
#endif
}

static void test_status_pending_to_running_ign6(void)
{
#if INJ_CHANNELS >= 6
    test_status_pending_to_running(ignitionSchedules[5]);
#endif
}

static void test_status_pending_to_running_ign7(void)
{
#if INJ_CHANNELS >= 7
    test_status_pending_to_running(ignitionSchedules[6]);
#endif
}

static void test_status_pending_to_running_ign8(void)
{
#if INJ_CHANNELS >= 8
    test_status_pending_to_running(ignitionSchedules[7]);
#endif
}

void test_status_pending_to_running(void)
{
  SET_UNITY_FILENAME() {

    RUN_TEST_P(test_status_pending_to_running_fuel1);
    RUN_TEST_P(test_status_pending_to_running_fuel2);
    RUN_TEST_P(test_status_pending_to_running_fuel3);
    RUN_TEST_P(test_status_pending_to_running_fuel4);
    RUN_TEST_P(test_status_pending_to_running_fuel5);
    RUN_TEST_P(test_status_pending_to_running_fuel6);
    RUN_TEST_P(test_status_pending_to_running_fuel7);
    RUN_TEST_P(test_status_pending_to_running_fuel8);

    RUN_TEST_P(test_status_pending_to_running_ign1);
    RUN_TEST_P(test_status_pending_to_running_ign2);
    RUN_TEST_P(test_status_pending_to_running_ign3);
    RUN_TEST_P(test_status_pending_to_running_ign4);
    RUN_TEST_P(test_status_pending_to_running_ign5);
    RUN_TEST_P(test_status_pending_to_running_ign6);
    RUN_TEST_P(test_status_pending_to_running_ign7);
    RUN_TEST_P(test_status_pending_to_running_ign8);
  }
}
