
#include <Arduino.h>
#include <unity.h>
#include "../test_utils.h"
#include "scheduler.h"
#include "utilities.h"

#define TIMEOUT 1000
#define DURATION 1000

extern void resetFuelSchedulers(void);

void test_status_pending_to_running(FuelSchedule &schedule)
{
    extern void resetFuelSchedulers(void); 
    resetFuelSchedulers();
    TEST_ASSERT_EQUAL(OFF, schedule._status);
    _setSchedule(schedule, TIMEOUT, DURATION);
    extern void startFuelSchedulers(void); 
    startFuelSchedulers();    
    while(isPending(schedule)) /*Wait*/ ;
    TEST_ASSERT_EQUAL(RUNNING, schedule._status);
}

void test_status_pending_to_running_inj(void)
{
    for (uint8_t i = 0; i < _countof(fuelSchedules); i++)
    {
        test_status_pending_to_running(fuelSchedules[i]);
    } 
}

void test_status_pending_to_running(IgnitionSchedule &schedule)
{
    extern void resetIgnitionSchedulers(void);
    resetIgnitionSchedulers();
    TEST_ASSERT_EQUAL(OFF, schedule._status);
    _setSchedule(schedule, TIMEOUT, DURATION);
    extern void startIgnitionSchedulers(void);
    startIgnitionSchedulers();
    while(isPending(schedule)) /*Wait*/ ;
    TEST_ASSERT_EQUAL(RUNNING, schedule._status);
}

void test_status_pending_to_running_ign(void)
{
    for (uint8_t i = 0; i < _countof(ignitionSchedules); i++)
    {
        test_status_pending_to_running(ignitionSchedules[i]);
    }
}


void test_status_pending_to_running(void)
{
  SET_UNITY_FILENAME() {
    RUN_TEST(test_status_pending_to_running_inj);
    RUN_TEST(test_status_pending_to_running_ign);
  }
}
