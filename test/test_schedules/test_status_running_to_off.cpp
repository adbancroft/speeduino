
#include <Arduino.h>
#include <unity.h>
#include "../test_utils.h"
#include "scheduler.h"
#include "utilities.h"

#define TIMEOUT 1000
#define DURATION 1000

void test_status_running_to_off(FuelSchedule &schedule)
{
    resetFuelSchedulers();
    TEST_ASSERT_EQUAL(OFF, schedule._status);
    _setSchedule(schedule, TIMEOUT, DURATION);
    while( (isPending(schedule)) || (schedule._status == RUNNING) ) /*Wait*/ ;
    TEST_ASSERT_EQUAL(OFF, schedule._status);
}

void test_status_running_to_off_inj(void)
{
    for (uint8_t i = 0; i < _countof(fuelSchedules); i++)
    {
        test_status_running_to_off(fuelSchedules[i]);
    } 
}

void test_status_running_to_off(IgnitionSchedule &schedule)
{
    resetIgnitionSchedulers();
    TEST_ASSERT_EQUAL(OFF, schedule._status);
    _setSchedule(schedule, TIMEOUT, DURATION);
    while( (isPending(schedule)) || (schedule._status == RUNNING) ) /*Wait*/ ;
    TEST_ASSERT_EQUAL(OFF, schedule._status);
}


void test_status_running_to_off_ign(void)
{
    for (uint8_t i = 0; i < _countof(ignitionSchedules); i++)
    {
        test_status_running_to_off(ignitionSchedules[i]);
    }
}

void test_status_running_to_off(void)
{
  SET_UNITY_FILENAME() {
    RUN_TEST(test_status_running_to_off_inj);
    RUN_TEST(test_status_running_to_off_ign);
  }
}
