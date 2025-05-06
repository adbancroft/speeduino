#include <Arduino.h>
#include <unity.h>
#include "globals.h"
#include "init.h"
#include "scheduler.h"
#include "scheduledIO.h"
#include "utilities.h"
#include "../test_utils.h"
#include "storage.h"

extern uint16_t req_fuel_uS;
void prepareForInitialiseAll(uint8_t boardId);

static constexpr uint16_t reqFuel = 86; // ms * 10

static void assert_fuel_channel(bool enabled, uint16_t angle, uint8_t channelIndex, const FuelSchedule &schedule)
{
  char msg[39];

  sprintf_P(msg, PSTR("channel%" PRIu8 ".InjChannelIsEnabled. Max:%" PRIu8), channelIndex+1, totalInjOutputs());
  TEST_ASSERT_TRUE_MESSAGE(!enabled || (channelIndex+1)<=totalInjOutputs(), msg);
  sprintf_P(msg, PSTR("channel%" PRIu8 "InjDegrees"), channelIndex+1);
  TEST_ASSERT_EQUAL_MESSAGE(angle, schedule.channelDegrees, msg);
  sprintf_P(msg, PSTR("inj%" PRIu8 "StartFunction: %" PRId16), channelIndex+1, (int16_t)enabled);
  TEST_ASSERT_TRUE_MESSAGE(!enabled || (schedule._pStartCallback!=nullCallback), msg);
  sprintf_P(msg, PSTR("inj%" PRIu8 "EndFunction: %" PRId16), channelIndex+1, (int16_t)enabled);
  TEST_ASSERT_TRUE_MESSAGE(!enabled || (schedule._pEndCallback!=nullCallback), msg);
}

static void assert_fuel_schedules(uint16_t crankAngle, uint16_t reqFuel, uint16_t expectedPrimaries, uint16_t expectedSecondaries, const uint16_t angle[], uint8_t injLayout)
{
  char msg[64];

  strcpy_P(msg, PSTR("CRANK_ANGLE_MAX_INJ"));
  TEST_ASSERT_EQUAL_INT16_MESSAGE(crankAngle, CRANK_ANGLE_MAX_INJ, msg);
  strcpy_P(msg, PSTR("req_fuel_uS"));
  TEST_ASSERT_EQUAL_UINT16_MESSAGE(reqFuel, req_fuel_uS, msg);
  strcpy_P(msg, PSTR("imaxInjPrimaryOutputs"));
  TEST_ASSERT_EQUAL_UINT16_MESSAGE(expectedPrimaries, maxInjPrimaryOutputs, msg);
  strcpy_P(msg, PSTR("maxInjSecondaryOutputs"));
  TEST_ASSERT_EQUAL_UINT16_MESSAGE(expectedSecondaries, maxInjSecondaryOutputs, msg);
  strcpy_P(msg, PSTR("injLayout"));
  TEST_ASSERT_EQUAL_MESSAGE(injLayout, configPage2.injLayout, msg);
  
  for (uint8_t index=0; index<_countof(fuelSchedules); ++index) {
    assert_fuel_channel(expectedPrimaries+expectedSecondaries>index, angle[index], index, fuelSchedules[index]);
  }
}

static void cylinder1_stroke4_seq_nostage(void)
{
  configPage2.injLayout = INJ_SEQUENTIAL;
  configPage2.injTiming = INJ_TIMING_ALTERNATING;;
  configPage10.stagingEnabled = false;
  initialiseAll(); //Run the main initialise function
	const uint16_t angle[] = {0,0,0,0,0,0,0,0};
  assert_fuel_schedules(720U, reqFuel * 100U, 1U, 0U, angle, INJ_SEQUENTIAL);
}

static void cylinder1_stroke4_semiseq_nostage(void)
{
  configPage2.injLayout = INJ_SEMISEQUENTIAL;
  configPage2.injTiming = INJ_TIMING_ALTERNATING;;
  configPage10.stagingEnabled = false;
  initialiseAll(); //Run the main initialise function
	const uint16_t angle[] = {0,0,0,0,0,0,0,0};
  assert_fuel_schedules(720U, reqFuel * 50U, 1U, 0U, angle, INJ_PAIRED);
}

static void cylinder1_stroke4_seq_staged(void)
{
  configPage2.injLayout = INJ_SEQUENTIAL;
  configPage2.injTiming = INJ_TIMING_ALTERNATING;;
  configPage10.stagingEnabled = true;
  initialiseAll(); //Run the main initialise function
	const uint16_t angle[] = {0,0,0,0,0,0,0,0};
  assert_fuel_schedules(720U, reqFuel * 100U, 1U, 1U, angle, INJ_SEQUENTIAL);
  }

static void cylinder1_stroke4_semiseq_staged(void)
{
  configPage2.injLayout = INJ_SEMISEQUENTIAL;
  configPage2.injTiming = INJ_TIMING_ALTERNATING;;
  configPage10.stagingEnabled = true;
  initialiseAll(); //Run the main initialise function
	const uint16_t angle[] = {0,0,0,0,0,0,0,0};
  assert_fuel_schedules(720U, reqFuel * 50U, 1U, 1U, angle, INJ_PAIRED);
}

static void run_1_cylinder_4stroke_tests(void)
{
  prepareForInitialiseAll(3U);
  configPage2.nCylinders = 1;
  configPage2.strokes = FOUR_STROKE;
  configPage2.engineType = EVEN_FIRE;
  configPage2.reqFuel = reqFuel; 
  configPage2.divider = 1;

  RUN_TEST_P(cylinder1_stroke4_seq_nostage);
  RUN_TEST_P(cylinder1_stroke4_semiseq_nostage);
  RUN_TEST_P(cylinder1_stroke4_seq_staged);
  RUN_TEST_P(cylinder1_stroke4_semiseq_staged);
}

