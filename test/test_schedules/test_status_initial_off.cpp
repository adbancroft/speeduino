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
    test_status_initial_off(fuelSchedule1);
}

void test_status_initial_off_inj2(void)
{
    test_status_initial_off(fuelSchedule2);
}

void test_status_initial_off_inj3(void)
{
    test_status_initial_off(fuelSchedule3);
}

void test_status_initial_off_inj4(void)
{
    test_status_initial_off(fuelSchedule4);
}

void test_status_initial_off_inj5(void)
{
#if ING_CHANNELS >= 5
    test_status_initial_off(fuelSchedule5);
#endif
}

void test_status_initial_off_inj6(void)
{
#if ING_CHANNELS >= 6
    test_status_initial_off(fuelSchedule6);
#endif
}

void test_status_initial_off_inj7(void)
{
#if ING_CHANNELS >= 7
    test_status_initial_off(fuelSchedule7);
#endif
}

void test_status_initial_off_inj8(void)
{
#if ING_CHANNELS >= 8
    test_status_initial_off(fuelSchedule8);
#endif
}

static void test_status_initial_off(IgnitionSchedule &schedule) 
{
    resetIgnitionSchedulers();
    TEST_ASSERT_EQUAL(OFF, schedule._status);
}

void test_status_initial_off_ign1(void)
{
    test_status_initial_off(ignitionSchedule1);
}

void test_status_initial_off_ign2(void)
{
    test_status_initial_off(ignitionSchedule2);
}

void test_status_initial_off_ign3(void)
{
    test_status_initial_off(ignitionSchedule3);
}

void test_status_initial_off_ign4(void)
{
    test_status_initial_off(ignitionSchedule4);
}

void test_status_initial_off_ign5(void)
{
#if IGN_CHANNELS >= 5
    test_status_initial_off(ignitionSchedule5);
#endif
}

void test_status_initial_off_ign6(void)
{
#if IGN_CHANNELS >= 6
    test_status_initial_off(ignitionSchedule6);
#endif
}

void test_status_initial_off_ign7(void)
{
#if IGN_CHANNELS >= 7
    test_status_initial_off(ignitionSchedule7);
#endif
}

void test_status_initial_off_ign8(void)
{
#if IGN_CHANNELS >= 8
    test_status_initial_off(ignitionSchedule8);
#endif
}

void test_status_initial_off(void)
{
  SET_UNITY_FILENAME() {

    RUN_TEST(test_status_initial_off_inj1);
    RUN_TEST(test_status_initial_off_inj2);
    RUN_TEST(test_status_initial_off_inj3);
    RUN_TEST(test_status_initial_off_inj4);
    RUN_TEST(test_status_initial_off_inj5);
    RUN_TEST(test_status_initial_off_inj6);
    RUN_TEST(test_status_initial_off_inj7);
    RUN_TEST(test_status_initial_off_inj8);

    RUN_TEST(test_status_initial_off_ign1);
    RUN_TEST(test_status_initial_off_ign2);
    RUN_TEST(test_status_initial_off_ign3);
    RUN_TEST(test_status_initial_off_ign4);
    RUN_TEST(test_status_initial_off_ign5);
    RUN_TEST(test_status_initial_off_ign6);
    RUN_TEST(test_status_initial_off_ign7);
    RUN_TEST(test_status_initial_off_ign8);
  }
}