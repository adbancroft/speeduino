#include <decoders.h>
#include <globals.h>
#include <unity.h>
#include "test_ngc.h"
#include "scheduler.h"
#include "../../test_utils.h"
#include "utilities.h"

extern uint16_t ignitionEndTeeth[_countof(ignitionSchedules)];

void test_ngc_newIgn_12_trig0_1()
{
    triggerSetup_NGC();
    CRANK_ANGLE_MAX_IGN = 360;
    configPage4.sparkMode = IGN_MODE_WASTED;
    configPage4.triggerAngle = 0; //No trigger offset
    
    calculateIgnitionAngles(ignitionSchedules[0], 5, 10);
    triggerSetEndTeeth_NGC();
    TEST_ASSERT_EQUAL(34, ignitionEndTeeth[0]);

    //Test again with 0 degrees advance
    calculateIgnitionAngles(ignitionSchedules[0], 5, 0);
    triggerSetEndTeeth_NGC();
    TEST_ASSERT_EQUAL(34, ignitionEndTeeth[0]);

    //Test again with 35 degrees advance
    calculateIgnitionAngles(ignitionSchedules[0], 5, 35);
    triggerSetEndTeeth_NGC();
    TEST_ASSERT_EQUAL(31, ignitionEndTeeth[0]);
}

void test_ngc_newIgn_12_trig90_1()
{
    triggerSetup_NGC();
    CRANK_ANGLE_MAX_IGN = 360;
    configPage4.sparkMode = IGN_MODE_WASTED;
    configPage4.triggerAngle = 90;
    calculateIgnitionAngles(ignitionSchedules[0], 5, 10);
    
    triggerSetEndTeeth_NGC();
    TEST_ASSERT_EQUAL(25, ignitionEndTeeth[0]);
}

void test_ngc_newIgn_12_trig180_1()
{
    triggerSetup_NGC();
    CRANK_ANGLE_MAX_IGN = 360;
    configPage4.sparkMode = IGN_MODE_WASTED;
    configPage4.triggerAngle = 180;
    calculateIgnitionAngles(ignitionSchedules[0], 5, 10);
    
    triggerSetEndTeeth_NGC();
    TEST_ASSERT_EQUAL(16, ignitionEndTeeth[0]);
}

void test_ngc_newIgn_12_trig270_1()
{
    triggerSetup_NGC();
    CRANK_ANGLE_MAX_IGN = 360;
    configPage4.sparkMode = IGN_MODE_WASTED;
    configPage4.triggerAngle = 270;
    calculateIgnitionAngles(ignitionSchedules[0], 5, 10);
    
    triggerSetEndTeeth_NGC();
    TEST_ASSERT_EQUAL(7, ignitionEndTeeth[0]);
}

void test_ngc_newIgn_12_trig360_1()
{
    triggerSetup_NGC();
    CRANK_ANGLE_MAX_IGN = 360;
    configPage4.sparkMode = IGN_MODE_WASTED;
    configPage4.triggerAngle = 360;
    calculateIgnitionAngles(ignitionSchedules[0], 5, 10);
    
    triggerSetEndTeeth_NGC();
    TEST_ASSERT_EQUAL(34, ignitionEndTeeth[0]);
}

void test_ngc_newIgn_12_trigNeg90_1()
{
    triggerSetup_NGC();
    CRANK_ANGLE_MAX_IGN = 360;
    configPage4.sparkMode = IGN_MODE_WASTED;
    configPage4.triggerAngle = -90;
    calculateIgnitionAngles(ignitionSchedules[0], 5, 10);
    
    triggerSetEndTeeth_NGC();
    TEST_ASSERT_EQUAL(7, ignitionEndTeeth[0]);
}

void test_ngc_newIgn_12_trigNeg180_1()
{
    triggerSetup_NGC();
    CRANK_ANGLE_MAX_IGN = 360;
    configPage4.sparkMode = IGN_MODE_WASTED;
    configPage4.triggerAngle = -180;
    calculateIgnitionAngles(ignitionSchedules[0], 5, 10);
    
    triggerSetEndTeeth_NGC();
    TEST_ASSERT_EQUAL(16, ignitionEndTeeth[0]);
}

void test_ngc_newIgn_12_trigNeg270_1()
{
    triggerSetup_NGC();
    CRANK_ANGLE_MAX_IGN = 360;
    configPage4.sparkMode = IGN_MODE_WASTED;
    configPage4.triggerAngle = -270;
    calculateIgnitionAngles(ignitionSchedules[0], 5, 10);
    
    triggerSetEndTeeth_NGC();
    TEST_ASSERT_EQUAL(25, ignitionEndTeeth[0]);
}

void test_ngc_newIgn_12_trigNeg360_1()
{
    triggerSetup_NGC();
    CRANK_ANGLE_MAX_IGN = 360;
    configPage4.sparkMode = IGN_MODE_WASTED;
    configPage4.triggerAngle = -360;
    calculateIgnitionAngles(ignitionSchedules[0], 5, 10);
    
    triggerSetEndTeeth_NGC();
    TEST_ASSERT_EQUAL(34, ignitionEndTeeth[0]);
}

void testNGC()
{
   SET_UNITY_FILENAME() {

    RUN_TEST(test_ngc_newIgn_12_trig0_1);
    RUN_TEST(test_ngc_newIgn_12_trig90_1);
    RUN_TEST(test_ngc_newIgn_12_trig180_1);
    RUN_TEST(test_ngc_newIgn_12_trig270_1);
    RUN_TEST(test_ngc_newIgn_12_trig360_1);
    RUN_TEST(test_ngc_newIgn_12_trigNeg90_1);
    RUN_TEST(test_ngc_newIgn_12_trigNeg180_1);
    RUN_TEST(test_ngc_newIgn_12_trigNeg270_1);
    RUN_TEST(test_ngc_newIgn_12_trigNeg360_1);
   }
}