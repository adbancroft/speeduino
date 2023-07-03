
#include <Arduino.h>
#include <unity.h>
#include "../test_utils.h"
#include "scheduler.h"
#include "utilities.h"

#define TIMEOUT 1000
#define DURATION 1000
#define DELTA 20

static uint32_t start_time, end_time;
static void startCallback(void) { start_time = micros(); }
static void endCallback(void) { end_time = micros(); }

void test_accuracy_duration(FuelSchedule &schedule)
{
    extern void resetFuelSchedulers(void); 
    resetFuelSchedulers();
    setCallbacks(schedule, startCallback, endCallback);
    _setSchedule(schedule, TIMEOUT, DURATION);
    extern void startFuelSchedulers(void); 
    startFuelSchedulers();    
    while(schedule._status != OFF) /*Wait*/ ;
    TEST_ASSERT_UINT32_WITHIN(DELTA, DURATION, end_time - start_time);
}

void test_accuracy_duration_inj(void)
{
    for (uint8_t i = 0; i < _countof(fuelSchedules); i++)
    {
        test_accuracy_duration(fuelSchedules[i]);
    }
}

void test_accuracy_duration(IgnitionSchedule &schedule)
{
    extern void resetIgnitionSchedulers(void);
    resetIgnitionSchedulers();
    setCallbacks(schedule, startCallback, endCallback);
    _setSchedule(schedule, TIMEOUT, DURATION);
    extern void startIgnitionSchedulers(void);
    startIgnitionSchedulers();
    while(schedule._status != OFF) /*Wait*/ ;
    TEST_ASSERT_UINT32_WITHIN(DELTA, DURATION, end_time - start_time);    
}

void test_accuracy_duration_ign(void)
{
    for (uint8_t i = 0; i < _countof(ignitionSchedules); i++)
    {
        test_accuracy_duration(ignitionSchedules[i]);
    }
}


void test_accuracy_duration(void)
{
    SET_UNITY_FILENAME() {
        RUN_TEST(test_accuracy_duration_inj);
        RUN_TEST(test_accuracy_duration_ign);
    }
}
