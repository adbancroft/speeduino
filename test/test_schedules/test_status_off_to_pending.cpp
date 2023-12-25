
#include <Arduino.h>
#include <unity.h>
#include "../test_utils.h"
#include "scheduler.h"
#include "utilities.h"

#define TIMEOUT 1000
#define DURATION 1000

static void emptyCallback(void) {  }

void test_status_off_to_pending(FuelSchedule &schedule)
{
    initialiseFuelSchedulers(pinInjectors);
    _setSchedule(schedule, TIMEOUT, DURATION);
    TEST_ASSERT_EQUAL(PENDING, schedule._status);
}

void test_status_off_to_pending_inj(void)
{
    for (uint8_t i = 0; i < _countof(fuelSchedules); i++)
    {
        test_status_off_to_pending(fuelSchedules[i]);
    }    
}

extern void setCallbacks(Schedule &schedule, voidVoidCallback pStartCallback, voidVoidCallback pEndCallback);

void test_status_off_to_pending_ign(IgnitionSchedule &schedule)
{
    initialiseIgnitionSchedulers(pinCoils);
    setCallbacks(schedule, emptyCallback, emptyCallback);
    _setSchedule(schedule, TIMEOUT, DURATION);
    TEST_ASSERT_EQUAL(PENDING, schedule._status);
}

void test_status_off_to_pending_ign(void)
{
    for (uint8_t i = 0; i < _countof(ignitionSchedules); i++)
    {
        test_status_off_to_pending_ign(ignitionSchedules[i]);
    }
}

void test_status_off_to_pending(void)
{
  SET_UNITY_FILENAME() {
    RUN_TEST(test_status_off_to_pending_inj);
    RUN_TEST(test_status_off_to_pending_ign);
  }
}
