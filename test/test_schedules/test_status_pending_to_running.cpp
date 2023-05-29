
#include <Arduino.h>
#include <unity.h>
#include "../test_utils.h"
#include "scheduler.h"

#define TIMEOUT 1000
#define DURATION 1000

static void test_status_pending_to_running(FuelSchedule &schedule)
{
    initialiseSchedulers();
    setFuelSchedule(schedule, TIMEOUT, DURATION);
    while(schedule._status == PENDING) /*Wait*/ ;
    TEST_ASSERT_EQUAL(RUNNING, schedule._status);
}

static void test_status_pending_to_running_inj1(void)
{
    test_status_pending_to_running(fuelSchedule1);
}

static void test_status_pending_to_running_inj2(void)
{
    test_status_pending_to_running(fuelSchedule2);
}

static void test_status_pending_to_running_inj3(void)
{
    test_status_pending_to_running(fuelSchedule3);
}

static void test_status_pending_to_running_inj4(void)
{
    test_status_pending_to_running(fuelSchedule4);
}

static void test_status_pending_to_running_inj5(void)
{
#if INJ_CHANNELS >= 5
    test_status_pending_to_running(fuelSchedule5);
#endif
}

static void test_status_pending_to_running_inj6(void)
{
#if INJ_CHANNELS >= 6
    test_status_pending_to_running(fuelSchedule6);
#endif
}

static void test_status_pending_to_running_inj7(void)
{
#if INJ_CHANNELS >= 7
    test_status_pending_to_running(fuelSchedule7);
#endif
}

static void test_status_pending_to_running_inj8(void)
{
#if INJ_CHANNELS >= 8
    test_status_pending_to_running(fuelSchedule8);
#endif
}

static void test_status_pending_to_running(IgnitionSchedule &schedule)
{
    initialiseSchedulers();
    _setIgnitionScheduleDuration(schedule, TIMEOUT, DURATION);
    while(schedule._status == PENDING) /*Wait*/ ;
    TEST_ASSERT_EQUAL(RUNNING, schedule._status);
}

static void test_status_pending_to_running_ign1(void)
{
    test_status_pending_to_running(ignitionSchedule1);
}

static void test_status_pending_to_running_ign2(void)
{
    test_status_pending_to_running(ignitionSchedule2);
}

static void test_status_pending_to_running_ign3(void)
{
    test_status_pending_to_running(ignitionSchedule3);
}

static void test_status_pending_to_running_ign4(void)
{
    test_status_pending_to_running(ignitionSchedule4);
}

static void test_status_pending_to_running_ign5(void)
{
#if IGN_CHANNELS >= 5
    test_status_pending_to_running(ignitionSchedule5);
#endif
}

static void test_status_pending_to_running_ign6(void)
{
#if INJ_CHANNELS >= 6
    test_status_pending_to_running(ignitionSchedule6);
#endif
}

static void test_status_pending_to_running_ign7(void)
{
#if INJ_CHANNELS >= 7
    test_status_pending_to_running(ignitionSchedule7);
#endif
}

static void test_status_pending_to_running_ign8(void)
{
#if INJ_CHANNELS >= 8
    test_status_pending_to_running(ignitionSchedule8);
#endif
}

void test_status_pending_to_running(void)
{
  SET_UNITY_FILENAME() {

    RUN_TEST(test_status_pending_to_running_inj1);
    RUN_TEST(test_status_pending_to_running_inj2);
    RUN_TEST(test_status_pending_to_running_inj3);
    RUN_TEST(test_status_pending_to_running_inj4);
    RUN_TEST(test_status_pending_to_running_inj5);
    RUN_TEST(test_status_pending_to_running_inj6);
    RUN_TEST(test_status_pending_to_running_inj7);
    RUN_TEST(test_status_pending_to_running_inj8);

    RUN_TEST(test_status_pending_to_running_ign1);
    RUN_TEST(test_status_pending_to_running_ign2);
    RUN_TEST(test_status_pending_to_running_ign3);
    RUN_TEST(test_status_pending_to_running_ign4);
    RUN_TEST(test_status_pending_to_running_ign5);
    RUN_TEST(test_status_pending_to_running_ign6);
    RUN_TEST(test_status_pending_to_running_ign7);
    RUN_TEST(test_status_pending_to_running_ign8);
  }
}