static void cylinder1_stroke2_seq_nostage(void)
{
  configPage2.injLayout = INJ_SEQUENTIAL;
  configPage2.injTiming = INJ_TIMING_ALTERNATING;;
  configPage10.stagingEnabled = false;
  initialiseAll(); //Run the main initialise function
  const uint16_t angle[] = {0,0,0,0,0,0,0,0};
  assert_fuel_schedules(360U, reqFuel * 100U, 1U, 0U, angle, INJ_PAIRED);  
}

static void cylinder1_stroke2_semiseq_nostage(void)
{
  configPage2.injLayout = INJ_SEMISEQUENTIAL;
  configPage2.injTiming = INJ_TIMING_ALTERNATING;;
  configPage10.stagingEnabled = false;
  initialiseAll(); //Run the main initialise function
	const uint16_t angle[] = {0,0,0,0,0,0,0,0};
  assert_fuel_schedules(360U, reqFuel * 100U, 1U, 0U, angle, INJ_PAIRED);
}

static void cylinder1_stroke2_seq_staged(void)
{
  configPage2.injLayout = INJ_SEQUENTIAL;
  configPage2.injTiming = INJ_TIMING_ALTERNATING;;
  configPage10.stagingEnabled = true;
  initialiseAll(); //Run the main initialise function
	const uint16_t angle[] = {0,0,0,0,0,0,0,0};
  assert_fuel_schedules(360U, reqFuel * 100U, 1U, 1U, angle, INJ_PAIRED);
}

static void cylinder1_stroke2_semiseq_staged(void)
{
  configPage2.injLayout = INJ_SEMISEQUENTIAL;
  configPage2.injTiming = INJ_TIMING_ALTERNATING;;
  configPage10.stagingEnabled = true;
  initialiseAll(); //Run the main initialise function
  const uint16_t angle[] = {0,0,0,0,0,0,0,0};
  assert_fuel_schedules(360U, reqFuel * 100U, 1U, 1U, angle, INJ_PAIRED);  
}

static void run_1_cylinder_2stroke_tests(void)
{
  prepareForInitialiseAll(3U);
  configPage2.nCylinders = 1;
  configPage2.strokes = TWO_STROKE;
  configPage2.engineType = EVEN_FIRE;
  configPage2.reqFuel = reqFuel;
  configPage2.divider = 1;

  RUN_TEST_P(cylinder1_stroke2_seq_nostage);
  RUN_TEST_P(cylinder1_stroke2_semiseq_nostage);
  RUN_TEST_P(cylinder1_stroke2_seq_staged);
  RUN_TEST_P(cylinder1_stroke2_semiseq_staged);
}

static void cylinder2_stroke4_seq_nostage(void)
{
  configPage2.injLayout = INJ_SEQUENTIAL;
  configPage2.injTiming = INJ_TIMING_ALTERNATING;;
  configPage10.stagingEnabled = false;
  initialiseAll(); //Run the main initialise function
	const uint16_t angle[] = {0,360,0,0,0,0,0,0};
  assert_fuel_schedules(720U, reqFuel * 100U, 2U, 0U, angle, INJ_SEQUENTIAL);
}

static void cylinder2_stroke4_semiseq_nostage(void)
{
  configPage2.injLayout = INJ_SEMISEQUENTIAL;
  configPage2.injTiming = INJ_TIMING_ALTERNATING;;
  configPage10.stagingEnabled = false;
  initialiseAll(); //Run the main initialise function
	const uint16_t angle[] = {0,180,0,0,0,0,0,0};
  assert_fuel_schedules(360U, reqFuel * 50U, 2U, 0U, angle, INJ_PAIRED);
}

static void cylinder2_stroke4_seq_staged(void)
{
  configPage2.injLayout = INJ_SEQUENTIAL;
  configPage2.injTiming = INJ_TIMING_ALTERNATING;;
  configPage10.stagingEnabled = true;
  initialiseAll(); //Run the main initialise function
	const uint16_t angle[] = {0,360,0,360,0,0,0,0};
  assert_fuel_schedules(720U, reqFuel * 100U, 2U, 2U, angle, INJ_SEQUENTIAL);
}

static void cylinder2_stroke4_semiseq_staged(void)
{
  configPage2.injLayout = INJ_SEMISEQUENTIAL;
  configPage2.injTiming = INJ_TIMING_ALTERNATING;;
  configPage10.stagingEnabled = true;
  initialiseAll(); //Run the main initialise function
	const uint16_t angle[] = {0,180,0,180,0,0,0,0};
  assert_fuel_schedules(360U, reqFuel * 50U, 2U, 2U, angle, INJ_PAIRED);
}

static void run_2_cylinder_4stroke_tests(void)
{
  prepareForInitialiseAll(3U);
  configPage2.nCylinders = 2;
  configPage2.strokes = FOUR_STROKE;
  configPage2.engineType = EVEN_FIRE;
  configPage2.reqFuel = reqFuel;
  configPage2.divider = 1;

  RUN_TEST_P(cylinder2_stroke4_seq_nostage);
  RUN_TEST_P(cylinder2_stroke4_semiseq_nostage);
  RUN_TEST_P(cylinder2_stroke4_seq_staged);
  RUN_TEST_P(cylinder2_stroke4_semiseq_staged);
}


static void cylinder2_stroke2_seq_nostage(void)
{
  configPage2.injLayout = INJ_SEQUENTIAL;
  configPage2.injTiming = INJ_TIMING_ALTERNATING;;
  configPage10.stagingEnabled = false;
  initialiseAll(); //Run the main initialise function
	const uint16_t angle[] = {0,90,0,0,0,0,0,0};
  assert_fuel_schedules(180U, reqFuel * 100U, 2U, 0U, angle, INJ_PAIRED);
}

