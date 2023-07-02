#include <decoders.h>
#include <globals.h>
#include <unity.h>
#include "FordST170.h"
#include "scheduler.h"
#include "../../test_utils.h"
#include "utilities.h"

extern uint16_t ignitionEndTeeth[_countof(ignitionSchedules)];

void testSetup_FordST170(void)
{
    triggerSetup_FordST170();
    maxIgnOutputs = 4;
}

void test_fordst170_newIgn_12_trig0_1()
{
    //Test the set end tooth function. Conditions:
    //Trigger: 12/1
    //Advance: 10
    //triggerAngle=0
    testSetup_FordST170();
    configPage4.sparkMode = IGN_MODE_WASTED;
    configPage4.triggerAngle = 0; //No trigger offset
    calculateIgnitionAngles(ignitionSchedules[0], 5, 10);
  
    triggerSetEndTeeth_FordST170();
    TEST_ASSERT_EQUAL(34, ignitionEndTeeth[0]);

    //Test again with 0 degrees advance
    calculateIgnitionAngles(ignitionSchedules[0], 5, 0);

    triggerSetEndTeeth_FordST170();
    TEST_ASSERT_EQUAL(35, ignitionEndTeeth[0]);

    //Test again with 35 degrees advance
    calculateIgnitionAngles(ignitionSchedules[0], 5, 35);

    triggerSetEndTeeth_FordST170();
    TEST_ASSERT_EQUAL(31, ignitionEndTeeth[0]);
}

void test_fordst170_newIgn_12_trig90_1()
{
    //Test the set end tooth function. Conditions:
    //Trigger: 12/1
    //Advance: 10
    //triggerAngle=90
    testSetup_FordST170();
    configPage4.sparkMode = IGN_MODE_WASTED;
    configPage4.triggerAngle = 90; //No trigger offset
    calculateIgnitionAngles(ignitionSchedules[0], 5, 35);

    triggerSetEndTeeth_FordST170();
    TEST_ASSERT_EQUAL(22, ignitionEndTeeth[0]);
}

void test_fordst170_newIgn_12_trig180_1()
{
    //Test the set end tooth function. Conditions:
    //Trigger: 36-1
    //Advance: 10
    //triggerAngle=180
    testSetup_FordST170();
    configPage4.sparkMode = IGN_MODE_WASTED;
    configPage4.triggerAngle = 180; //No trigger offset
    calculateIgnitionAngles(ignitionSchedules[0], 5, 10);
 
    triggerSetEndTeeth_FordST170();
    TEST_ASSERT_EQUAL(16, ignitionEndTeeth[0]);
}

void test_fordst170_newIgn_12_trig270_1()
{
    //Test the set end tooth function. Conditions:
    //Trigger: 36-1
    //Advance: 10
    //triggerAngle=270
    testSetup_FordST170();
    configPage4.sparkMode = IGN_MODE_WASTED;
    configPage4.triggerAngle = 270; //No trigger offset
    calculateIgnitionAngles(ignitionSchedules[0], 5, 10);

    triggerSetEndTeeth_FordST170();
    TEST_ASSERT_EQUAL(7, ignitionEndTeeth[0]);
}

void test_fordst170_newIgn_12_trig360_1()
{
    //Test the set end tooth function. Conditions:
    //Trigger: 36-1
    //Advance: 10
    //triggerAngle=360
    testSetup_FordST170();
    configPage4.sparkMode = IGN_MODE_WASTED;
    configPage4.triggerAngle = 360; //No trigger offset
    calculateIgnitionAngles(ignitionSchedules[0], 5, 10);
    
    triggerSetEndTeeth_FordST170();
    TEST_ASSERT_EQUAL(34, ignitionEndTeeth[0]);
}

void test_fordst170_newIgn_12_trigNeg90_1()
{
    //Test the set end tooth function. Conditions:
    //Trigger: 36-1
    //Advance: 10
    //triggerAngle=-90
    testSetup_FordST170();
    configPage4.sparkMode = IGN_MODE_WASTED;
    configPage4.triggerAngle = -90; //No trigger offset
    calculateIgnitionAngles(ignitionSchedules[0], 5, 10);

    triggerSetEndTeeth_FordST170();
    TEST_ASSERT_EQUAL(7, ignitionEndTeeth[0]);
}

void test_fordst170_newIgn_12_trigNeg180_1()
{
    //Test the set end tooth function. Conditions:
    //Trigger: 36-1
    //Advance: 10
    //triggerAngle=-180
    testSetup_FordST170();
    configPage4.sparkMode = IGN_MODE_WASTED;
    configPage4.triggerAngle = -180; //No trigger offset
    calculateIgnitionAngles(ignitionSchedules[0], 5, 10);

    triggerSetEndTeeth_FordST170();
    TEST_ASSERT_EQUAL(16, ignitionEndTeeth[0]);
}

void test_fordst170_newIgn_12_trigNeg270_1()
{
    //Test the set end tooth function. Conditions:
    //Trigger: 36-1
    //Advance: 10
    //triggerAngle=-270
    testSetup_FordST170();
    configPage4.sparkMode = IGN_MODE_WASTED;
    configPage4.triggerAngle = -270; //No trigger offset
    calculateIgnitionAngles(ignitionSchedules[0], 5, 10);
    
    triggerSetEndTeeth_FordST170();
    TEST_ASSERT_EQUAL(25, ignitionEndTeeth[0]);
}

void test_fordst170_newIgn_12_trigNeg360_1()
{
    //Test the set end tooth function. Conditions:
    //Trigger: 36-1
    //Advance: 10
    //triggerAngle=-360
    testSetup_FordST170();
    configPage4.sparkMode = IGN_MODE_WASTED;
    configPage4.triggerAngle = -360; //No trigger offset
    calculateIgnitionAngles(ignitionSchedules[0], 5, 10);

    triggerSetEndTeeth_FordST170();
    TEST_ASSERT_EQUAL(34, ignitionEndTeeth[0]);
}

void testFordST170()
{
    SET_UNITY_FILENAME() {

    RUN_TEST(test_fordst170_newIgn_12_trig0_1);
    RUN_TEST(test_fordst170_newIgn_12_trig90_1);
    RUN_TEST(test_fordst170_newIgn_12_trig180_1);
    RUN_TEST(test_fordst170_newIgn_12_trig270_1);
    RUN_TEST(test_fordst170_newIgn_12_trig360_1);
    RUN_TEST(test_fordst170_newIgn_12_trigNeg90_1);
    RUN_TEST(test_fordst170_newIgn_12_trigNeg180_1);
    RUN_TEST(test_fordst170_newIgn_12_trigNeg270_1);
    RUN_TEST(test_fordst170_newIgn_12_trigNeg360_1);
    
    }
}