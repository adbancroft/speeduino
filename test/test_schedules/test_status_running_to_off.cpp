
#include <Arduino.h>
#include <unity.h>
#include "../test_utils.h"
#include "scheduler.h"

#define TIMEOUT 1000
#define DURATION 1000

static void emptyCallback(void) {  }

void test_status_running_to_off_inj1(void)
{
    initialiseSchedulers();
    setFuelSchedule(fuelSchedule1, TIMEOUT, DURATION);
    while( (fuelSchedule1._status == PENDING) || (fuelSchedule1._status == RUNNING) ) /*Wait*/ ;
    TEST_ASSERT_EQUAL(OFF, fuelSchedule1._status);
}

void test_status_running_to_off_inj2(void)
{
    initialiseSchedulers();
    setFuelSchedule(fuelSchedule2, TIMEOUT, DURATION);
    while( (fuelSchedule2._status == PENDING) || (fuelSchedule2._status == RUNNING) ) /*Wait*/ ;
    TEST_ASSERT_EQUAL(OFF, fuelSchedule2._status);
}

void test_status_running_to_off_inj3(void)
{
    initialiseSchedulers();
    setFuelSchedule(fuelSchedule3, TIMEOUT, DURATION);
    while( (fuelSchedule3._status == PENDING) || (fuelSchedule3._status == RUNNING) ) /*Wait*/ ;
    TEST_ASSERT_EQUAL(OFF, fuelSchedule3._status);
}

void test_status_running_to_off_inj4(void)
{
    initialiseSchedulers();
    setFuelSchedule(fuelSchedule4, TIMEOUT, DURATION);
    while( (fuelSchedule4._status == PENDING) || (fuelSchedule4._status == RUNNING) ) /*Wait*/ ;
    TEST_ASSERT_EQUAL(OFF, fuelSchedule4._status);
}

void test_status_running_to_off_inj5(void)
{
#if INJ_CHANNELS >= 5
    initialiseSchedulers();
    setFuelSchedule(fuelSchedule5, TIMEOUT, DURATION);
    while( (fuelSchedule5._status == PENDING) || (fuelSchedule5._status == RUNNING) ) /*Wait*/ ;
    TEST_ASSERT_EQUAL(OFF, fuelSchedule5._status);
#endif
}

void test_status_running_to_off_inj6(void)
{
#if INJ_CHANNELS >= 6
    initialiseSchedulers();
    setFuelSchedule(fuelSchedule6, TIMEOUT, DURATION);
    while( (fuelSchedule6._status == PENDING) || (fuelSchedule6._status == RUNNING) ) /*Wait*/ ;
    TEST_ASSERT_EQUAL(OFF, fuelSchedule6._status);
#endif
}

void test_status_running_to_off_inj7(void)
{
#if INJ_CHANNELS >= 7
    initialiseSchedulers();
    setFuelSchedule(fuelSchedule7, TIMEOUT, DURATION);
    while( (fuelSchedule7._status == PENDING) || (fuelSchedule7._status == RUNNING) ) /*Wait*/ ;
    TEST_ASSERT_EQUAL(OFF, fuelSchedule7._status);
#endif
}

void test_status_running_to_off_inj8(void)
{
#if INJ_CHANNELS >= 8
    initialiseSchedulers();
    setFuelSchedule(fuelSchedule8, TIMEOUT, DURATION);
    while( (fuelSchedule8._status == PENDING) || (fuelSchedule8._status == RUNNING) ) /*Wait*/ ;
    TEST_ASSERT_EQUAL(OFF, fuelSchedule8._status);
#endif
}


void test_status_running_to_off_ign1(void)
{
    initialiseSchedulers();
    setCallbacks(ignitionSchedule1, emptyCallback, emptyCallback);
    
    _setIgnitionScheduleDuration(ignitionSchedule1, TIMEOUT, DURATION);
    while( (ignitionSchedule1._status == PENDING) || (ignitionSchedule1._status == RUNNING) ) /*Wait*/ ;
    TEST_ASSERT_EQUAL(OFF, ignitionSchedule1._status);
}

void test_status_running_to_off_ign2(void)
{
    initialiseSchedulers();
    setCallbacks(ignitionSchedule2, emptyCallback, emptyCallback);
    
    _setIgnitionScheduleDuration(ignitionSchedule2, TIMEOUT, DURATION);
    while( (ignitionSchedule2._status == PENDING) || (ignitionSchedule2._status == RUNNING) ) /*Wait*/ ;
    TEST_ASSERT_EQUAL(OFF, ignitionSchedule2._status);
}