static void cylinder2_stroke2_semiseq_nostage(void)
{
  configPage2.injLayout = INJ_SEMISEQUENTIAL;
  configPage2.injTiming = INJ_TIMING_ALTERNATING;;
  configPage10.stagingEnabled = false;
  initialiseAll(); //Run the main initialise function
	const uint16_t angle[] = {0,90,0,0,0,0,0,0};
  assert_fuel_schedules(180U, reqFuel * 100U, 2U, 0U, angle, INJ_PAIRED);
}

static void cylinder2_stroke2_seq_staged(void)
{
  configPage2.injLayout = INJ_SEQUENTIAL;
  configPage2.injTiming = INJ_TIMING_ALTERNATING;;
  configPage10.stagingEnabled = true;
  initialiseAll(); //Run the main initialise function
	const uint16_t angle[] = {0,90,0,90,0,0,0,0};
  assert_fuel_schedules(180U, reqFuel * 100U, 2U, 2U, angle, INJ_PAIRED);
}

static void cylinder2_stroke2_semiseq_staged(void)
{
  configPage2.injLayout = INJ_SEMISEQUENTIAL;
  configPage2.injTiming = INJ_TIMING_ALTERNATING;;
  configPage10.stagingEnabled = true;
  initialiseAll(); //Run the main initialise function
	const uint16_t angle[] = {0,90,0,90,0,0,0,0};
  assert_fuel_schedules(180U, reqFuel * 100U, 2U, 2U, angle, INJ_PAIRED);
}

static void run_2_cylinder_2stroke_tests(void)
{
  prepareForInitialiseAll(3U);
  configPage2.nCylinders = 2;
  configPage2.strokes = TWO_STROKE;
  configPage2.engineType = EVEN_FIRE;
  configPage2.reqFuel = reqFuel;
  configPage2.divider = 1;

  RUN_TEST_P(cylinder2_stroke2_seq_nostage);
  RUN_TEST_P(cylinder2_stroke2_semiseq_nostage);
  RUN_TEST_P(cylinder2_stroke2_seq_staged);
  RUN_TEST_P(cylinder2_stroke2_semiseq_staged);
}

static void cylinder3_stroke4_seq_nostage(void)
{
  configPage2.injLayout = INJ_SEQUENTIAL;
  configPage2.injTiming = INJ_TIMING_ALTERNATING;;
  configPage10.stagingEnabled = false;
  initialiseAll(); //Run the main initialise function
	const uint16_t angle[] = {0,240,480,0,0,0,0,0};
  assert_fuel_schedules(720U, reqFuel * 100U, 3U, 0U, angle, INJ_SEQUENTIAL);
}

static void cylinder3_stroke4_semiseq_nostage_tb(void)
{
  configPage2.injLayout = INJ_SEMISEQUENTIAL;
  configPage2.injTiming = INJ_TIMING_ALTERNATING;;
  configPage10.stagingEnabled = false;
  configPage2.injType = INJ_TYPE_TBODY;
  initialiseAll(); //Run the main initialise function
	const uint16_t angle[] = {0,80,160,0,0,0,0,0};
  assert_fuel_schedules(720U/3U, reqFuel * 50U, 3U, 0U, angle, INJ_PAIRED);
}

static void cylinder3_stroke4_semiseq_nostage_port(void)
{
  configPage2.injLayout = INJ_SEMISEQUENTIAL;
  configPage2.injTiming = true;
  configPage10.stagingEnabled = false;
  configPage2.injType = INJ_TYPE_PORT;
  initialiseAll(); //Run the main initialise function
	const uint16_t angle[] = {0,120,240,0,0,0,0,0};
  assert_fuel_schedules(720U/2U, reqFuel * 50U, 3U, 0U, angle, INJ_PAIRED);
}


static void cylinder3_stroke4_seq_staged(void)
{
  configPage2.injLayout = INJ_SEQUENTIAL;
  configPage2.injTiming = INJ_TIMING_ALTERNATING;;
  configPage10.stagingEnabled = true;
  initialiseAll(); //Run the main initialise function
#if INJ_CHANNELS>=6
	const uint16_t angle[] = {0,240,480,0,240,480,0,0};
  assert_fuel_schedules(720U, reqFuel * 100U, 3U, 3U, angle, INJ_SEQUENTIAL);
#else
	const uint16_t angle[] = {0,240,480,0,0,0,0,0};
  assert_fuel_schedules(720U, reqFuel * 100U, 3U, 1U, angle, INJ_SEQUENTIAL);
#endif
}

static void cylinder3_stroke4_semiseq_staged_tb(void)
{
  configPage2.injLayout = INJ_SEMISEQUENTIAL;
  configPage10.stagingEnabled = true;
  configPage2.injType = INJ_TYPE_TBODY;
  initialiseAll(); //Run the main initialise function
#if INJ_CHANNELS>=6
	const uint16_t angle[] = {0,80,160,0,80,160,0,0};
  assert_fuel_schedules(720U/3U, reqFuel * 50U, 3U, 3U, angle, INJ_PAIRED); //Special case as 3 squirts per cycle MUST be over 720 degrees
#else
	const uint16_t angle[] = {0,80,160,0,0,0,0,0};
  assert_fuel_schedules(720U/3U, reqFuel * 50U, 3U, 1U, angle, INJ_PAIRED); //Special case as 3 squirts per cycle MUST be over 720 degrees
#endif
}


