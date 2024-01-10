#include <decoders.h>
#include <globals.h>
#include <unity.h>
#include "dual_wheel.h"
#include "scheduler.h"
#include "../../test_utils.h"
#include "utilities.h"

static decoder_t test_setup_dualwheel_12_1()
{
    //Setup a 12-1 wheel
    configPage4.triggerTeeth = 12;
    configPage4.TrigSpeed = CRANK_SPEED;
    configPage4.trigPatternSec = SEC_TRIGGER_SINGLE;
    maxIgnOutputs = 4;

    return triggerSetup_DualWheel();
}
/*
static decoder_t test_setup_dualwheel_60_2()
{
    //Setup a 60-2 wheel
    configPage4.triggerTeeth = 60;
    configPage4.triggerMissingTeeth = 2;
    configPage4.TrigSpeed = CRANK_SPEED;
    configPage4.trigPatternSec = SEC_TRIGGER_SINGLE;
    maxIgnOutputs = 4;

    return triggerSetup_missingTooth();
}
*/

extern uint16_t ignitionEndTeeth[_countof(ignitionSchedules)];

//************************************** Begin the new ignition setEndTooth tests **************************************
void test_dualwheel_newIgn_12_1_trig0_1()
{
    //Test the set end tooth function. Conditions:
    //Trigger: 12/1
    //Advance: 10
    //triggerAngle=0
    decoder_t decoder = test_setup_dualwheel_12_1();
    configPage4.sparkMode = IGN_MODE_WASTED;
    ignitionSchedules[0].dischargeAngle = 360 - 10; //Set 10 degrees advance
    configPage4.triggerAngle = 0; //No trigger offset
    
    decoder.triggerSetEndTeeth();
    TEST_ASSERT_EQUAL(11, ignitionEndTeeth[0]);

    //Test again with 0 degrees advance
    ignitionSchedules[0].dischargeAngle = 360 - 0; //Set 0 degrees advance
    decoder.triggerSetEndTeeth();
    TEST_ASSERT_EQUAL(12, ignitionEndTeeth[0]);

    //Test again with 35 degrees advance
    ignitionSchedules[0].dischargeAngle = 360 - 35; //Set 35 degrees advance
    decoder.triggerSetEndTeeth();
    TEST_ASSERT_EQUAL(10, ignitionEndTeeth[0]);
}

void test_dualwheel_newIgn_12_1_trig90_1()
{
    //Test the set end tooth function. Conditions:
    //Trigger: 12/1
    //Advance: 10
    //triggerAngle=90
    decoder_t decoder = test_setup_dualwheel_12_1();
    configPage4.sparkMode = IGN_MODE_WASTED;
    ignitionSchedules[0].dischargeAngle = 360 - 10; //Set 10 degrees advance
    configPage4.triggerAngle = 90; //No trigger offset
    
    decoder.triggerSetEndTeeth();
    TEST_ASSERT_EQUAL(8, ignitionEndTeeth[0]);
}

void test_dualwheel_newIgn_12_1_trig180_1()
{
    //Test the set end tooth function. Conditions:
    //Trigger: 36-1
    //Advance: 10
    //triggerAngle=180
    decoder_t decoder = test_setup_dualwheel_12_1();
    configPage4.sparkMode = IGN_MODE_WASTED;
    ignitionSchedules[0].dischargeAngle = 360 - 10; //Set 10 degrees advance
    configPage4.triggerAngle = 180; //No trigger offset
    
    decoder.triggerSetEndTeeth();
    TEST_ASSERT_EQUAL(5, ignitionEndTeeth[0]);
}

void test_dualwheel_newIgn_12_1_trig270_1()
{
    //Test the set end tooth function. Conditions:
    //Trigger: 36-1
    //Advance: 10
    //triggerAngle=270
    decoder_t decoder = test_setup_dualwheel_12_1();
    configPage4.sparkMode = IGN_MODE_WASTED;
    ignitionSchedules[0].dischargeAngle = 360 - 10; //Set 10 degrees advance
    configPage4.triggerAngle = 270; //No trigger offset
    
    decoder.triggerSetEndTeeth();
    TEST_ASSERT_EQUAL(2, ignitionEndTeeth[0]);
}

void test_dualwheel_newIgn_12_1_trig360_1()
{
    //Test the set end tooth function. Conditions:
    //Trigger: 36-1
    //Advance: 10
    //triggerAngle=360
    decoder_t decoder = test_setup_dualwheel_12_1();
    configPage4.sparkMode = IGN_MODE_WASTED;
    ignitionSchedules[0].dischargeAngle = 360 - 10; //Set 10 degrees advance
    configPage4.triggerAngle = 360; //No trigger offset
    
    decoder.triggerSetEndTeeth();
    TEST_ASSERT_EQUAL(12, ignitionEndTeeth[0]);
}

