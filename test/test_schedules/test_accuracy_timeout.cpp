
#include <Arduino.h>
#include <unity.h>
#include "../test_utils.h"
#include "scheduler.h"
#include "scheduledIO.h"

#define TIMEOUT 1000
#define DURATION 1000
#define DELTA 24

static uint32_t start_time, end_time;
static void startCallback(void) { end_time = micros(); }
static void endCallback(void) { /*Empty*/ }

void test_accuracy_timeout_inj(FuelSchedule &schedule)
{
    resetFuelSchedulers();
    setCallbacks(schedule, startCallback, endCallback);
    start_time = micros();
    _setSchedule(schedule, TIMEOUT, DURATION);
    while(isPending(schedule)) /*Wait*/ ;
    TEST_ASSERT_UINT32_WITHIN(DELTA, TIMEOUT, end_time - start_time);
}

void test_accuracy_timeout_inj1(void)
{
    test_accuracy_timeout_inj(fuelSchedules[0]);
}

void test_accuracy_timeout_inj2(void)
{
    test_accuracy_timeout_inj(fuelSchedules[1]);
}

void test_accuracy_timeout_inj3(void)
{
    test_accuracy_timeout_inj(fuelSchedules[2]);
}

void test_accuracy_timeout_inj4(void)
{
    test_accuracy_timeout_inj(fuelSchedules[3]);
}

#if INJ_CHANNELS >= 5
void test_accuracy_timeout_inj5(void)
{
    test_accuracy_timeout_inj(fuelSchedules[4]);
}
#endif

#if INJ_CHANNELS >= 6
void test_accuracy_timeout_inj6(void)
{
    test_accuracy_timeout_inj(fuelSchedules[5]);
}
#endif

#if INJ_CHANNELS >= 7
void test_accuracy_timeout_inj7(void)
{
    test_accuracy_timeout_inj(fuelSchedules[6]);
}
#endif

#if INJ_CHANNELS >= 8
void test_accuracy_timeout_inj8(void)
{
    test_accuracy_timeout_inj(fuelSchedules[7]);
}
#endif

void test_accuracy_timeout_ign(IgnitionSchedule &schedule)
{
    resetIgnitionSchedulers();
    setCallbacks(schedule, startCallback, endCallback);
    start_time = micros();
    _setSchedule(schedule, TIMEOUT, DURATION);
    while(isPending(schedule)) /*Wait*/ ;
    TEST_ASSERT_UINT32_WITHIN(DELTA, TIMEOUT, end_time - start_time);
}

void test_accuracy_timeout_ign1(void)
{
    test_accuracy_timeout_ign(ignitionSchedules[0]);
}

void test_accuracy_timeout_ign2(void)
{
    test_accuracy_timeout_ign(ignitionSchedules[1]);
}

void test_accuracy_timeout_ign3(void)
{
    test_accuracy_timeout_ign(ignitionSchedules[2]);
}

void test_accuracy_timeout_ign4(void)
{
    test_accuracy_timeout_ign(ignitionSchedules[3]);
}

#if IGN_CHANNELS >= 5
void test_accuracy_timeout_ign5(void)
{
    test_accuracy_timeout_ign(ignitionSchedules[4]);
}
#endif

#if IGN_CHANNELS >= 6
void test_accuracy_timeout_ign6(void)
{
    test_accuracy_timeout_ign(ignitionSchedules[5]);
}
#endif

#if IGN_CHANNELS >= 7
void test_accuracy_timeout_ign7(void)
{
    test_accuracy_timeout_ign(ignitionSchedules[6]);
}
#endif

#if IGN_CHANNELS >= 8
void test_accuracy_timeout_ign8(void)
{
    test_accuracy_timeout_ign(ignitionSchedules[7]);
}
#endif

void test_accuracy_timeout(void)
{
  SET_UNITY_FILENAME() {

    RUN_TEST(test_accuracy_timeout_inj1);
    RUN_TEST(test_accuracy_timeout_inj2);
    RUN_TEST(test_accuracy_timeout_inj3);
    RUN_TEST(test_accuracy_timeout_inj4);
#if INJ_CHANNELS >= 5
    RUN_TEST(test_accuracy_timeout_inj5);
#endif
#if INJ_CHANNELS >= 6
    RUN_TEST(test_accuracy_timeout_inj6);
#endif
#if INJ_CHANNELS >= 7
    RUN_TEST(test_accuracy_timeout_inj7);
#endif
#if INJ_CHANNELS >= 8
    RUN_TEST(test_accuracy_timeout_inj8);
#endif

    RUN_TEST(test_accuracy_timeout_ign1);
    RUN_TEST(test_accuracy_timeout_ign2);
    RUN_TEST(test_accuracy_timeout_ign3);
    RUN_TEST(test_accuracy_timeout_ign4);
#if IGN_CHANNELS >= 5
    RUN_TEST(test_accuracy_timeout_ign5);
#endif
#if IGN_CHANNELS >= 6
    RUN_TEST(test_accuracy_timeout_ign6);
#endif
#if IGN_CHANNELS >= 7
    RUN_TEST(test_accuracy_timeout_ign7);
#endif
#if IGN_CHANNELS >= 8
    RUN_TEST(test_accuracy_timeout_ign8);
#endif
  }
}