void test_status_running_to_off_ign3(void)
{
    initialiseSchedulers();
    setCallbacks(ignitionSchedule3, emptyCallback, emptyCallback);
    
    _setIgnitionScheduleDuration(ignitionSchedule3, TIMEOUT, DURATION);
    while( (ignitionSchedule3._status == PENDING) || (ignitionSchedule3._status == RUNNING) ) /*Wait*/ ;
    TEST_ASSERT_EQUAL(OFF, ignitionSchedule3._status);
}

void test_status_running_to_off_ign4(void)
{
    initialiseSchedulers();
    setCallbacks(ignitionSchedule4, emptyCallback, emptyCallback);
    
    _setIgnitionScheduleDuration(ignitionSchedule4, TIMEOUT, DURATION);
    while( (ignitionSchedule4._status == PENDING) || (ignitionSchedule4._status == RUNNING) ) /*Wait*/ ;
    TEST_ASSERT_EQUAL(OFF, ignitionSchedule4._status);
}

void test_status_running_to_off_ign5(void)
{
#if IGN_CHANNELS >= 5
    initialiseSchedulers();
    setCallbacks(ignitionSchedule5, emptyCallback, emptyCallback);
    
    _setIgnitionScheduleDuration(ignitionSchedule5, TIMEOUT, DURATION);
    while( (ignitionSchedule5._status == PENDING) || (ignitionSchedule5._status == RUNNING) ) /*Wait*/ ;
    TEST_ASSERT_EQUAL(OFF, ignitionSchedule5._status);
#endif
}

void test_status_running_to_off_ign6(void)
{
#if IGN_CHANNELS >= 6
    initialiseSchedulers();
    setCallbacks(ignitionSchedule6, emptyCallback, emptyCallback);
    
    _setIgnitionScheduleDuration(ignitionSchedule6, TIMEOUT, DURATION);
    while( (ignitionSchedule6._status == PENDING) || (ignitionSchedule6._status == RUNNING) ) /*Wait*/ ;
    TEST_ASSERT_EQUAL(OFF, ignitionSchedule6._status);
#endif
}

void test_status_running_to_off_ign7(void)
{
#if IGN_CHANNELS >= 7
    initialiseSchedulers();
    setCallbacks(ignitionSchedule7, emptyCallback, emptyCallback);
    
    _setIgnitionScheduleDuration(ignitionSchedule7, TIMEOUT, DURATION);
    while( (ignitionSchedule7._status == PENDING) || (ignitionSchedule7._status == RUNNING) ) /*Wait*/ ;
    TEST_ASSERT_EQUAL(OFF, ignitionSchedule7._status);
#endif
}

void test_status_running_to_off_ign8(void)
{
#if IGN_CHANNELS >= 8
    initialiseSchedulers();
    setCallbacks(ignitionSchedule8, emptyCallback, emptyCallback);
    
    _setIgnitionScheduleDuration(ignitionSchedule8, TIMEOUT, DURATION);
    while( (ignitionSchedule8._status == PENDING) || (ignitionSchedule8._status == RUNNING) ) /*Wait*/ ;
    TEST_ASSERT_EQUAL(OFF, ignitionSchedule8._status);
#endif
}

void test_status_running_to_off(void)
{
  SET_UNITY_FILENAME() {

    RUN_TEST(test_status_running_to_off_inj1);
    RUN_TEST(test_status_running_to_off_inj2);
    RUN_TEST(test_status_running_to_off_inj3);
    RUN_TEST(test_status_running_to_off_inj4);
    RUN_TEST(test_status_running_to_off_inj5);
    RUN_TEST(test_status_running_to_off_inj6);
    RUN_TEST(test_status_running_to_off_inj7);
    RUN_TEST(test_status_running_to_off_inj8);

    RUN_TEST(test_status_running_to_off_ign1);
    RUN_TEST(test_status_running_to_off_ign2);
    RUN_TEST(test_status_running_to_off_ign3);
    RUN_TEST(test_status_running_to_off_ign4);
    RUN_TEST(test_status_running_to_off_ign5);
    RUN_TEST(test_status_running_to_off_ign6);
    RUN_TEST(test_status_running_to_off_ign7);
    RUN_TEST(test_status_running_to_off_ign8);
  }
}