static void cylinder3_stroke4_semiseq_staged_port(void)
{
  configPage2.injLayout = INJ_SEMISEQUENTIAL;
  configPage10.stagingEnabled = true;
  configPage2.injType = INJ_TYPE_PORT;
  initialiseAll(); //Run the main initialise function
#if INJ_CHANNELS>=6
	const uint16_t angle[] = {0,120,240,0,120,240,0,0};
  assert_fuel_schedules(720U/2U, reqFuel * 50U, 3U, 3U, angle, INJ_PAIRED);
#else
	const uint16_t angle[] = {0,120,240,0,0,0,0,0};
  assert_fuel_schedules(720U/2U, reqFuel * 50U, 3U, 1U, angle, INJ_PAIRED); 
#endif
}

static void run_3_cylinder_4stroke_tests(void)
{
  prepareForInitialiseAll(3U);
  configPage2.nCylinders = 3;
  configPage2.strokes = FOUR_STROKE;
  configPage2.engineType = EVEN_FIRE;
  configPage2.reqFuel = reqFuel;
  configPage2.divider = 1; //3 squirts per cycle for a 3 cylinder

  RUN_TEST_P(cylinder3_stroke4_seq_nostage);
  RUN_TEST_P(cylinder3_stroke4_semiseq_nostage_tb);
  RUN_TEST_P(cylinder3_stroke4_semiseq_nostage_port);
  RUN_TEST_P(cylinder3_stroke4_seq_staged);
  RUN_TEST_P(cylinder3_stroke4_semiseq_staged_tb);
  RUN_TEST_P(cylinder3_stroke4_semiseq_staged_port);
}

static void cylinder3_stroke2_seq_nostage(void)
{
  configPage2.injLayout = INJ_SEQUENTIAL;
  configPage10.stagingEnabled = false;
  initialiseAll(); //Run the main initialise function
	const uint16_t angle[] = {0,60,120,0,0,0,0,0};
  assert_fuel_schedules(180U, reqFuel * 100U, 3U, 0U, angle, INJ_PAIRED);
  }

static void cylinder3_stroke2_semiseq_nostage_tb(void)
{
  configPage2.injLayout = INJ_SEMISEQUENTIAL;
  configPage10.stagingEnabled = false;
  configPage2.injType = INJ_TYPE_TBODY;
  initialiseAll(); //Run the main initialise function
	const uint16_t angle[] = {0,40,80,0,0,0,0,0};
  assert_fuel_schedules(360U/3U, reqFuel * 100U, 3U, 0U, angle, INJ_PAIRED);
}

static void cylinder3_stroke2_semiseq_nostage_port(void)
{
  configPage2.injLayout = INJ_SEMISEQUENTIAL;
  configPage10.stagingEnabled = false;
  configPage2.injType = INJ_TYPE_PORT;
  initialiseAll(); //Run the main initialise function
	const uint16_t angle[] = {0,60,120,0,0,0,0,0};
  assert_fuel_schedules(360U/2U, reqFuel * 100U, 3U, 0U, angle, INJ_PAIRED);
}

static void cylinder3_stroke2_seq_staged(void)
{
  configPage2.injLayout = INJ_SEQUENTIAL;
  configPage10.stagingEnabled = true;
  initialiseAll(); //Run the main initialise function
#if INJ_CHANNELS>=6
	const uint16_t angle[] = {0,60,120,0,60,120,0,0};
  assert_fuel_schedules(180U, reqFuel * 100U, 3U, 3U, angle, INJ_PAIRED);
#else
	const uint16_t angle[] = {0,60,120,0,0,0,0};
  assert_fuel_schedules(180U, reqFuel * 100U, 3U, 1U, angle, INJ_PAIRED);
#endif
  }

static void cylinder3_stroke2_semiseq_staged_tb(void)
{
  configPage2.injLayout = INJ_SEMISEQUENTIAL;
  configPage10.stagingEnabled = true;
  configPage2.injType = INJ_TYPE_TBODY;
  initialiseAll(); //Run the main initialise function
#if INJ_CHANNELS>=6
	const uint16_t angle[] = {0,40,80,0,40,80,0,0};
  assert_fuel_schedules(360U/3U, reqFuel * 100U, 3U, 3U, angle, INJ_PAIRED);
#else
	const uint16_t angle[] = {0,40,80,0,0,0,0,0};
  assert_fuel_schedules(360U/3U, reqFuel * 100U, 3U, 1U, angle, INJ_PAIRED);
#endif
}

static void cylinder3_stroke2_semiseq_staged_port(void)
{
  configPage2.injLayout = INJ_SEMISEQUENTIAL;
  configPage10.stagingEnabled = true;
  configPage2.injType = INJ_TYPE_PORT;
  initialiseAll(); //Run the main initialise function
#if INJ_CHANNELS>=6
	const uint16_t angle[] = {0,60,120,0,60,120,0,0};
  assert_fuel_schedules(360U/2U, reqFuel * 100U, 3U, 3U, angle, INJ_PAIRED);
#else
	const uint16_t angle[] = {0,60,120,0,0,0,0,0};
  assert_fuel_schedules(360U/2U, reqFuel * 100U, 3U, 1U, angle, INJ_PAIRED);
#endif
}

static void run_3_cylinder_2stroke_tests(void)
{
  prepareForInitialiseAll(3U);
  configPage2.nCylinders = 3;
  configPage2.strokes = TWO_STROKE;
  configPage2.engineType = EVEN_FIRE;
  configPage2.injTiming = INJ_TIMING_ALTERNATING;;
  configPage2.reqFuel = reqFuel;
  configPage2.divider = 1;
 
  RUN_TEST_P(cylinder3_stroke2_seq_nostage);
  RUN_TEST_P(cylinder3_stroke2_semiseq_nostage_tb);
  RUN_TEST_P(cylinder3_stroke2_semiseq_nostage_port);
  RUN_TEST_P(cylinder3_stroke2_seq_staged);
  RUN_TEST_P(cylinder3_stroke2_semiseq_staged_tb);
  RUN_TEST_P(cylinder3_stroke2_semiseq_staged_port);
}

