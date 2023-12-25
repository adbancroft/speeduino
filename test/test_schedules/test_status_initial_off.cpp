#include <Arduino.h>
#include <unity.h>
#include "scheduler.h"
#include "../test_utils.h"
#include "utilities.h"

void test_status_initial_off(FuelSchedule &schedule)
{
    initialiseFuelSchedulers(pinInjectors);
    TEST_ASSERT_EQUAL(OFF, schedule._status);
}

void test_status_initial_off_inj(void)
{
    for (uint8_t i = 0; i < _countof(fuelSchedules); i++)
    {
        test_status_initial_off(fuelSchedules[i]);
    }
}

void test_status_initial_off(IgnitionSchedule &schedule)
{
    initialiseFuelSchedulers(pinInjectors);
    TEST_ASSERT_EQUAL(OFF, schedule._status);
}

void test_status_initial_off_ign(void)
{
    for (uint8_t i = 0; i < _countof(ignitionSchedules); i++)
    {
        test_status_initial_off(ignitionSchedules[i]);
    }
}

void test_status_initial_off(void)
{
  SET_UNITY_FILENAME() {
    RUN_TEST(test_status_initial_off_inj);
    RUN_TEST(test_status_initial_off_ign);
  }
}