void test_dualwheel_newIgn_12_1_trigNeg90_1()
{
    //Test the set end tooth function. Conditions:
    //Trigger: 36-1
    //Advance: 10
    //triggerAngle=-90
    decoder_t decoder = test_setup_dualwheel_12_1();
    configPage4.sparkMode = IGN_MODE_WASTED;
    ignitionSchedules[0].dischargeAngle = 360 - 10; //Set 10 degrees advance
    configPage4.triggerAngle = -90; //No trigger offset
    
    decoder.triggerSetEndTeeth();
    TEST_ASSERT_EQUAL(2, ignitionEndTeeth[0]);
}

void test_dualwheel_newIgn_12_1_trigNeg180_1()
{
    //Test the set end tooth function. Conditions:
    //Trigger: 36-1
    //Advance: 10
    //triggerAngle=-180
    decoder_t decoder = test_setup_dualwheel_12_1();
    configPage4.sparkMode = IGN_MODE_WASTED;
    ignitionSchedules[0].dischargeAngle = 360 - 10; //Set 10 degrees advance
    configPage4.triggerAngle = -180; //No trigger offset
    
    decoder.triggerSetEndTeeth();
    TEST_ASSERT_EQUAL(5, ignitionEndTeeth[0]);
}

void test_dualwheel_newIgn_12_1_trigNeg270_1()
{
    //Test the set end tooth function. Conditions:
    //Trigger: 36-1
    //Advance: 10
    //triggerAngle=-270
    decoder_t decoder = test_setup_dualwheel_12_1();
    configPage4.sparkMode = IGN_MODE_WASTED;
    ignitionSchedules[0].dischargeAngle = 360 - 10; //Set 10 degrees advance
    configPage4.triggerAngle = -270; //No trigger offset
    
    decoder.triggerSetEndTeeth();
    TEST_ASSERT_EQUAL(8, ignitionEndTeeth[0]);
}

void test_dualwheel_newIgn_12_1_trigNeg360_1()
{
    //Test the set end tooth function. Conditions:
    //Trigger: 36-1
    //Advance: 10
    //triggerAngle=-360
    decoder_t decoder = test_setup_dualwheel_12_1();
    configPage4.sparkMode = IGN_MODE_WASTED;
    ignitionSchedules[0].dischargeAngle = 360 - 10; //Set 10 degrees advance
    configPage4.triggerAngle = -360; //No trigger offset
    
    decoder.triggerSetEndTeeth();
    TEST_ASSERT_EQUAL(11, ignitionEndTeeth[0]);
}

// ******* CHannel 2 *******
void test_dualwheel_newIgn_12_1_trig0_2()
{
    //Test the set end tooth function. Conditions:
    //Trigger: 36-1
    //Advance: 10
    //triggerAngle=0
    decoder_t decoder = test_setup_dualwheel_12_1();
    configPage4.sparkMode = IGN_MODE_WASTED;
    ignitionSchedules[1].dischargeAngle = 180 - 10; //Set 10 degrees advance
    configPage4.triggerAngle = 0; //No trigger offset
    
    decoder.triggerSetEndTeeth();
    TEST_ASSERT_EQUAL(16, ignitionEndTeeth[1]);
}

void test_dualwheel_newIgn_12_1_trig90_2()
{
    //Test the set end tooth function. Conditions:
    //Trigger: 36-1
    //Advance: 10
    //triggerAngle=90
    decoder_t decoder = test_setup_dualwheel_12_1();
    configPage4.sparkMode = IGN_MODE_WASTED;
    ignitionSchedules[1].dischargeAngle = 180 - 10; //Set 10 degrees advance
    configPage4.triggerAngle = 90; //No trigger offset
    
    decoder.triggerSetEndTeeth();
    TEST_ASSERT_EQUAL(7, ignitionEndTeeth[1]);
}

void test_dualwheel_newIgn_12_1_trig180_2()
{
    //Test the set end tooth function. Conditions:
    //Trigger: 36-1
    //Advance: 10
    //triggerAngle=180
    decoder_t decoder = test_setup_dualwheel_12_1();
    configPage4.sparkMode = IGN_MODE_WASTED;
    ignitionSchedules[1].dischargeAngle = 180 - 10; //Set 10 degrees advance
    configPage4.triggerAngle = 180; //No trigger offset
    
    decoder.triggerSetEndTeeth();
    TEST_ASSERT_EQUAL(34, ignitionEndTeeth[1]);
}

void test_dualwheel_newIgn_12_1_trig270_2()
{
    //Test the set end tooth function. Conditions:
    //Trigger: 36-1
    //Advance: 10
    //triggerAngle=270
    decoder_t decoder = test_setup_dualwheel_12_1();
    configPage4.sparkMode = IGN_MODE_WASTED;
    ignitionSchedules[1].dischargeAngle = 180 - 10; //Set 10 degrees advance
    configPage4.triggerAngle = 270; //No trigger offset
    
    decoder.triggerSetEndTeeth();
    TEST_ASSERT_EQUAL(25, ignitionEndTeeth[1]);
}