static void assert_4cylinder_4stroke_seq_nostage(void)
{
	const uint16_t angle[] = {0,180,360,540,0,0,0,0};
  assert_fuel_schedules(720U, reqFuel * 100U, 4U, 0U, angle, INJ_SEQUENTIAL);
}

static void cylinder4_stroke4_seq_nostage(void)
{
  configPage2.injLayout = INJ_SEQUENTIAL;
  configPage10.stagingEnabled = false;
  initialiseAll(); //Run the main initialise function
  assert_4cylinder_4stroke_seq_nostage();
}

static void cylinder4_stroke4_semiseq_nostage(void)
{
  configPage2.injLayout = INJ_SEMISEQUENTIAL;
  configPage10.stagingEnabled = false;
  initialiseAll(); //Run the main initialise function
	const uint16_t angle[] = {0,180,0,0,0,0,0,0};
  assert_fuel_schedules(360U, reqFuel * 50U, 2U, 0U, angle, INJ_SEMISEQUENTIAL);
  }


static void cylinder4_stroke4_seq_staged(void)
{
  configPage2.injLayout = INJ_SEQUENTIAL;
  configPage10.stagingEnabled = true;
  initialiseAll(); //Run the main initialise function
#if INJ_CHANNELS>=8
	const uint16_t angle[] = {0,180,360,540,0,180,360,540};
  assert_fuel_schedules(720U, reqFuel * 100U, 4U, 4U, angle, INJ_SEQUENTIAL);
#elif INJ_CHANNELS >= 5
	const uint16_t angle[] = {0,180,360,540,0,0,0,0};
  assert_fuel_schedules(720U, reqFuel * 100U, 4U, 1U, angle, INJ_SEQUENTIAL);
#else
  assert_4cylinder_4stroke_seq_nostage();
#endif
}

static void cylinder4_stroke4_semiseq_staged(void)  
{
  configPage2.injLayout = INJ_SEMISEQUENTIAL;
  configPage10.stagingEnabled = true;
  initialiseAll(); //Run the main initialise function
	// const uint16_t angle[] = {0,180,0,180,0,0,0,0};
  // assert_fuel_schedules(360U, reqFuel * 50U, 2U, 2U, angle, INJ_SEMISEQUENTIAL);
}

void run_4_cylinder_4stroke_tests(void)
{
  prepareForInitialiseAll(3U);
  configPage2.nCylinders = 4;
  configPage2.strokes = FOUR_STROKE;
  configPage2.engineType = EVEN_FIRE;
  configPage2.injTiming = INJ_TIMING_ALTERNATING;;
  configPage2.reqFuel = reqFuel;
  configPage2.divider = 2;

  RUN_TEST_P(cylinder4_stroke4_seq_nostage);
  RUN_TEST_P(cylinder4_stroke4_semiseq_nostage);
  RUN_TEST_P(cylinder4_stroke4_seq_staged);
  RUN_TEST_P(cylinder4_stroke4_semiseq_staged);  
}

static void cylinder4_stroke2_seq_nostage(void)
{
  configPage2.injLayout = INJ_SEQUENTIAL;
  configPage10.stagingEnabled = false;
  initialiseAll(); //Run the main initialise function
	const uint16_t angle[] = {0,90,0,0,0,0,0,0};
  assert_fuel_schedules(180U, reqFuel * 100U, 2U, 0U, angle, INJ_PAIRED);
  }

static void cylinder4_stroke2_semiseq_nostage(void)
{
  configPage2.injLayout = INJ_SEMISEQUENTIAL;
  configPage10.stagingEnabled = false;
  initialiseAll(); //Run the main initialise function
	const uint16_t angle[] = {0,90,0,0,0,0,0,0};
  assert_fuel_schedules(180U, reqFuel * 100U, 2U, 0U, angle, INJ_SEMISEQUENTIAL);
  }

static void cylinder4_stroke2_seq_staged(void)
{
  configPage2.injLayout = INJ_SEQUENTIAL;
  configPage10.stagingEnabled = true;
  initialiseAll(); //Run the main initialise function
#if INJ_CHANNELS>=8
	const uint16_t angle[] = {0,90,0,90,0,0,0,0};
  assert_fuel_schedules(180U, reqFuel * 100U, 2U, 2U, angle, INJ_PAIRED);
#elif INJ_CHANNELS >= 5
	const uint16_t angle[] = {0,180,0,0,0,0,0,0};
  assert_fuel_schedules(180U, reqFuel * 100U, 4U, 1U, angle, INJ_PAIRED);
#else
	const uint16_t angle[] = {0,90,0,90,0,0,0,0};
  assert_fuel_schedules(180U, reqFuel * 100U, 2U, 2U, angle, INJ_PAIRED);
#endif
  }

static void cylinder4_stroke2_semiseq_staged(void)
{
  configPage2.injLayout = INJ_SEMISEQUENTIAL;
  configPage10.stagingEnabled = true;
  initialiseAll(); //Run the main initialise function
	// const uint16_t angle[] = {0,90,0,90,0,0,0,0};
  // assert_fuel_schedules(180U, reqFuel * 100U, 2U, 2U,angle, INJ_SEMISEQUENTIAL);
}

