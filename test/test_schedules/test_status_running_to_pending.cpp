
#include <Arduino.h>
#include <unity.h>
#include "../test_utils.h"
#include "scheduler.h"

#define TIMEOUT 1000
#define DURATION 1000

static void test_status_running_to_pending(FuelSchedule &schedule)
{
    extern void resetFuelSchedulers(void); 
    resetFuelSchedulers();
    TEST_ASSERT_EQUAL(OFF, schedule._status);
    _setFuelScheduleDuration(schedule, TIMEOUT, DURATION);
    extern void startFuelSchedulers(void); 
    startFuelSchedulers();    
    while(schedule._status == PENDING) /*Wait*/ ;
    _setFuelScheduleDuration(schedule, 2*TIMEOUT, DURATION);
    while(isRunning(schedule)) /*Wait*/ ;
    TEST_ASSERT_EQUAL(PENDING, schedule._status);
}

static void test_status_running_to_pending_fuel1(void)
{
    test_status_running_to_pending(fuelSchedules[0]);
}

static void test_status_running_to_pending_fuel2(void)
{
    test_status_running_to_pending(fuelSchedules[1]);
}

static void test_status_running_to_pending_fuel3(void)
{
    test_status_running_to_pending(fuelSchedules[2]);
}

static void test_status_running_to_pending_fuel4(void)
{
    test_status_running_to_pending(fuelSchedules[3]);
}

static void test_status_running_to_pending_fuel5(void)
{
#if INJ_CHANNELS >= 5
    test_status_running_to_pending(fuelSchedules[4]);
#endif
}

static void test_status_running_to_pending_fuel6(void)
{
#if INJ_CHANNELS >= 6
    test_status_running_to_pending(fuelSchedules[5]);
#endif
}

static void test_status_running_to_pending_fuel7(void)
{
#if INJ_CHANNELS >= 7
    test_status_running_to_pending(fuelSchedules[6]);
#endif
}

static void test_status_running_to_pending_fuel8(void)
{
#if INJ_CHANNELS >= 8
    test_status_running_to_pending(fuelSchedules[7]);
#endif
}

static void test_status_running_to_pending(IgnitionSchedule &schedule)
{
    extern void resetIgnitionSchedulers(void); 
    resetIgnitionSchedulers();
    TEST_ASSERT_EQUAL(OFF, schedule._status);
    _setIgnitionScheduleDuration(schedule, TIMEOUT, DURATION);
    extern void startIgnitionSchedulers(void); 
    startIgnitionSchedulers();    
    while(isRunning(schedule)) /*Wait*/ ;
    TEST_ASSERT_EQUAL(PENDING, schedule._status);
}

static void test_status_running_to_pending_ign1(void)
{
    test_status_running_to_pending(ignitionSchedules[0]);
}

static void test_status_running_to_pending_ign2(void)
{
    test_status_running_to_pending(ignitionSchedules[1]);
}

static void test_status_running_to_pending_ign3(void)
{
    test_status_running_to_pending(ignitionSchedules[2]);
}

static void test_status_running_to_pending_ign4(void)
{
    test_status_running_to_pending(ignitionSchedules[3]);
}

static void test_status_running_to_pending_ign5(void)
{
#if IGN_CHANNELS >= 5
    test_status_running_to_pending(ignitionSchedules[4]);
#endif
}

static void test_status_running_to_pending_ign6(void)
{
#if INJ_CHANNELS >= 6
    test_status_running_to_pending(ignitionSchedules[5]);
#endif
}

static void test_status_running_to_pending_ign7(void)
{
#if INJ_CHANNELS >= 7
    test_status_running_to_pending(ignitionSchedules[6]);
#endif
}

static void test_status_running_to_pending_ign8(void)
{
#if INJ_CHANNELS >= 8
    test_status_running_to_pending(ignitionSchedules[7]);
#endif
}

void test_status_running_to_pending(void)
{
  SET_UNITY_FILENAME() {

    RUN_TEST_P(test_status_running_to_pending_fuel1);
    RUN_TEST_P(test_status_running_to_pending_fuel2);
    RUN_TEST_P(test_status_running_to_pending_fuel3);
    RUN_TEST_P(test_status_running_to_pending_fuel4);
    RUN_TEST_P(test_status_running_to_pending_fuel5);
    RUN_TEST_P(test_status_running_to_pending_fuel6);
    RUN_TEST_P(test_status_running_to_pending_fuel7);
    RUN_TEST_P(test_status_running_to_pending_fuel8);

    RUN_TEST_P(test_status_running_to_pending_ign1);
    RUN_TEST_P(test_status_running_to_pending_ign2);
    RUN_TEST_P(test_status_running_to_pending_ign3);
    RUN_TEST_P(test_status_running_to_pending_ign4);
    RUN_TEST_P(test_status_running_to_pending_ign5);
    RUN_TEST_P(test_status_running_to_pending_ign6);
    RUN_TEST_P(test_status_running_to_pending_ign7);
    RUN_TEST_P(test_status_running_to_pending_ign8);
  }
}