void test_dualwheel_newIgn_12_1_trig360_2()
{
    //Test the set end tooth function. Conditions:
    //Trigger: 36-1
    //Advance: 10
    //triggerAngle=360
    decoder_t decoder = test_setup_dualwheel_12_1();
    configPage4.sparkMode = IGN_MODE_WASTED;
    ignitionSchedules[1].dischargeAngle = 180 - 10; //Set 10 degrees advance
    configPage4.triggerAngle = 360; //No trigger offset
    
    decoder.triggerSetEndTeeth();
    TEST_ASSERT_EQUAL(16, ignitionEndTeeth[1]);
}

void test_dualwheel_newIgn_12_1_trigNeg90_2()
{
    //Test the set end tooth function. Conditions:
    //Trigger: 36-1
    //Advance: 10
    //triggerAngle=-90
    decoder_t decoder = test_setup_dualwheel_12_1();
    configPage4.sparkMode = IGN_MODE_WASTED;
    ignitionSchedules[1].dischargeAngle = 180 - 10; //Set 10 degrees advance
    configPage4.triggerAngle = -90; //No trigger offset
    
    decoder.triggerSetEndTeeth();
    TEST_ASSERT_EQUAL(25, ignitionEndTeeth[1]);
}

void test_dualwheel_newIgn_12_1_trigNeg180_2()
{
    //Test the set end tooth function. Conditions:
    //Trigger: 36-1
    //Advance: 10
    //triggerAngle=-180
    decoder_t decoder = test_setup_dualwheel_12_1();
    configPage4.sparkMode = IGN_MODE_WASTED;
    ignitionSchedules[1].dischargeAngle = 180 - 10; //Set 10 degrees advance
    configPage4.triggerAngle = -180; //No trigger offset
    
    decoder.triggerSetEndTeeth();
    TEST_ASSERT_EQUAL(34, ignitionEndTeeth[1]);
}

void test_dualwheel_newIgn_12_1_trigNeg270_2()
{
    //Test the set end tooth function. Conditions:
    //Trigger: 36-1
    //Advance: 10
    //triggerAngle=-270
    decoder_t decoder = test_setup_dualwheel_12_1();
    configPage4.sparkMode = IGN_MODE_WASTED;
    ignitionSchedules[1].dischargeAngle = 180 - 10; //Set 10 degrees advance
    configPage4.triggerAngle = -270; //No trigger offset
    
    decoder.triggerSetEndTeeth();
    TEST_ASSERT_EQUAL(7, ignitionEndTeeth[1]);
}

void test_dualwheel_newIgn_12_1_trigNeg360_2()
{
    //Test the set end tooth function. Conditions:
    //Trigger: 36-1
    //Advance: 10
    //triggerAngle=-360
    decoder_t decoder = test_setup_dualwheel_12_1();
    configPage4.sparkMode = IGN_MODE_WASTED;
    ignitionSchedules[1].dischargeAngle = 180 - 10; //Set 10 degrees advance
    configPage4.triggerAngle = -360; //No trigger offset
    
    decoder.triggerSetEndTeeth();
    TEST_ASSERT_EQUAL(16, ignitionEndTeeth[1]);
}

void test_dualwheel_newIgn_2()
{

}
void test_dualwheel_newIgn_3()
{

}
void test_dualwheel_newIgn_4()
{

}

void testDualWheel()
{
  SET_UNITY_FILENAME() {

  RUN_TEST(test_dualwheel_newIgn_12_1_trig0_1);
  RUN_TEST(test_dualwheel_newIgn_12_1_trig90_1);
  RUN_TEST(test_dualwheel_newIgn_12_1_trig180_1);
  RUN_TEST(test_dualwheel_newIgn_12_1_trig270_1);
  RUN_TEST(test_dualwheel_newIgn_12_1_trig360_1);
  RUN_TEST(test_dualwheel_newIgn_12_1_trigNeg90_1);
  RUN_TEST(test_dualwheel_newIgn_12_1_trigNeg180_1);
  RUN_TEST(test_dualwheel_newIgn_12_1_trigNeg270_1);
  RUN_TEST(test_dualwheel_newIgn_12_1_trigNeg360_1);

/*
  RUN_TEST(test_dualwheel_newIgn_12_1_trig0_2);
  RUN_TEST(test_dualwheel_newIgn_12_1_trig90_2);
  RUN_TEST(test_dualwheel_newIgn_12_1_trig180_2);
  RUN_TEST(test_dualwheel_newIgn_12_1_trig270_2);
  RUN_TEST(test_dualwheel_newIgn_12_1_trig360_2);
  RUN_TEST(test_dualwheel_newIgn_12_1_trigNeg90_2);
  RUN_TEST(test_dualwheel_newIgn_12_1_trigNeg180_2);
  RUN_TEST(test_dualwheel_newIgn_12_1_trigNeg270_2);
  RUN_TEST(test_dualwheel_newIgn_12_1_trigNeg360_2);
*/
  //RUN_TEST(test_dualwheel_newIgn_60_2_trig181_2);
  //RUN_TEST(test_dualwheel_newIgn_60_2_trig182_2);

  }
}