
#include <Arduino.h>
#include <unity.h>
#include "../test_utils.h"
#include "scheduler.h"

#define TIMEOUT 1000
#define DURATION 1000

static void test_status_running_to_off(FuelSchedule &schedule)
{
    resetFuelSchedulers();
    setFuelSchedule(schedule, TIMEOUT, DURATION);
    while( (schedule._status == PENDING) || (schedule._status == RUNNING) ) /*Wait*/ ;
    TEST_ASSERT_EQUAL(OFF, schedule._status);
}

static void test_status_running_to_off_inj1(void)
{
    test_status_running_to_off(fuelSchedule1);
}

static void test_status_running_to_off_inj2(void)
{
    test_status_running_to_off(fuelSchedule2);
}

static void test_status_running_to_off_inj3(void)
{
    test_status_running_to_off(fuelSchedule3);
}

static void test_status_running_to_off_inj4(void)
{
    test_status_running_to_off(fuelSchedule4);
}

static void test_status_running_to_off_inj5(void)
{
#if INJ_CHANNELS >= 5
    test_status_running_to_off(fuelSchedule5);
#endif
}

static void test_status_running_to_off_inj6(void)
{
#if INJ_CHANNELS >= 6
    test_status_running_to_off(fuelSchedule6);
#endif
}

static void test_status_running_to_off_inj7(void)
{
#if INJ_CHANNELS >= 7
    test_status_running_to_off(fuelSchedule7);
#endif
}

static void test_status_running_to_off_inj8(void)
{
#if INJ_CHANNELS >= 8
    test_status_running_to_off(fuelSchedule8);
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
    test_status_running_to_off(ignitionSchedule1);
}

static void test_status_running_to_off_ign2(void)
{
    test_status_running_to_off(ignitionSchedule2);
}

static void test_status_running_to_off_ign3(void)
{
    test_status_running_to_off(ignitionSchedule3);
}

static void test_status_running_to_off_ign4(void)
{
    test_status_running_to_off(ignitionSchedule4);
}

static void test_status_running_to_off_ign5(void)
{
#if IGN_CHANNELS >= 5
    test_status_running_to_off(ignitionSchedule5);
#endif
}

static void test_status_running_to_off_ign6(void)
{
#if IGN_CHANNELS >= 6
    test_status_running_to_off(ignitionSchedule6);
#endif
}

static void test_status_running_to_off_ign7(void)
{
#if IGN_CHANNELS >= 7
    test_status_running_to_off(ignitionSchedule7);
#endif
}

static void test_status_running_to_off_ign8(void)
{
#if IGN_CHANNELS >= 8
    test_status_running_to_off(ignitionSchedule8);
#endif
}

void test_status_running_to_off(void)
{
  SET_UNITY_FILENAME() {

    RUN_TEST(test_status_running_to_off_inj1);
    RUN_TEST(test_status_running_to_off_inj2);
    RUN_TEST(test_status_running_to_off_inj3);
    RUN_TEST(test_status_running_to_off_inj4);
    RUN_TEST(test_status_running_to_off_inj5);
    RUN_TEST(test_status_running_to_off_inj6);
    RUN_TEST(test_status_running_to_off_inj7);
    RUN_TEST(test_status_running_to_off_inj8);

    RUN_TEST(test_status_running_to_off_ign1);
    RUN_TEST(test_status_running_to_off_ign2);
    RUN_TEST(test_status_running_to_off_ign3);
    RUN_TEST(test_status_running_to_off_ign4);
    RUN_TEST(test_status_running_to_off_ign5);
    RUN_TEST(test_status_running_to_off_ign6);
    RUN_TEST(test_status_running_to_off_ign7);
    RUN_TEST(test_status_running_to_off_ign8);
  }
}
