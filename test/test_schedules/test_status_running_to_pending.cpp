
#include <Arduino.h>
#include <unity.h>
#include "../test_utils.h"
#include "scheduler.h"
#include "utilities.h"

#define TIMEOUT 1000
#define DURATION 1000

void test_status_running_to_pending(FuelSchedule &schedule)
{
    extern void resetFuelSchedulers(void); 
    resetFuelSchedulers();
    TEST_ASSERT_EQUAL(OFF, schedule._status);
    _setSchedule(schedule, TIMEOUT, DURATION);
    extern void startFuelSchedulers(void); 
    startFuelSchedulers();    
    while(isPending(schedule)) /*Wait*/ ;
    _setSchedule(schedule, 2*TIMEOUT, DURATION);
    while(isRunning(schedule)) /*Wait*/ ;
    TEST_ASSERT_EQUAL(PENDING, schedule._status);
}

void test_status_running_to_pending_inj(void)
{
    for (uint8_t i = 0; i < _countof(fuelSchedules); i++)
    {
        test_status_running_to_pending(fuelSchedules[i]);
    } 
}

void test_status_running_to_pending(IgnitionSchedule &schedule)
{
    extern void resetIgnitionSchedulers(void);
    resetIgnitionSchedulers();
    TEST_ASSERT_EQUAL(OFF, schedule._status);
    _setSchedule(schedule, TIMEOUT, DURATION);
    while(isPending(schedule)) /*Wait*/ ;
    _setSchedule(schedule, 2*TIMEOUT, DURATION);
    extern void startIgnitionSchedulers(void);
    startIgnitionSchedulers();
    while(isRunning(schedule)) /*Wait*/ ;
    TEST_ASSERT_EQUAL(PENDING, schedule._status);
}


void test_status_running_to_pending_ign(void)
{
    for (uint8_t i = 0; i < _countof(ignitionSchedules); i++)
    {
        test_status_running_to_pending(ignitionSchedules[i]);
    }
}

void test_status_running_to_pending(void)
{
  SET_UNITY_FILENAME() {
    RUN_TEST(test_status_running_to_pending_inj);
    RUN_TEST(test_status_running_to_pending_ign);
  }
}
