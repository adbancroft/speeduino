
#include <Arduino.h>
#include <unity.h>
#include "../test_utils.h"
#include "scheduler.h"

#define TIMEOUT 1000
#define DURATION 1000

void test_status_pending_to_running_inj(FuelSchedule &schedule)
{
    resetFuelSchedulers();
    TEST_ASSERT_EQUAL(OFF, schedule._status);
    _setSchedule(schedule, TIMEOUT, DURATION);
    while(isPending(schedule)) /*Wait*/ ;
    TEST_ASSERT_EQUAL(RUNNING, schedule._status);
}

void test_status_pending_to_running_inj1(void)
{
    test_status_pending_to_running_inj(fuelSchedules[0]);
}

void test_status_pending_to_running_inj2(void)
{
    test_status_pending_to_running_inj(fuelSchedules[1]);
}

void test_status_pending_to_running_inj3(void)
{
    test_status_pending_to_running_inj(fuelSchedules[2]);
}

void test_status_pending_to_running_inj4(void)
{
    test_status_pending_to_running_inj(fuelSchedules[3]);
}

void test_status_pending_to_running_inj5(void)
{
#if INJ_CHANNELS >= 5
    test_status_pending_to_running_inj(fuelSchedules[4]);
#endif
}

void test_status_pending_to_running_inj6(void)
{
#if INJ_CHANNELS >= 6
    test_status_pending_to_running_inj(fuelSchedules[5]);
#endif
}

void test_status_pending_to_running_inj7(void)
{
#if INJ_CHANNELS >= 7
    test_status_pending_to_running_inj(fuelSchedules[6]);
#endif
}

void test_status_pending_to_running_inj8(void)
{
#if INJ_CHANNELS >= 8
    test_status_pending_to_running_inj(fuelSchedules[7]);
#endif
}

void test_status_pending_to_running_ign(IgnitionSchedule &schedule)
{
    resetIgnitionSchedulers();
    TEST_ASSERT_EQUAL(OFF, schedule._status);
    _setSchedule(schedule, TIMEOUT, DURATION);
    while(isPending(schedule)) /*Wait*/ ;
    TEST_ASSERT_EQUAL(RUNNING, schedule._status);
}

void test_status_pending_to_running_ign1(void)
{
    test_status_pending_to_running_ign(ignitionSchedules[0]);
}

void test_status_pending_to_running_ign2(void)
{
    test_status_pending_to_running_ign(ignitionSchedules[1]);
}

void test_status_pending_to_running_ign3(void)
{
    test_status_pending_to_running_ign(ignitionSchedules[2]);
}

void test_status_pending_to_running_ign4(void)
{
    test_status_pending_to_running_ign(ignitionSchedules[3]);
}

void test_status_pending_to_running_ign5(void)
{
#if IGN_CHANNELS >= 5
    test_status_pending_to_running_ign(ignitionSchedules[4]);
#endif
}

void test_status_pending_to_running_ign6(void)
{
#if INJ_CHANNELS >= 6
    test_status_pending_to_running_ign(ignitionSchedules[5]);
#endif
}

void test_status_pending_to_running_ign7(void)
{
#if INJ_CHANNELS >= 7
    test_status_pending_to_running_ign(ignitionSchedules[6]);
#endif
}

void test_status_pending_to_running_ign8(void)
{
#if INJ_CHANNELS >= 8
    test_status_pending_to_running_ign(ignitionSchedules[7]);
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