void run_4_cylinder_2stroke_tests(void)
{
  prepareForInitialiseAll(3U);
  configPage2.nCylinders = 4;
  configPage2.strokes = TWO_STROKE;
  configPage2.engineType = EVEN_FIRE;
  configPage2.injTiming = INJ_TIMING_ALTERNATING;;
  configPage2.reqFuel = reqFuel;
  configPage2.divider = 2;

  RUN_TEST_P(cylinder4_stroke2_seq_nostage);
  RUN_TEST_P(cylinder4_stroke2_semiseq_nostage);
  RUN_TEST_P(cylinder4_stroke2_seq_staged);
  RUN_TEST_P(cylinder4_stroke2_semiseq_staged);  
}

static void cylinder5_stroke4_seq_nostage(void)
{
  configPage2.injLayout = INJ_SEQUENTIAL;
  configPage10.stagingEnabled = false;
  initialiseAll(); //Run the main initialise function
#if INJ_CHANNELS >= 5
	const uint16_t angle[] = {0,144,288,432,576,0,0,0};
  assert_fuel_schedules(720U, reqFuel * 100U, 5U, 0U, angle, INJ_SEQUENTIAL);
#else
  const uint16_t angle[] = {0,180,360,540,0,0,0,0};
  assert_fuel_schedules(720U, reqFuel * 50U, 4U, 0U, angle, INJ_PAIRED);
#endif
}


static void cylinder5_stroke4_semiseq_nostage(void)
{
  configPage2.injLayout = INJ_SEMISEQUENTIAL;
  configPage10.stagingEnabled = false;
  initialiseAll(); //Run the main initialise function
#if INJ_CHANNELS >= 5
	const uint16_t angle[] = {0,144,288,432,576,0,0,0};
  assert_fuel_schedules(720U, reqFuel * 50U, 5U, 0U, angle, INJ_SEMISEQUENTIAL);
#else
  const uint16_t angle[] = {0,180,360,540,0,0,0,0};
  assert_fuel_schedules(720U, reqFuel * 50U, 4U, 0U, angle, INJ_SEMISEQUENTIAL);
#endif
}

static void cylinder5_stroke4_seq_staged(void)
{
  configPage2.injLayout = INJ_SEQUENTIAL;
  configPage10.stagingEnabled = true;
  initialiseAll(); //Run the main initialise function
#if INJ_CHANNELS >= 6
	const uint16_t angle[] = {0,144,288,432,576,0,0,0};
  assert_fuel_schedules(720U, reqFuel * 100U, 5U, 1U, angle, INJ_SEQUENTIAL);
#else
  const uint16_t angle[] = {0,180,360,540,0,0,0,0};
  assert_fuel_schedules(720U, reqFuel * 50U, 4U, 0U, angle, INJ_PAIRED);
#endif
}

static void cylinder5_stroke4_semiseq_staged(void) 
{
  configPage2.injLayout = INJ_SEMISEQUENTIAL;
  configPage10.stagingEnabled = true;
  initialiseAll(); //Run the main initialise function
#if INJ_CHANNELS >= 6
	const uint16_t angle[] = {0,72,144,216,288,0,0,0};
  assert_fuel_schedules(720U, reqFuel * 50U, 5U, 1U, angle, INJ_SEMISEQUENTIAL);
#else
  const uint16_t angle[] = {0,180,360,540,0,0,0,0};
  assert_fuel_schedules(720U, reqFuel * 50U, 4U, 0U, angle, INJ_SEMISEQUENTIAL);
#endif
}

void run_5_cylinder_4stroke_tests(void)
{
  prepareForInitialiseAll(3U);
  configPage2.nCylinders = 5;
  configPage2.strokes = FOUR_STROKE;
  configPage2.engineType = EVEN_FIRE;
  configPage2.injTiming = INJ_TIMING_ALTERNATING;;
  configPage2.reqFuel = reqFuel;
  configPage2.divider = 5;

  RUN_TEST_P(cylinder5_stroke4_seq_nostage);
  RUN_TEST_P(cylinder5_stroke4_semiseq_nostage);
  RUN_TEST_P(cylinder5_stroke4_seq_staged);
  RUN_TEST_P(cylinder5_stroke4_semiseq_staged); 
}

static void cylinder6_stroke4_seq_nostage(void)
{
  configPage2.injLayout = INJ_SEQUENTIAL;
  configPage10.stagingEnabled = false;
  initialiseAll(); //Run the main initialise function
#if INJ_CHANNELS >= 6
	const uint16_t angle[] = {0,120,240,360,480,600,0,0};
  assert_fuel_schedules(720U, reqFuel * 100U, 6U, 0U, angle, INJ_SEQUENTIAL);
#else
  const uint16_t angle[] = {0,240,480,0,0,0,0,0};
  assert_fuel_schedules(720U, reqFuel * 50U, 3U, 0U, angle, INJ_PAIRED);
#endif
}

static void cylinder6_stroke4_semiseq_nostage(void)
{
  configPage2.injLayout = INJ_SEMISEQUENTIAL;
  configPage10.stagingEnabled = false;
  initialiseAll(); //Run the main initialise function
	const uint16_t angle[] = {0,240,480,0,0,0,0,0};
  assert_fuel_schedules(720U, reqFuel * 50U, 3U, 0U, angle, INJ_SEMISEQUENTIAL);
}

static void cylinder6_stroke4_seq_staged(void)
{
  configPage2.injLayout = INJ_SEQUENTIAL;
  configPage10.stagingEnabled = true;
  initialiseAll(); //Run the main initialise function
#if INJ_CHANNELS >= 8
	const uint16_t angle[] = {0,120,240,360,480,600,0,0};
  assert_fuel_schedules(720U, reqFuel * 100U, 6U, 0U, angle, INJ_SEQUENTIAL);
#else
  const uint16_t angle[] = {0,240,480,0,0,0,0,0};
  assert_fuel_schedules(720U, reqFuel * 50U, 3U, 0U, angle, INJ_PAIRED);
#endif
}

