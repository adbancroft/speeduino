
#include <Arduino.h>
#include <unity.h>
#include "../test_utils.h"
#include "scheduler.h"

#define TIMEOUT 1000
#define DURATION 1000

static void emptyCallback(void) {  }

void test_status_off_to_pending(FuelSchedule &schedule)
{
    initialiseSchedulers();
    _setSchedule(schedule, TIMEOUT, DURATION);
    TEST_ASSERT_EQUAL(PENDING, schedule._status);
}

void test_status_off_to_pending_inj1(void)
{
    test_status_off_to_pending(fuelSchedule1);
}

void test_status_off_to_pending_inj2(void)
{
    test_status_off_to_pending(fuelSchedule2);
}

void test_status_off_to_pending_inj3(void)
{
    test_status_off_to_pending(fuelSchedule3);
}

void test_status_off_to_pending_inj4(void)
{
    test_status_off_to_pending(fuelSchedule4);
}

void test_status_off_to_pending_inj5(void)
{
#if INJ_CHANNELS >= 5
    test_status_off_to_pending(fuelSchedule5);
#endif
}

void test_status_off_to_pending_inj6(void)
{
#if INJ_CHANNELS >= 6
    test_status_off_to_pending(fuelSchedule6);
#endif
}

void test_status_off_to_pending_inj7(void)
{
#if INJ_CHANNELS >= 7
    test_status_off_to_pending(fuelSchedule7);
#endif
}

void test_status_off_to_pending_inj8(void)
{
#if INJ_CHANNELS >= 8
    test_status_off_to_pending(fuelSchedule8);
#endif
}

static void test_status_off_to_pending(IgnitionSchedule &schedule)
{
    initialiseSchedulers();
    setCallbacks(schedule, emptyCallback, emptyCallback);
    _setSchedule(schedule, TIMEOUT, DURATION);
    TEST_ASSERT_EQUAL(PENDING, schedule._status);
}

void test_status_off_to_pending_ign1(void)
{
    test_status_off_to_pending(ignitionSchedule1);
}

void test_status_off_to_pending_ign2(void)
{
    test_status_off_to_pending(ignitionSchedule2);
}

void test_status_off_to_pending_ign3(void)
{
    test_status_off_to_pending(ignitionSchedule3);
}

void test_status_off_to_pending_ign4(void)
{
    test_status_off_to_pending(ignitionSchedule4);
}

void test_status_off_to_pending_ign5(void)
{
#if IGN_CHANNELS >= 5
    test_status_off_to_pending(ignitionSchedule5);
#endif
}

void test_status_off_to_pending_ign6(void)
{
#if IGN_CHANNELS >= 6
    test_status_off_to_pending(ignitionSchedule6);
#endif
}

void test_status_off_to_pending_ign7(void)
{
#if IGN_CHANNELS >= 7
    test_status_off_to_pending(ignitionSchedule7);
#endif
}

void test_status_off_to_pending_ign8(void)
{
#if IGN_CHANNELS >= 8
    test_status_off_to_pending(ignitionSchedule8);
#endif
}

void test_status_off_to_pending(void)
{
  SET_UNITY_FILENAME() {

    RUN_TEST(test_status_off_to_pending_inj1);
    RUN_TEST(test_status_off_to_pending_inj2);
    RUN_TEST(test_status_off_to_pending_inj3);
    RUN_TEST(test_status_off_to_pending_inj4);
    RUN_TEST(test_status_off_to_pending_inj5);
    RUN_TEST(test_status_off_to_pending_inj6);
    RUN_TEST(test_status_off_to_pending_inj7);
    RUN_TEST(test_status_off_to_pending_inj8);

    RUN_TEST(test_status_off_to_pending_ign1);
    RUN_TEST(test_status_off_to_pending_ign2);
    RUN_TEST(test_status_off_to_pending_ign3);
    RUN_TEST(test_status_off_to_pending_ign4);
    RUN_TEST(test_status_off_to_pending_ign5);
    RUN_TEST(test_status_off_to_pending_ign6);
    RUN_TEST(test_status_off_to_pending_ign7);
    RUN_TEST(test_status_off_to_pending_ign8);
  }
}
