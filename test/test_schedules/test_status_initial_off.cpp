#include <Arduino.h>
#include <unity.h>
#include "scheduler.h"
#include "../test_utils.h"

static void test_status_initial_off(FuelSchedule &schedule) 
{
    resetFuelSchedulers();
    TEST_ASSERT_EQUAL(OFF, schedule._status);
}

void test_status_initial_off_inj1(void)
{
    test_status_initial_off(fuelSchedules[0]);
}

void test_status_initial_off_inj2(void)
{
    test_status_initial_off(fuelSchedules[1]);
}

void test_status_initial_off_inj3(void)
{
    test_status_initial_off(fuelSchedules[2]);
}

void test_status_initial_off_inj4(void)
{
    test_status_initial_off(fuelSchedules[3]);
}

void test_status_initial_off_inj5(void)
{
#if ING_CHANNELS >= 5
    test_status_initial_off(fuelSchedules[4]);
#endif
}

void test_status_initial_off_inj6(void)
{
#if ING_CHANNELS >= 6
    test_status_initial_off(fuelSchedules[5]);
#endif
}

void test_status_initial_off_inj7(void)
{
#if ING_CHANNELS >= 7
    test_status_initial_off(fuelSchedules[6]);
#endif
}

void test_status_initial_off_inj8(void)
{
#if ING_CHANNELS >= 8
    test_status_initial_off(fuelSchedules[7]);
#endif
}

static void test_status_initial_off(IgnitionSchedule &schedule) 
{
    resetIgnitionSchedulers();
    TEST_ASSERT_EQUAL(OFF, schedule._status);
}

void test_status_initial_off_ign1(void)
{
    test_status_initial_off(ignitionSchedules[0]);
}

void test_status_initial_off_ign2(void)
{
    test_status_initial_off(ignitionSchedules[1]);
}

void test_status_initial_off_ign3(void)
{
    test_status_initial_off(ignitionSchedules[2]);
}

void test_status_initial_off_ign4(void)
{
    test_status_initial_off(ignitionSchedules[3]);
}

void test_status_initial_off_ign5(void)
{
#if IGN_CHANNELS >= 5
    test_status_initial_off(ignitionSchedules[4]);
#endif
}

void test_status_initial_off_ign6(void)
{
#if IGN_CHANNELS >= 6
    test_status_initial_off(ignitionSchedules[5]);
#endif
}

void test_status_initial_off_ign7(void)
{
#if IGN_CHANNELS >= 7
    test_status_initial_off(ignitionSchedules[6]);
#endif
}

void test_status_initial_off_ign8(void)
{
#if IGN_CHANNELS >= 8
    test_status_initial_off(ignitionSchedules[7]);
#endif
}

void test_status_initial_off(void)
{
  SET_UNITY_FILENAME() {

    RUN_TEST_P(test_status_initial_off_inj1);
    RUN_TEST_P(test_status_initial_off_inj2);
    RUN_TEST_P(test_status_initial_off_inj3);
    RUN_TEST_P(test_status_initial_off_inj4);
    RUN_TEST_P(test_status_initial_off_inj5);
    RUN_TEST_P(test_status_initial_off_inj6);
    RUN_TEST_P(test_status_initial_off_inj7);
    RUN_TEST_P(test_status_initial_off_inj8);

    RUN_TEST_P(test_status_initial_off_ign1);
    RUN_TEST_P(test_status_initial_off_ign2);
    RUN_TEST_P(test_status_initial_off_ign3);
    RUN_TEST_P(test_status_initial_off_ign4);
    RUN_TEST_P(test_status_initial_off_ign5);
    RUN_TEST_P(test_status_initial_off_ign6);
    RUN_TEST_P(test_status_initial_off_ign7);
    RUN_TEST_P(test_status_initial_off_ign8);
  }
}