static void cylinder6_stroke4_semiseq_staged(void)
{
  configPage2.injLayout = INJ_SEMISEQUENTIAL;
  configPage10.stagingEnabled = true;
  initialiseAll(); //Run the main initialise function
#if INJ_CHANNELS >= 6
	const uint16_t angle[] = {0,240,480,0,240,480,0,0};
  assert_fuel_schedules(720U, reqFuel * 50U, 3U, 3U, angle, INJ_SEMISEQUENTIAL);
#else
  const uint16_t angle[] = {0,240,480,0,0,0,0,0};
  assert_fuel_schedules(720U, reqFuel * 50U, 3U, 0U, angle, INJ_SEMISEQUENTIAL);
#endif
}

void run_6_cylinder_4stroke_tests(void)
{
  prepareForInitialiseAll(3U);
  configPage2.nCylinders = 6;
  configPage2.strokes = FOUR_STROKE;
  configPage2.engineType = EVEN_FIRE;
  configPage2.injTiming = INJ_TIMING_ALTERNATING;;
  configPage2.reqFuel = reqFuel;
  configPage2.divider = 6;

  RUN_TEST_P(cylinder6_stroke4_seq_nostage);
  RUN_TEST_P(cylinder6_stroke4_semiseq_nostage);
  RUN_TEST_P(cylinder6_stroke4_seq_staged);
  RUN_TEST_P(cylinder6_stroke4_semiseq_staged); 
}

static void cylinder8_stroke4_seq_nostage(void)
{
  configPage2.injLayout = INJ_SEQUENTIAL;
  configPage10.stagingEnabled = false;
  initialiseAll(); //Run the main initialise function
#if INJ_CHANNELS >= 8
  const uint16_t angle[] = {0,90,180,270,360,450,540,630};
  assert_fuel_schedules(720U, reqFuel * 100U, 8U, 0U, angle, INJ_SEQUENTIAL);
#else
  const uint16_t angle[] = {0,180,360,540,0,0,0,0};
  assert_fuel_schedules(720U, reqFuel * 50U, 4U, 0U, angle, INJ_PAIRED);
#endif
}

static void cylinder8_stroke4_paired_nostage(void)
{
  configPage2.injLayout = INJ_PAIRED;
  configPage10.stagingEnabled = false;
  initialiseAll(); //Run the main initialise function
  const uint16_t angle[] = {0,180,360,540,0,0,0,0};
  assert_fuel_schedules(720U, reqFuel * 50U, 4U, 0U, angle, INJ_PAIRED);
}

void run_8_cylinder_4stroke_tests(void)
{
  prepareForInitialiseAll(3U);
  configPage2.nCylinders = 8;
  configPage2.strokes = FOUR_STROKE;
  configPage2.engineType = EVEN_FIRE;
  configPage2.injTiming = INJ_TIMING_ALTERNATING;;
  configPage2.reqFuel = reqFuel;
  configPage2.divider = 8;

  // Staging not supported on 8 cylinders

  RUN_TEST_P(cylinder8_stroke4_seq_nostage);
  RUN_TEST_P(cylinder8_stroke4_paired_nostage);
}

static constexpr uint16_t zeroAngles[] = {0,0,0,0,0,0,0,0};

static void cylinder_1_NoinjTiming_paired(void) {
  configPage2.injLayout = INJ_PAIRED;
  configPage2.nCylinders = 1;
  configPage2.divider = 1;

  initialiseAll(); //Run the main initialise function

  assert_fuel_schedules(720U, reqFuel * 50U, 1, 0U, zeroAngles, INJ_PAIRED);  
}

static void cylinder_2_NoinjTiming_paired(void) {
  configPage2.injLayout = INJ_PAIRED;
  configPage2.nCylinders = 2;
  configPage2.divider = 2;

  initialiseAll(); //Run the main initialise function

  assert_fuel_schedules(720U, reqFuel * 50U, 2, 0U, zeroAngles, INJ_PAIRED);   
}

static void cylinder_3_NoinjTiming_paired(void) {
  configPage2.injLayout = INJ_PAIRED;
  configPage2.nCylinders = 3;
  configPage2.divider = 3;
  configPage2.injType = INJ_TYPE_PORT;

  initialiseAll(); //Run the main initialise function

  assert_fuel_schedules(360U, reqFuel * 50U, 3, 0U, zeroAngles, INJ_PAIRED);      
}

static void cylinder_4_NoinjTiming_paired(void) {
  configPage2.injLayout = INJ_PAIRED;
  configPage2.nCylinders = 4;
  configPage2.divider = 4;

  initialiseAll(); //Run the main initialise function

  assert_fuel_schedules(720U, reqFuel * 50U, 2, 0U, zeroAngles, INJ_PAIRED);
}

static void cylinder_5_NoinjTiming_paired(void) {
  configPage2.injLayout = INJ_PAIRED;
  configPage2.nCylinders = 5;
  configPage2.divider = 5;

  initialiseAll(); //Run the main initialise function

#if INJ_CHANNELS>=5  
  assert_fuel_schedules(720U, reqFuel * 50U, 5, 0U, zeroAngles, INJ_PAIRED);   
#else
  assert_fuel_schedules(720U, reqFuel * 50U, 4, 0U, zeroAngles, INJ_PAIRED);   
#endif
}

static void cylinder_6_NoinjTiming_paired(void) {
  configPage2.injLayout = INJ_PAIRED;
  configPage2.nCylinders = 6;
  configPage2.divider = 6;

  initialiseAll(); //Run the main initialise function

  assert_fuel_schedules(720U, reqFuel * 50U, 3, 0U, zeroAngles, INJ_PAIRED);   
}

