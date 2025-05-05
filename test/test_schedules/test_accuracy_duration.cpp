
#include <Arduino.h>
#include <unity.h>
#include "../test_utils.h"
#include "scheduler.h"

#define TIMEOUT 1000
#define DURATION 1000
#define DELTA 20

static uint32_t start_time, end_time;
static void startCallback(void) { start_time = micros(); }
static void endCallback(void) { end_time = micros(); }

extern void setCallbacks(Schedule &schedule, voidVoidCallback pStartCallback, voidVoidCallback pEndCallback);

void test_accuracy_duration_inj(FuelSchedule &schedule)
{
    resetFuelSchedulers();
    setCallbacks(schedule, startCallback, endCallback);
    _setFuelScheduleDuration(schedule, TIMEOUT, DURATION);
    while(schedule._status != OFF) /*Wait*/ ;
    TEST_ASSERT_UINT32_WITHIN(DELTA, DURATION, end_time - start_time);
}

void test_accuracy_duration_inj1(void)
{
    test_accuracy_duration_inj(fuelSchedules[0]);
}

void test_accuracy_duration_inj2(void)
{
    test_accuracy_duration_inj(fuelSchedules[1]);
}

void test_accuracy_duration_inj3(void)
{
    test_accuracy_duration_inj(fuelSchedules[2]);
}

void test_accuracy_duration_inj4(void)
{
    test_accuracy_duration_inj(fuelSchedules[3]);
}

#if INJ_CHANNELS >= 5
void test_accuracy_duration_inj5(void)
{
    test_accuracy_duration_inj(fuelSchedules[4]);
}
#endif

#if INJ_CHANNELS >= 6
void test_accuracy_duration_inj6(void)
{
    test_accuracy_duration_inj(fuelSchedules[5]);
}
#endif

#if INJ_CHANNELS >= 7
void test_accuracy_duration_inj7(void)
{
    test_accuracy_duration_inj(fuelSchedules[6]);
}
#endif

#if INJ_CHANNELS >= 8
void test_accuracy_duration_inj8(void)
{
    test_accuracy_duration_inj(fuelSchedules[7]);
}
#endif

void test_accuracy_duration_ign(IgnitionSchedule &schedule)
{
    resetIgnitionSchedulers();
    setCallbacks(schedule, startCallback, endCallback);
    _setIgnitionScheduleDuration(schedule, TIMEOUT, DURATION);
    while(schedule._status != OFF) /*Wait*/ ;
    TEST_ASSERT_UINT32_WITHIN(DELTA, DURATION, end_time - start_time);    

}
void test_accuracy_duration_ign1(void)
{
    test_accuracy_duration_ign(ignitionSchedules[0]);
}

void test_accuracy_duration_ign2(void)
{
    test_accuracy_duration_ign(ignitionSchedules[1]);
}

void test_accuracy_duration_ign3(void)
{
    test_accuracy_duration_ign(ignitionSchedules[2]);
}

void test_accuracy_duration_ign4(void)
{
    test_accuracy_duration_ign(ignitionSchedules[3]);
}

void test_accuracy_duration_ign5(void)
{
#if IGN_CHANNELS >= 5
    test_accuracy_duration_ign(ignitionSchedules[4]);
#endif
}

#if INJ_CHANNELS >= 6
void test_accuracy_duration_ign6(void)
{
    test_accuracy_duration_ign(ignitionSchedules[5]);
}
#endif

#if INJ_CHANNELS >= 7
void test_accuracy_duration_ign7(void)
{
    test_accuracy_duration_ign(ignitionSchedules[6]);
}
#endif

#if INJ_CHANNELS >= 8
void test_accuracy_duration_ign8(void)
{
    test_accuracy_duration_ign(ignitionSchedules[7]);
}
#endif

void test_accuracy_duration(void)
{
  SET_UNITY_FILENAME() {

    RUN_TEST(test_accuracy_duration_inj1);
    RUN_TEST(test_accuracy_duration_inj2);
    RUN_TEST(test_accuracy_duration_inj3);
    RUN_TEST(test_accuracy_duration_inj4);
#if INJ_CHANNELS >= 5
    RUN_TEST(test_accuracy_duration_inj5);
#endif
#if INJ_CHANNELS >= 6
    RUN_TEST(test_accuracy_duration_inj6);
#endif
#if INJ_CHANNELS >= 7
    RUN_TEST(test_accuracy_duration_inj7);
#endif
#if INJ_CHANNELS >= 8
    RUN_TEST(test_accuracy_duration_inj8);
#endif

    RUN_TEST(test_accuracy_duration_ign1);
    RUN_TEST(test_accuracy_duration_ign2);
    RUN_TEST(test_accuracy_duration_ign3);
    RUN_TEST(test_accuracy_duration_ign4);
#if INJ_CHANNELS >= 5
    RUN_TEST(test_accuracy_duration_ign5);
#endif
#if INJ_CHANNELS >= 6
    RUN_TEST(test_accuracy_duration_ign6);
#endif
#if INJ_CHANNELS >= 7
    RUN_TEST(test_accuracy_duration_ign7);
#endif
#if INJ_CHANNELS >= 8
    RUN_TEST(test_accuracy_duration_ign8);
#endif
  }
}
