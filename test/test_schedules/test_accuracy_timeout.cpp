
#include <Arduino.h>
#include <unity.h>
#include "../test_utils.h"
#include "scheduler.h"
#include "scheduledIO.h"
#include "utilities.h"

#define TIMEOUT 1000
#define DURATION 1000
#define DELTA 24

static uint32_t start_time, end_time;
static void startCallback(void) { end_time = micros(); }
static void endCallback(void) { /*Empty*/ }

void test_accuracy_timeout(FuelSchedule &schedule)
{
    resetFuelSchedulers();
    setCallbacks(schedule, startCallback, endCallback);
    start_time = micros();
    _setSchedule(schedule, TIMEOUT, DURATION);
    while(isPending(schedule)) /*Wait*/ ;
    TEST_ASSERT_UINT32_WITHIN(DELTA, TIMEOUT, end_time - start_time);
}

void test_accuracy_timeout_inj(void)
{
    for (uint8_t i = 0; i < _countof(fuelSchedules); i++)
    {
        test_accuracy_timeout(fuelSchedules[i]);
    }    
}

void test_accuracy_timeout_ign(IgnitionSchedule &schedule)
{
    resetIgnitionSchedulers();
    setCallbacks(schedule, startCallback, endCallback);
    start_time = micros();
    _setSchedule(schedule, TIMEOUT, DURATION);
    while(isPending(schedule)) /*Wait*/ ;
    TEST_ASSERT_UINT32_WITHIN(DELTA, TIMEOUT, end_time - start_time);
}

void test_accuracy_timeout_ign(void)
{
    for (uint8_t i = 0; i < _countof(ignitionSchedules); i++)
    {
        test_accuracy_timeout_ign(ignitionSchedules[i]);
    }
}

void test_accuracy_timeout(void)
{
    SET_UNITY_FILENAME() {
     RUN_TEST(test_accuracy_timeout_inj);
     RUN_TEST(test_accuracy_timeout_ign);
    }
}