static void cylinder_8_NoinjTiming_paired(void) {
  configPage2.injLayout = INJ_PAIRED;
  configPage2.nCylinders = 8;
  configPage2.divider = 8;

  initialiseAll(); //Run the main initialise function

  assert_fuel_schedules(720U, reqFuel * 50U, 4, 0U, zeroAngles, INJ_PAIRED);   
}

static void run_no_inj_timing_tests(void)
{
  prepareForInitialiseAll(3U);
  configPage2.strokes = FOUR_STROKE;
  configPage2.engineType = EVEN_FIRE;
  configPage2.injTiming = false;
  configPage2.reqFuel = reqFuel;
  configPage10.stagingEnabled = false;

  RUN_TEST_P(cylinder_1_NoinjTiming_paired);
  RUN_TEST_P(cylinder_2_NoinjTiming_paired);
  RUN_TEST_P(cylinder_3_NoinjTiming_paired);
  RUN_TEST_P(cylinder_4_NoinjTiming_paired);
  RUN_TEST_P(cylinder_5_NoinjTiming_paired);
  RUN_TEST_P(cylinder_6_NoinjTiming_paired);
  RUN_TEST_P(cylinder_8_NoinjTiming_paired);
}

static void cylinder_2_oddfire(void)
{
  configPage2.injLayout = INJ_PAIRED;
  configPage2.nCylinders = 2;
  configPage2.divider = 2;

  initialiseAll(); //Run the main initialise function

	const uint16_t angle[] = {0,13,0,0,0,0,0,0};
  assert_fuel_schedules(720U, reqFuel * 50U, 2U, 0U, angle, INJ_PAIRED);
}

static void run_oddfire_tests()
{
  prepareForInitialiseAll(3U);
  configPage2.strokes = FOUR_STROKE;
  configPage2.injTiming = INJ_TIMING_ALTERNATING;;
  configPage2.reqFuel = reqFuel;
  configPage10.stagingEnabled = false;
  configPage2.oddfire[0] = 13;
  configPage2.oddfire[1] = 111;
  configPage2.oddfire[2] = 217;
  
  // Oddfire only affects 2 cylinder configurations
  configPage2.nCylinders = 1;
  configPage2.divider = 1;
  configPage2.engineType = ODD_FIRE;
  RUN_TEST_P(cylinder1_stroke4_seq_nostage);

  configPage2.engineType = ODD_FIRE;
  RUN_TEST_P(cylinder_2_oddfire);

  configPage2.nCylinders = 3;
  configPage2.divider = 1;
  configPage2.engineType = ODD_FIRE;
  RUN_TEST_P(cylinder3_stroke4_seq_nostage);
  configPage2.nCylinders = 4;
  configPage2.divider = 2;
  configPage2.engineType = ODD_FIRE;
  RUN_TEST_P(cylinder4_stroke4_seq_nostage);
  configPage2.nCylinders = 5;
  configPage2.divider = 5;
  configPage2.engineType = ODD_FIRE;
  RUN_TEST_P(cylinder5_stroke4_seq_nostage);
  configPage2.nCylinders = 6;
  configPage2.divider = 6;
  configPage2.engineType = ODD_FIRE;
  RUN_TEST_P(cylinder6_stroke4_seq_nostage);
  configPage2.nCylinders = 8;
  configPage2.divider = 8;
  configPage2.engineType = ODD_FIRE;
  RUN_TEST_P(cylinder8_stroke4_seq_nostage);
}

static void test_partial_sync(void)
{
  prepareForInitialiseAll(3U);
  configPage2.nCylinders = 4;
  configPage2.strokes = FOUR_STROKE;
  configPage2.engineType = EVEN_FIRE;
  configPage2.injTiming = INJ_TIMING_ALTERNATING;;
  configPage2.reqFuel = reqFuel;
  configPage2.injLayout = INJ_SEQUENTIAL;
  configPage10.stagingEnabled = false;
  initialiseAll(); //Run the main initialise function

  // Confirm initial state
  assert_4cylinder_4stroke_seq_nostage();

  currentStatus.hasSync = false;
  BIT_SET(currentStatus.status3, BIT_STATUS3_HALFSYNC);
  CRANK_ANGLE_MAX_INJ = 720U;
  matchInjectionModeToSyncStatus();
  {
	  const uint16_t angle[] = {0,180,360,540,0,0,0,0};
    assert_fuel_schedules(360U, reqFuel * 50U, 2U, 0U, angle, INJ_SEQUENTIAL);    
  }

  currentStatus.hasSync = true;
  BIT_CLEAR(currentStatus.status3, BIT_STATUS3_HALFSYNC);
  CRANK_ANGLE_MAX_INJ = 360U;
  matchInjectionModeToSyncStatus();
  assert_4cylinder_4stroke_seq_nostage();
}


void testFuelScheduleInit()
{
  SET_UNITY_FILENAME() {

  run_1_cylinder_4stroke_tests();
  run_1_cylinder_2stroke_tests();
  run_2_cylinder_4stroke_tests();
  run_2_cylinder_2stroke_tests();
  run_3_cylinder_4stroke_tests();
  run_3_cylinder_2stroke_tests();
  run_4_cylinder_4stroke_tests();
  run_4_cylinder_2stroke_tests();
  run_5_cylinder_4stroke_tests();
  run_6_cylinder_4stroke_tests();
  run_8_cylinder_4stroke_tests();

  run_no_inj_timing_tests();

  run_oddfire_tests();

  RUN_TEST_P(test_partial_sync);
  }
}