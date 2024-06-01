#include <unity.h>
#include "globals.h"
#include "corrections.h"
#include "test_corrections.h"
#include "../test_utils.h"
#include "init.h"
#include "sensors.h"
#include "speeduino.h"
#include "scale_translate.h"
#include "../test_utils.h"

extern void construct2dTables(void);
extern void construct2dTable(table2D &table, uint8_t valueSize, uint8_t axisSize, uint8_t length, void *values, void *bins);
extern void construct2dTable(table2D &table, uint8_t length, uint8_t *values, uint8_t *bins);

extern uint8_t correctionWUE(statuses &current, const table2D &lookUpTable);

template <uint8_t length>
struct test_2dtable_t {
  table2D lookupTable;
  uint8_t bins[length];
  uint8_t values[length];  

  test_2dtable_t() {
    construct2dTable(lookupTable, length, values, bins);
  }
};

struct wue_test_data_t : public test_2dtable_t<10> {
  statuses current;
};

static void setup_wue_table(wue_test_data_t &testData) {
  LOOP_TIMER = 0;
  BIT_SET(LOOP_TIMER, CLT_READ_TIMER_BIT) ;

  //Set some fake values in the table axis. Target value will fall between points 6 and 7
  TEST_DATA_P uint8_t bins[_countof(testData.bins)] = { 
    0, 0, 0, 0, 0, 0,
    toStorageTemperature(70),
    toStorageTemperature(90),
    toStorageTemperature(100),
    toStorageTemperature(120)
  };
  TEST_DATA_P uint8_t values[_countof(testData.values)] = { 0, 0, 0, 0, 0, 0, 120, 130, 140, 150 };
  populate_2dtable_P(&testData.lookupTable, values, bins);
}

static void test_corrections_WUE_active(void)
{
  wue_test_data_t testData;
  setup_wue_table(testData);

  //Check for WUE being active
  testData.current.coolant = 0;

  correctionWUE(testData.current, testData.lookupTable);
  TEST_ASSERT_BIT_HIGH(BIT_ENGINE_WARMUP, testData.current.engine);
}

static void test_corrections_WUE_inactive(void)
{
  wue_test_data_t testData;
  setup_wue_table(testData);

  //Check for WUE being inactive due to the temp being too high
  testData.current.coolant = 200;
  correctionWUE(testData.current, testData.lookupTable);
  TEST_ASSERT_BIT_LOW(BIT_ENGINE_WARMUP, testData.current.engine);
}

static void test_corrections_WUE_inactive_value(void)
{
  wue_test_data_t testData;
  setup_wue_table(testData);

  //Check for WUE being set to the final row of the WUE curve if the coolant is above the max WUE temp
  testData.current.coolant = 200;
  testData.bins[9] = 100;
  testData.values[9] = 123; //Use a value other than 100 here to ensure we are using the non-default value
  
  TEST_ASSERT_EQUAL(123, correctionWUE(testData.current, testData.lookupTable) );
}

static void test_corrections_WUE_active_value(void)
{
  wue_test_data_t testData;
  setup_wue_table(testData);

  //Check for WUE being made active and returning a correct interpolated value
  testData.current.coolant = 80;
  
  //Value should be midway between 120 and 130 = 125
  TEST_ASSERT_EQUAL(125, correctionWUE(testData.current, testData.lookupTable) );
}

static void test_corrections_WUE(void)
{
  RUN_TEST_P(test_corrections_WUE_active);
  RUN_TEST_P(test_corrections_WUE_inactive);
  RUN_TEST_P(test_corrections_WUE_active_value);
  RUN_TEST_P(test_corrections_WUE_inactive_value);
}

extern uint16_t correctionCranking(const statuses &current, const table2D &lookupTable, const config10 &page10);

struct cranking_testdata_t : public test_2dtable_t<4> {
  statuses current;
  config10 page10;
};

static void setup_correctionCranking(cranking_testdata_t &testData) {
  LOOP_TIMER = 0;
  BIT_SET(LOOP_TIMER, BIT_TIMER_10HZ);
  constexpr int16_t COOLANT_INITIAL = toWorkingTemperature(150); 
  testData.current.coolant = COOLANT_INITIAL;

  TEST_DATA_P uint8_t values[_countof(testData.values)] = { 120U / 5U, 130U / 5U, 140U / 5U, 150U / 5U };
  TEST_DATA_P uint8_t bins[_countof(testData.bins)] = { 
    (uint8_t)(toStorageTemperature(COOLANT_INITIAL) - 10U),
    (uint8_t)(toStorageTemperature(COOLANT_INITIAL) + 10U),
    (uint8_t)(toStorageTemperature(COOLANT_INITIAL) + 20U),
    (uint8_t)(toStorageTemperature(COOLANT_INITIAL) + 30U)
  };
  populate_2dtable_P(&testData.lookupTable, values, bins);
}

static void test_corrections_cranking_inactive(void) {
  cranking_testdata_t testData;
  setup_correctionCranking(testData);
  
  BIT_CLEAR(testData.current.engine, BIT_ENGINE_CRANK);
  BIT_CLEAR(testData.current.engine, BIT_ENGINE_ASE);
  testData.page10.crankingEnrichTaper = 0U;

  TEST_ASSERT_EQUAL(100, correctionCranking(testData.current, testData.lookupTable, testData.page10) );
} 

static void test_corrections_cranking_cranking(void) {
  cranking_testdata_t testData;
  setup_correctionCranking(testData);
  
  BIT_SET(testData.current.engine, BIT_ENGINE_CRANK);
  BIT_CLEAR(testData.current.engine, BIT_ENGINE_ASE);
  testData.page10.crankingEnrichTaper = 0U;

  // Should be half way between the 2 table values.
  TEST_ASSERT_EQUAL(125, correctionCranking(testData.current, testData.lookupTable, testData.page10) );
} 

static void test_corrections_cranking_taper_noase(void) {
  cranking_testdata_t testData;
  setup_correctionCranking(testData);
  BIT_CLEAR(testData.current.engine, BIT_ENGINE_ASE);
  
  testData.page10.crankingEnrichTaper = 100U;
  testData.current.ASEValue = 100U;

  // Reset taper
  BIT_SET(testData.current.engine, BIT_ENGINE_CRANK);
  (void)correctionCranking(testData.current, testData.lookupTable, testData.page10);

  // Advance taper to halfway
  BIT_CLEAR(testData.current.engine, BIT_ENGINE_CRANK);
  for (uint8_t index=0; index<testData.page10.crankingEnrichTaper/2U; ++index) {
    (void)correctionCranking(testData.current, testData.lookupTable, testData.page10);
  }

  // Should be half way between the interpolated table value and 100%.
  TEST_ASSERT_INT_WITHIN(1, 113U, correctionCranking(testData.current, testData.lookupTable, testData.page10) );
  
  // Final taper step
  for (uint8_t index=testData.page10.crankingEnrichTaper/2U; index<testData.page10.crankingEnrichTaper-2U; ++index) {
    (void)correctionCranking(testData.current, testData.lookupTable, testData.page10);
  }
  TEST_ASSERT_INT_WITHIN(1, 101U, correctionCranking(testData.current, testData.lookupTable, testData.page10) );

  // Taper finished
  TEST_ASSERT_EQUAL(100U, correctionCranking(testData.current, testData.lookupTable, testData.page10));
  TEST_ASSERT_EQUAL(100U, correctionCranking(testData.current, testData.lookupTable, testData.page10));
} 


static void test_corrections_cranking_taper_withase(void) {
  cranking_testdata_t testData;
  setup_correctionCranking(testData);
  testData.page10.crankingEnrichTaper = 100U;
  
  BIT_SET(testData.current.engine, BIT_ENGINE_ASE);
  testData.current.ASEValue = 50U;

  // Reset taper
  BIT_SET(testData.current.engine, BIT_ENGINE_CRANK);
  (void)correctionCranking(testData.current, testData.lookupTable, testData.page10);

  // Advance taper to halfway
  BIT_CLEAR(testData.current.engine, BIT_ENGINE_CRANK);
  for (uint8_t index=0; index<testData.page10.crankingEnrichTaper/2U; ++index) {
    (void)correctionCranking(testData.current, testData.lookupTable, testData.page10);
  }

  // Should be half way between the interpolated table value and 100%.
  TEST_ASSERT_INT_WITHIN(1, 175U, correctionCranking(testData.current, testData.lookupTable, testData.page10) );
  
  // Final taper step
  for (uint8_t index=testData.page10.crankingEnrichTaper/2U; index<testData.page10.crankingEnrichTaper-2U; ++index) {
    (void)correctionCranking(testData.current, testData.lookupTable, testData.page10);
  }
  TEST_ASSERT_INT_WITHIN(1, 102U, correctionCranking(testData.current, testData.lookupTable, testData.page10) );

  // Taper finished
  TEST_ASSERT_EQUAL(100U, correctionCranking(testData.current, testData.lookupTable, testData.page10));
  TEST_ASSERT_EQUAL(100U, correctionCranking(testData.current, testData.lookupTable, testData.page10));
} 

static void test_corrections_cranking(void)
{
  RUN_TEST_P(test_corrections_cranking_inactive);
  RUN_TEST_P(test_corrections_cranking_cranking);
  RUN_TEST_P(test_corrections_cranking_taper_noase);
  RUN_TEST_P(test_corrections_cranking_taper_withase);
}

extern uint8_t correctionASE(statuses &current, const table2D &durationTable, const table2D &amountTable, const config2 &page2);

static void test_corrections_ASE_inactive_cranking(void)
{
  statuses current;
  BIT_SET(current.engine, BIT_ENGINE_CRANK);

  // Taper finished
  TEST_ASSERT_EQUAL(100U, correctionASE(current, table2D(), table2D(), config2()));
  TEST_ASSERT_BIT_LOW(BIT_ENGINE_ASE, current.engine);
}

struct ase_testdata_t {
  statuses current;
  table2D durationTable;
  uint8_t durationBins[4];
  uint8_t durationValues[4];  
  table2D amountTable;
  uint8_t amountBins[4];
  uint8_t amountValues[4];  
  config2 page2;
};

static inline void setup_correctionASE(ase_testdata_t &testData) {
  BIT_CLEAR(testData.current.engine, BIT_ENGINE_CRANK);
  LOOP_TIMER = 0;
  BIT_SET(LOOP_TIMER, BIT_TIMER_10HZ) ;
  constexpr int16_t COOLANT_INITIAL = toWorkingTemperature(150); 
  testData.current.coolant = COOLANT_INITIAL;
  testData.current.runSecs = 3;

  {
    construct2dTable(testData.durationTable, _countof(testData.durationBins), testData.durationValues, testData.durationBins);
    TEST_DATA_P uint8_t values[] = { 10, 8, 6, 4 };
    TEST_DATA_P uint8_t bins[] = { 
      toStorageTemperature(COOLANT_INITIAL) - 10U,
      toStorageTemperature(COOLANT_INITIAL) + 10U,
      toStorageTemperature(COOLANT_INITIAL) + 20U,
      toStorageTemperature(COOLANT_INITIAL) + 30U
    };
    populate_2dtable_P(&testData.durationTable, values, bins);
  }

  {
    construct2dTable(testData.amountTable, _countof(testData.amountBins), testData.amountValues, testData.amountBins);
    TEST_DATA_P uint8_t values[] = { 20, 30, 40, 50 };
    TEST_DATA_P uint8_t bins[] = { 
      toStorageTemperature(COOLANT_INITIAL) - 10U,
      toStorageTemperature(COOLANT_INITIAL) + 10U,
      toStorageTemperature(COOLANT_INITIAL) + 20U,
      toStorageTemperature(COOLANT_INITIAL) + 30U
    };
    populate_2dtable_P(&testData.amountTable, values, bins);
  } 
}

static void test_corrections_ASE_initial(void)
{
  ase_testdata_t testData;
  setup_correctionASE(testData);

  // Should be half way between the 2 table values.
  TEST_ASSERT_EQUAL(125, correctionASE(testData.current, testData.durationTable, testData.amountTable, testData.page2));
  TEST_ASSERT_BIT_HIGH(BIT_ENGINE_ASE, testData.current.engine);
}

static void test_corrections_ASE_taper(void) {
  ase_testdata_t testData;
  setup_correctionASE(testData);
  // Switch to ASE taper
  testData.page2.aseTaperTime = 12U;
  testData.current.runSecs = 9;

  // Advance taper to halfway
  BIT_CLEAR(testData.current.engine, BIT_ENGINE_CRANK);
  for (uint8_t index=0; index<testData.page2.aseTaperTime/2U; ++index) {
    (void)correctionASE(testData.current, testData.durationTable, testData.amountTable, testData.page2);
  }

  // Should be half way between the interpolated table value and 100%.
  TEST_ASSERT_INT_WITHIN(1, 113, correctionASE(testData.current, testData.durationTable, testData.amountTable, testData.page2));
  TEST_ASSERT_BIT_HIGH(BIT_ENGINE_ASE, testData.current.engine);
  
  // Final taper step
  for (uint8_t index=testData.page2.aseTaperTime/2U; index<testData.page2.aseTaperTime-2U; ++index) {
    (void)correctionASE(testData.current, testData.durationTable, testData.amountTable, testData.page2);
  }
  TEST_ASSERT_INT_WITHIN(1, 103U, correctionASE(testData.current, testData.durationTable, testData.amountTable, testData.page2) );

  // Taper finished
  TEST_ASSERT_EQUAL(100U, correctionASE(testData.current, testData.durationTable, testData.amountTable, testData.page2));  
  TEST_ASSERT_EQUAL(100U, correctionASE(testData.current, testData.durationTable, testData.amountTable, testData.page2));  
}

static void test_corrections_ASE(void)
{
  RUN_TEST_P(test_corrections_ASE_inactive_cranking);
  RUN_TEST_P(test_corrections_ASE_initial);
  RUN_TEST_P(test_corrections_ASE_taper);
}

extern uint8_t correctionFloodClear(const statuses &current, const config4 &page4);

static void test_corrections_floodclear_no_crank_inactive(void) {
  statuses current;
  config4  page4;
  BIT_CLEAR(current.engine, BIT_ENGINE_CRANK);
  page4.floodClear = 90;
  current.TPS = page4.floodClear + 10;

  TEST_ASSERT_EQUAL(100U, correctionFloodClear(current, page4) );
}

static void test_corrections_floodclear_crank_below_threshold_inactive(void) {
  statuses current;
  config4  page4;
  BIT_SET(current.engine, BIT_ENGINE_CRANK);
  page4.floodClear = 90;
  current.TPS = page4.floodClear - 10;

  TEST_ASSERT_EQUAL(100U, correctionFloodClear(current, page4) );
}

static void test_corrections_floodclear_crank_above_threshold_active(void) {
  statuses current;
  config4  page4;
  BIT_SET(current.engine, BIT_ENGINE_CRANK);
  page4.floodClear = 90;
  current.TPS = page4.floodClear + 10;

  TEST_ASSERT_EQUAL(0U, correctionFloodClear(current, page4) );
}

static void test_corrections_floodclear(void)
{
  RUN_TEST_P(test_corrections_floodclear_no_crank_inactive);
  RUN_TEST_P(test_corrections_floodclear_crank_below_threshold_inactive);
  RUN_TEST_P(test_corrections_floodclear_crank_above_threshold_active);
}

uint8_t correctionAFRClosedLoop(void);
extern uint16_t AFRnextCycle;

static void setup_valid_ego_cycle(void) {
  AFRnextCycle = 4196;
  ignitionCount = AFRnextCycle + (configPage6.egoCount/2U); 
}

static void setup_ego_simple(void) {
  construct2dTables();
  initialiseCorrections();

  configPage6.egoType = EGO_TYPE_NARROW;
  configPage6.egoAlgorithm = EGO_ALGORITHM_SIMPLE;
  configPage6.egoLimit = 30U;

  configPage6.ego_sdelay = 10;
  currentStatus.runSecs = configPage6.ego_sdelay + 2U;

  configPage6.egoTemp = 150U;
  currentStatus.coolant = toWorkingTemperature(configPage6.egoTemp) + 1U; 

  configPage6.egoRPM = 30U;
  currentStatus.RPM = configPage6.egoRPM*100U + 1U;

  configPage6.egoTPSMax = 33;
  currentStatus.TPS = configPage6.egoTPSMax - 1U;

  configPage6.ego_max = 150U;
  configPage6.ego_min = 50U;
  currentStatus.O2 = configPage6.ego_min + ((configPage6.ego_max-configPage6.ego_min)/2U);

  configPage9.egoMAPMax = 100U;
  configPage9.egoMAPMin = 50U;
  currentStatus.MAP = (configPage9.egoMAPMin + ((configPage9.egoMAPMax-configPage9.egoMAPMin)/2U))*2U;
  
  currentStatus.afrTarget = currentStatus.O2;
  currentStatus.egoCorrection = 100U;
  
  BIT_CLEAR(currentStatus.status1, BIT_STATUS1_DFCO);

  configPage6.egoCount = 100U;
  setup_valid_ego_cycle();
}

static void test_corrections_closedloop_off_nosensor(void) {
  setup_ego_simple();
  currentStatus.O2 = currentStatus.afrTarget + 1U;
  configPage6.egoType = EGO_TYPE_OFF;
  TEST_ASSERT_EQUAL(100U, correctionAFRClosedLoop());
}

static void test_corrections_closedloop_off_dfco(void) {
  setup_ego_simple();
  currentStatus.O2 = currentStatus.afrTarget + 1U;
  BIT_SET(currentStatus.status1, BIT_STATUS1_DFCO);
  TEST_ASSERT_EQUAL(100U, correctionAFRClosedLoop());
}

static void test_corrections_closedloop_off_no_algorithm(void) {
  setup_ego_simple();
  currentStatus.O2 = currentStatus.afrTarget + 1U;
  configPage6.egoAlgorithm = EGO_ALGORITHM_NONE;
  TEST_ASSERT_EQUAL(100U, correctionAFRClosedLoop());

  setup_ego_simple();
  currentStatus.O2 = currentStatus.afrTarget + 1U;
  configPage6.egoAlgorithm = EGO_ALGORITHM_INVALID1;
  TEST_ASSERT_EQUAL(100U, correctionAFRClosedLoop());
}

static void test_corrections_closedloop_off_invalidconditions_coolant(void) {
  setup_ego_simple();
  currentStatus.O2 = currentStatus.afrTarget + 1U;
  currentStatus.coolant = toWorkingTemperature(configPage6.egoTemp) - 1U; 
  TEST_ASSERT_EQUAL(100U, correctionAFRClosedLoop());
}

static void test_corrections_closedloop_off_invalidconditions_rpm(void) {
  setup_ego_simple();
  currentStatus.O2 = currentStatus.afrTarget + 1U;
  currentStatus.RPM = (configPage6.egoRPM*100U) - 1U;
  TEST_ASSERT_EQUAL(100U, correctionAFRClosedLoop());
}

static void test_corrections_closedloop_off_invalidconditions_tps(void) {
  setup_ego_simple();
  currentStatus.O2 = currentStatus.afrTarget + 1U;
  currentStatus.TPS = configPage6.egoTPSMax + 1U;
  TEST_ASSERT_EQUAL(100U, correctionAFRClosedLoop());
}

static void test_corrections_closedloop_off_invalidconditions_o2(void) {
  setup_ego_simple();
  currentStatus.O2 = configPage6.ego_min - 1U;
  TEST_ASSERT_EQUAL(100U, correctionAFRClosedLoop());

  setup_ego_simple();
  currentStatus.O2 = configPage6.ego_max + 1U;
  TEST_ASSERT_EQUAL(100U, correctionAFRClosedLoop());
}

static void test_corrections_closedloop_off_invalidconditions_map(void) {
  setup_ego_simple();
  currentStatus.O2 = currentStatus.afrTarget + 1U;
  currentStatus.MAP = (configPage9.egoMAPMin*2U) - 1U;
  TEST_ASSERT_EQUAL(100U, correctionAFRClosedLoop());

  setup_ego_simple();
  currentStatus.O2 = currentStatus.afrTarget + 1U;
  currentStatus.MAP = (configPage9.egoMAPMax*2U) + 1U;
  TEST_ASSERT_EQUAL(100U, correctionAFRClosedLoop());
}

static void test_corrections_closedloop_outsidecycle(void) {
  setup_ego_simple();
  currentStatus.O2 = currentStatus.afrTarget + 1U;
  currentStatus.egoCorrection = 173U;
  ignitionCount = AFRnextCycle - (configPage6.egoCount/2U); 
  TEST_ASSERT_EQUAL(currentStatus.egoCorrection, correctionAFRClosedLoop());
}

static void test_corrections_closedloop_cycle_countrollover(void) {
  setup_ego_simple();
  currentStatus.O2 = currentStatus.afrTarget + 1U;
  currentStatus.egoCorrection = 101U;
  ignitionCount = AFRnextCycle - (configPage6.egoCount*2U); 
  TEST_ASSERT_EQUAL(currentStatus.egoCorrection+1U, correctionAFRClosedLoop());
}

static void test_corrections_closedloop_simple_nocorrection(void) {
  setup_ego_simple();
  currentStatus.egoCorrection = 101U;
  currentStatus.O2 = currentStatus.afrTarget;
  TEST_ASSERT_EQUAL(currentStatus.egoCorrection, correctionAFRClosedLoop());
}

static void test_corrections_closedloop_simple_lean(void) {
  setup_ego_simple();
  currentStatus.O2 = currentStatus.afrTarget + 1U;
  TEST_ASSERT_EQUAL(currentStatus.egoCorrection+1U, correctionAFRClosedLoop());
}

static void test_corrections_closedloop_simple_lean_maxcorrection(void) {
  setup_ego_simple();

  currentStatus.O2 = configPage6.ego_max-1U;

  for (uint8_t index=0; index<configPage6.egoLimit; ++index) {
    setup_valid_ego_cycle();
    currentStatus.egoCorrection = 100U + index;
    TEST_ASSERT_EQUAL(currentStatus.egoCorrection+1U, correctionAFRClosedLoop());
  }
  setup_valid_ego_cycle();
  TEST_ASSERT_EQUAL(100U+configPage6.egoLimit, correctionAFRClosedLoop());
  setup_valid_ego_cycle();
  TEST_ASSERT_EQUAL(100U+configPage6.egoLimit, correctionAFRClosedLoop());
  setup_valid_ego_cycle();
  TEST_ASSERT_EQUAL(100U+configPage6.egoLimit, correctionAFRClosedLoop());  
}

static void test_corrections_closedloop_simple_rich(void) {
  setup_ego_simple();
  currentStatus.O2 = currentStatus.afrTarget - 1U;
  TEST_ASSERT_EQUAL(currentStatus.egoCorrection-1U, correctionAFRClosedLoop());
}

static void test_rich_max_correction(void) {
  currentStatus.O2 = configPage6.ego_min+1U;

  uint8_t correction = 100U; 
  uint8_t counter = 0;
  while (correction>(100U-configPage6.egoLimit)) {
    setup_valid_ego_cycle();
    currentStatus.egoCorrection = 100U - counter;
    correction = correctionAFRClosedLoop();
    TEST_ASSERT_LESS_THAN(100U, correction);
    ++counter;
  }
  setup_valid_ego_cycle();
  TEST_ASSERT_EQUAL(100U-configPage6.egoLimit, correctionAFRClosedLoop());
  setup_valid_ego_cycle();
  TEST_ASSERT_EQUAL(100U-configPage6.egoLimit, correctionAFRClosedLoop());
  setup_valid_ego_cycle();
  TEST_ASSERT_EQUAL(100U-configPage6.egoLimit, correctionAFRClosedLoop());  
}

static void test_corrections_closedloop_simple_rich_maxcorrection(void) {
  setup_ego_simple();

  test_rich_max_correction();
}

static void setup_ego_pid(void) {
  setup_ego_simple();
  configPage6.egoType = EGO_TYPE_WIDE;
  configPage6.egoAlgorithm = EGO_ALGORITHM_PID;  
  configPage6.egoKP = 50U;
  configPage6.egoKI = 20U;
  configPage6.egoKD = 10U;

  // Initial PID controller setup
  correctionAFRClosedLoop();
  setup_valid_ego_cycle();
}

// PID is time based and may need multiple cycles to move
static uint8_t run_pid(uint8_t cycles, uint8_t delayMillis) {
  for (uint8_t index=0; index<cycles-1U; ++index) {
    setup_valid_ego_cycle();
    // Serial.print(currentStatus.O2); Serial.print(" ");
    // Serial.print(currentStatus.afrTarget); Serial.print(" ");
    // Serial.println(correctionAFRClosedLoop());
    (void)correctionAFRClosedLoop();
    delay(delayMillis);
  }
  setup_valid_ego_cycle();
  return correctionAFRClosedLoop();
}

static void test_corrections_closedloop_pid_nocorrection(void) {
  setup_ego_pid();
  currentStatus.O2 = currentStatus.afrTarget;
  TEST_ASSERT_EQUAL(100U, run_pid(10, 10));
}

static void test_corrections_closedloop_pid_lean(void) {
  setup_ego_pid();
  currentStatus.O2 = configPage6.ego_max-1U;

  TEST_ASSERT_GREATER_THAN(100U, run_pid(10, 10));
}

static void test_corrections_closedloop_pid_lean_maxcorrection(void) {
  setup_ego_pid();

  currentStatus.O2 = configPage6.ego_max-1U;

  TEST_ASSERT_EQUAL(100U+configPage6.egoLimit, run_pid(40, 10));
  setup_valid_ego_cycle();
  TEST_ASSERT_EQUAL(100U+configPage6.egoLimit, correctionAFRClosedLoop());
  setup_valid_ego_cycle();
  TEST_ASSERT_EQUAL(100U+configPage6.egoLimit, correctionAFRClosedLoop());
  setup_valid_ego_cycle();
  TEST_ASSERT_EQUAL(100U+configPage6.egoLimit, correctionAFRClosedLoop());
}


static void test_corrections_closedloop_pid_rich(void) {
  setup_ego_pid();
  currentStatus.O2 = configPage6.ego_min+1U;
  TEST_ASSERT_LESS_THAN(100U, run_pid(10, 10));
}

static void test_corrections_closedloop_pid_rich_maxcorrection(void) {
  setup_ego_pid();

  currentStatus.O2 = configPage6.ego_min+1U;

  TEST_ASSERT_EQUAL(100U-configPage6.egoLimit, run_pid(40, 10));
  setup_valid_ego_cycle();
  TEST_ASSERT_EQUAL(100U-configPage6.egoLimit, correctionAFRClosedLoop());
  setup_valid_ego_cycle();
  TEST_ASSERT_EQUAL(100U-configPage6.egoLimit, correctionAFRClosedLoop());
  setup_valid_ego_cycle();
  TEST_ASSERT_EQUAL(100U-configPage6.egoLimit, correctionAFRClosedLoop());
}

static void test_corrections_closedloop(void)
{
  RUN_TEST_P(test_corrections_closedloop_off_nosensor);
  RUN_TEST_P(test_corrections_closedloop_off_dfco);
  RUN_TEST_P(test_corrections_closedloop_off_no_algorithm);
  RUN_TEST_P(test_corrections_closedloop_off_invalidconditions_coolant);
  RUN_TEST_P(test_corrections_closedloop_off_invalidconditions_rpm);
  RUN_TEST_P(test_corrections_closedloop_off_invalidconditions_tps);
  RUN_TEST_P(test_corrections_closedloop_off_invalidconditions_map);
  RUN_TEST_P(test_corrections_closedloop_off_invalidconditions_o2);
  RUN_TEST_P(test_corrections_closedloop_outsidecycle);
  RUN_TEST_P(test_corrections_closedloop_cycle_countrollover);
  RUN_TEST_P(test_corrections_closedloop_simple_nocorrection);
  RUN_TEST_P(test_corrections_closedloop_simple_lean);
  RUN_TEST_P(test_corrections_closedloop_simple_lean_maxcorrection);
  RUN_TEST_P(test_corrections_closedloop_simple_rich);
  RUN_TEST_P(test_corrections_closedloop_simple_rich_maxcorrection);
  RUN_TEST_P(test_corrections_closedloop_pid_nocorrection);
  RUN_TEST_P(test_corrections_closedloop_pid_lean);
  RUN_TEST_P(test_corrections_closedloop_pid_lean_maxcorrection);
  RUN_TEST_P(test_corrections_closedloop_pid_rich);
  RUN_TEST_P(test_corrections_closedloop_pid_rich_maxcorrection);
}


uint8_t correctionFlex(const statuses &current, const config2 &page2, const table2D &lookUptable);

struct flex_test_data_t : public test_2dtable_t<6> {
  statuses current;
  config2 page2;
};

static void setupFlexFuelTable(flex_test_data_t &testData) {
  TEST_DATA_P uint8_t bins[_countof(flex_test_data_t::bins)] = { 0, 10, 30, 50, 60, 70 };
  TEST_DATA_P uint8_t values[_countof(flex_test_data_t::values)] = { 0, 20, 40, 80, 120, 150 };
  populate_2dtable_P(&testData.lookupTable, values, bins);  
}

static void test_corrections_flex_flex_off(void) {
  flex_test_data_t testData;
  setupFlexFuelTable(testData);
  testData.page2.flexEnabled = false;
  testData.current.ethanolPct = 65;
  TEST_ASSERT_EQUAL(100U, correctionFlex(testData.current, testData.page2, testData.lookupTable) );
}

static void test_corrections_flex_flex_on(void) {
  flex_test_data_t testData;
  setupFlexFuelTable(testData);
  testData.page2.flexEnabled = true;
  testData.current.ethanolPct = 65;
  TEST_ASSERT_EQUAL(135U, correctionFlex(testData.current, testData.page2, testData.lookupTable) );
}

static void test_corrections_flex(void)
{
  RUN_TEST_P(test_corrections_flex_flex_off);
  RUN_TEST_P(test_corrections_flex_flex_on);
}

uint8_t correctionFuelTemp(void);

static void setupFuelTempTable(void) {
  construct2dTables();
  initialiseCorrections();

  TEST_DATA_P uint8_t bins[] = { 0, 10, 30, 50, 60, 70 };
  TEST_DATA_P uint8_t values[] = { 0, 20, 40, 80, 120, 150 };
  populate_2dtable_P(&fuelTempTable, values, bins);   
}

static void test_corrections_fueltemp_off(void) {
  setupFuelTempTable();
  configPage2.flexEnabled = false;
  currentStatus.fuelTemp = toWorkingTemperature(65);
  TEST_ASSERT_EQUAL(100U, correctionFuelTemp() );
}

static void test_corrections_fueltemp_on(void) {
  setupFuelTempTable();
  configPage2.flexEnabled = true;
  currentStatus.fuelTemp = toWorkingTemperature(65);
  TEST_ASSERT_EQUAL(135U, correctionFuelTemp() );
}

static void test_corrections_fueltemp(void)
{
  RUN_TEST_P(test_corrections_fueltemp_off);
  RUN_TEST_P(test_corrections_fueltemp_on);
}

extern uint8_t correctionBatVoltage(const statuses &current, const table2D &lookupTable, const config2 &page2);

struct battery_testdata_t : public test_2dtable_t<6> {
  statuses current;
  config2 page2;
};

static void setup_battery_correction(battery_testdata_t &testData) {
  LOOP_TIMER = 0;
  BIT_SET(LOOP_TIMER, BAT_READ_TIMER_BIT);

  TEST_DATA_P uint8_t bins[] = { 60, 70, 80, 90, 100, 110 };
  TEST_DATA_P uint8_t values[] = { 130, 120, 110, 100, 90, 80 };
  populate_2dtable_P(&testData.lookupTable, values, bins);   
}

static void test_corrections_bat_mode_wholePw(void) {
  battery_testdata_t testData;
  setup_battery_correction(testData);

  testData.page2.battVCorMode = BATTV_COR_MODE_WHOLE;
  testData.current.battery10 = 75;
  testData.page2.injOpen = 10;
  inj_opentime_uS = testData.page2.injOpen * 100U;

  TEST_ASSERT_EQUAL(115U, correctionBatVoltage(testData.current, testData.lookupTable, testData.page2) );
  TEST_ASSERT_EQUAL(testData.page2.injOpen * 100U, inj_opentime_uS );

  testData.current.battery10 = 105;
  TEST_ASSERT_EQUAL(85U, correctionBatVoltage(testData.current, testData.lookupTable, testData.page2) );
  TEST_ASSERT_EQUAL(testData.page2.injOpen * 100U, inj_opentime_uS );
}

static void test_corrections_bat_mode_opentime(void) {
  battery_testdata_t testData;
  setup_battery_correction(testData);

  testData.page2.battVCorMode = BATTV_COR_MODE_OPENTIME;
  testData.current.battery10 = 75;
  testData.page2.injOpen = 10;
  inj_opentime_uS = testData.page2.injOpen * 100U;

  TEST_ASSERT_EQUAL(100U, correctionBatVoltage(testData.current, testData.lookupTable, testData.page2) );
  TEST_ASSERT_EQUAL(testData.page2.injOpen * 115U, inj_opentime_uS );
  // Run again & confirm inj_opentime_uS is unchanged
  TEST_ASSERT_EQUAL(100U, correctionBatVoltage(testData.current, testData.lookupTable, testData.page2) );
  TEST_ASSERT_EQUAL(testData.page2.injOpen * 115U, inj_opentime_uS );
}

static void test_corrections_bat(void)
{
  RUN_TEST_P(test_corrections_bat_mode_wholePw);
  RUN_TEST_P(test_corrections_bat_mode_opentime);
}

uint8_t correctionLaunch(const statuses &current, const config6 &page6);

static void test_corrections_launch_inactive(void) {
  statuses current;
  config6 page6;
  BIT_CLEAR(current.status2, BIT_STATUS2_HLAUNCH);
  BIT_CLEAR(current.status2, BIT_STATUS2_SLAUNCH);
  page6.lnchFuelAdd = 25;

  TEST_ASSERT_EQUAL(100U, correctionLaunch(current, page6) );
}

static void test_corrections_launch_hard(void) {
  statuses current;
  config6 page6;
  BIT_SET(current.status2, BIT_STATUS2_HLAUNCH);
  BIT_CLEAR(current.status2, BIT_STATUS2_SLAUNCH);
  page6.lnchFuelAdd = 25;

  TEST_ASSERT_EQUAL(125U, correctionLaunch(current, page6) );
}

static void test_corrections_launch_soft(void) {
  statuses current;
  config6 page6;
  BIT_CLEAR(current.status2, BIT_STATUS2_HLAUNCH);
  BIT_SET(current.status2, BIT_STATUS2_SLAUNCH);
  page6.lnchFuelAdd = 25;

  TEST_ASSERT_EQUAL(125U, correctionLaunch(current, page6) );
}

static void test_corrections_launch_both(void) {
  statuses current;
  config6 page6;
  BIT_SET(current.status2, BIT_STATUS2_HLAUNCH);
  BIT_SET(current.status2, BIT_STATUS2_SLAUNCH);
  page6.lnchFuelAdd = 25;

  TEST_ASSERT_EQUAL(125U, correctionLaunch(current, page6) );
}

static void test_corrections_launch(void)
{
  RUN_TEST_P(test_corrections_launch_inactive);
  RUN_TEST_P(test_corrections_launch_hard);
  RUN_TEST_P(test_corrections_launch_soft);
  RUN_TEST_P(test_corrections_launch_both);
}

struct dfco_test_data_t : public test_2dtable_t<10> {
  statuses current;
  config2 page2;
  config4 page4;
  config9 page9;
};

extern bool correctionDFCO(const statuses &current, const config2 &page2, const config4 &page4);

static void setup_DFCO_on_taper_off_no_delay(dfco_test_data_t &testData)
{
  //Sets all the required conditions to have the DFCO be active
  testData.page2.dfcoEnabled = 1; //Ensure DFCO option is turned on
  testData.current.RPM = 4000; //Set the current simulated RPM to a level above the DFCO rpm threshold
  testData.current.TPS = 0; //Set the simulated TPS to 0 
  testData.current.coolant = 80;
  testData.page4.dfcoRPM = 150; //DFCO enable RPM = 1500
  testData.page4.dfcoTPSThresh = 1;
  testData.page4.dfcoHyster = 25;
  testData.page2.dfcoMinCLT = 40; //Actually 0 with offset
  testData.page2.dfcoDelay = 0;
  testData.page9.dfcoTaperEnable = 0; //Enable

  correctionDFCO(testData.current, testData.page2, testData.page4);
}

//**********************************************************************************************************************
static void test_corrections_dfco_on(void)
{
  //Test under ideal conditions that DFCO goes active
  dfco_test_data_t testData;
  setup_DFCO_on_taper_off_no_delay(testData);

  TEST_ASSERT_TRUE(correctionDFCO(testData.current, testData.page2, testData.page4));
}

static void test_corrections_dfco_off_RPM()
{
  //Test that DFCO comes on and then goes off when the RPM drops below threshold
  dfco_test_data_t testData;
  setup_DFCO_on_taper_off_no_delay(testData);

  TEST_ASSERT_TRUE(correctionDFCO(testData.current, testData.page2, testData.page4)); //Make sure DFCO is on initially
  testData.current.RPM = 1000; //Set the current simulated RPM below the threshold + hyster
  TEST_ASSERT_FALSE(correctionDFCO(testData.current, testData.page2, testData.page4)); //Test DFCO is now off
}

static void test_corrections_dfco_off_TPS()
{
  //Test that DFCO comes on and then goes off when the TPS goes above the required threshold (ie not off throttle)
  dfco_test_data_t testData;
  setup_DFCO_on_taper_off_no_delay(testData);

  TEST_ASSERT_TRUE(correctionDFCO(testData.current, testData.page2, testData.page4)); //Make sure DFCO is on initially
  testData.current.TPS = 10; //Set the current simulated TPS to be above the threshold
  TEST_ASSERT_FALSE(correctionDFCO(testData.current, testData.page2, testData.page4)); //Test DFCO is now off
}

static void test_corrections_dfco_off_delay()
{
  //Test that DFCO comes will not activate if there has not been a long enough delay
  //The steup function below simulates a 2 second delay
  dfco_test_data_t testData;
  setup_DFCO_on_taper_off_no_delay(testData);

  BIT_SET(LOOP_TIMER, BIT_TIMER_10HZ);
  testData.page2.dfcoDelay = 5;
  
  for (uint8_t index = 0; index < testData.page2.dfcoDelay; ++index) {
    TEST_ASSERT_FALSE(correctionDFCO(testData.current, testData.page2, testData.page4)); //Make sure DFCO does not come on...
  }
  // ...until simulated delay period expires
  TEST_ASSERT_TRUE(correctionDFCO(testData.current, testData.page2, testData.page4)); 
}

static void setup_DFCO_on_taper_on_no_delay( dfco_test_data_t &testData)
{
  setup_DFCO_on_taper_off_no_delay(testData);

  testData.page9.dfcoTaperEnable = 1; //Enable
  testData.page9.dfcoTaperTime = 20; //2.0 second
  testData.page9.dfcoTaperFuel = 0; //Scale fuel to 0%
  testData.page9.dfcoTaperAdvance = 20; //Reduce 20deg until full fuel cut
}

extern byte correctionDFCOfuel(const statuses &current, const config9 &page9);

extern int8_t correctionDFCOignition(int8_t advance, const statuses &current, const config9 &page9);

static void test_correctionDFCOfuel_DFCO_off()
{
  dfco_test_data_t testData;
  setup_DFCO_on_taper_off_no_delay(testData);

  BIT_CLEAR(testData.current.status1, BIT_STATUS1_DFCO);
  TEST_ASSERT_EQUAL(100, correctionDFCOfuel(testData.current, testData.page9));
}

static void test_correctionDFCOfuel_notaper()
{
  dfco_test_data_t testData;
  setup_DFCO_on_taper_off_no_delay(testData);

  testData.page9.dfcoTaperEnable = 0; //Disable
  BIT_SET(testData.current.status1, BIT_STATUS1_DFCO);
  TEST_ASSERT_EQUAL(0, correctionDFCOfuel(testData.current, testData.page9));
}

static inline void reset_dfco_taper(dfco_test_data_t &testData) {
  BIT_CLEAR(testData.current.status1, BIT_STATUS1_DFCO);
  TEST_ASSERT_EQUAL(100, correctionDFCOfuel(testData.current, testData.page9));
  TEST_ASSERT_EQUAL(20, correctionDFCOignition(20, testData.current, testData.page9));
}

static inline void advance_dfco_taper(uint8_t count, dfco_test_data_t &testData) {
  BIT_SET(LOOP_TIMER, BIT_TIMER_10HZ);
  for (uint8_t index = 0; index < count; ++index) {
    (void)correctionDFCOfuel(testData.current, testData.page9);
  }
}

static void test_correctionDFCOfuel_taper()
{
  dfco_test_data_t testData;
  setup_DFCO_on_taper_on_no_delay(testData);

  reset_dfco_taper(testData);

  BIT_SET(testData.current.status1, BIT_STATUS1_DFCO);

  // 50% test
  advance_dfco_taper(testData.page9.dfcoTaperTime/2, testData);
  BIT_CLEAR(LOOP_TIMER, BIT_TIMER_10HZ);
  TEST_ASSERT_EQUAL(50, correctionDFCOfuel(testData.current, testData.page9));

  // 75% test
  advance_dfco_taper(testData.page9.dfcoTaperTime/4, testData);
  BIT_CLEAR(LOOP_TIMER, BIT_TIMER_10HZ);
  TEST_ASSERT_EQUAL(25, correctionDFCOfuel(testData.current, testData.page9));

  // Advance taper to 100%
  advance_dfco_taper(testData.page9.dfcoTaperTime/4, testData);

  // 100% & beyond test
  BIT_SET(LOOP_TIMER, BIT_TIMER_10HZ);
  TEST_ASSERT_EQUAL(0, correctionDFCOfuel(testData.current, testData.page9));
  TEST_ASSERT_EQUAL(0, correctionDFCOfuel(testData.current, testData.page9));
  TEST_ASSERT_EQUAL(0, correctionDFCOfuel(testData.current, testData.page9));
  TEST_ASSERT_EQUAL(0, correctionDFCOfuel(testData.current, testData.page9));
  TEST_ASSERT_EQUAL(0, correctionDFCOfuel(testData.current, testData.page9));
}

static void test_correctionDFCOignition_DFCO_off()
{
  dfco_test_data_t testData;
  setup_DFCO_on_taper_off_no_delay(testData);

  BIT_CLEAR(testData.current.status1, BIT_STATUS1_DFCO);
  TEST_ASSERT_EQUAL(45, correctionDFCOignition(45, testData.current, testData.page9));
}

static void test_correctionDFCOignition_notaper()
{
  dfco_test_data_t testData;
  setup_DFCO_on_taper_off_no_delay(testData);

  testData.page9.dfcoTaperEnable = 0; //Disable
  BIT_SET(testData.current.status1, BIT_STATUS1_DFCO);
  TEST_ASSERT_EQUAL(45, correctionDFCOignition(45, testData.current, testData.page9));
}

static void test_correctionDFCOignition_taper()
{
  dfco_test_data_t testData;
  setup_DFCO_on_taper_on_no_delay(testData);

  reset_dfco_taper(testData);

  BIT_SET(testData.current.status1, BIT_STATUS1_DFCO);

  // 25% test
  advance_dfco_taper(testData.page9.dfcoTaperTime/4, testData);
  TEST_ASSERT_EQUAL(15, correctionDFCOignition(20, testData.current, testData.page9));

  // 50% test
  advance_dfco_taper(testData.page9.dfcoTaperTime/4, testData);
  TEST_ASSERT_EQUAL(10, correctionDFCOignition(20, testData.current, testData.page9));

  // 75% test
  advance_dfco_taper(testData.page9.dfcoTaperTime/4, testData);
  TEST_ASSERT_EQUAL(5, correctionDFCOignition(20, testData.current, testData.page9));

  // 100% & beyond test
  advance_dfco_taper(testData.page9.dfcoTaperTime/4, testData);
  TEST_ASSERT_EQUAL(0, correctionDFCOignition(20, testData.current, testData.page9));
  advance_dfco_taper(1, testData);
  TEST_ASSERT_EQUAL(0, correctionDFCOignition(20, testData.current, testData.page9));
  advance_dfco_taper(1, testData);
  TEST_ASSERT_EQUAL(0, correctionDFCOignition(20, testData.current, testData.page9));
}

static void test_corrections_dfco()
{
  RUN_TEST_P(test_corrections_dfco_on);
  RUN_TEST_P(test_corrections_dfco_off_RPM);
  RUN_TEST_P(test_corrections_dfco_off_TPS);
  RUN_TEST_P(test_corrections_dfco_off_delay);
  RUN_TEST_P(test_correctionDFCOfuel_DFCO_off);
  RUN_TEST_P(test_correctionDFCOfuel_notaper);
  RUN_TEST_P(test_correctionDFCOfuel_taper);
  RUN_TEST_P(test_correctionDFCOignition_DFCO_off);
  RUN_TEST_P(test_correctionDFCOignition_notaper);
  RUN_TEST_P(test_correctionDFCOignition_taper);
}

//**********************************************************************************************************************
//Setup a basic TAE enrichment curve, threshold etc that are common to all tests. Specifica values maybe updated in each individual test

struct ae_test_data_t :public test_2dtable_t<4> {
  statuses current;
  config2 page2;
};

static void reset_AE(statuses &current) {
  BIT_CLEAR(current.engine, BIT_ENGINE_ACC);
  BIT_CLEAR(current.engine, BIT_ENGINE_DCC);
}

static void setup_AE(statuses &current, config2 &page2) {
  //Divided by 100
  page2.aeTaperMin = 10; //1000
  page2.aeTaperMax = 50; //5000
	
	//Set the coolant to be above the warmup AE taper
	page2.aeColdTaperMax = 60;
	page2.aeColdTaperMin = 0;
	current.coolant = toWorkingTemperature(page2.aeColdTaperMax) + 1;

  current.AEEndTime = micros();

  reset_AE(current);
}

static void setup_TAE(statuses &current, config2 &page2, table2D &taeLookup) {
  setup_AE(current, page2);

  LOOP_TIMER = 0;
  BIT_SET(LOOP_TIMER, TPS_READ_TIMER_BIT);
  
  page2.aeMode = AE_MODE_TPS; //Set AE to TPS
  page2.taeThresh = 0;
  page2.taeMinChange = 0;

  TEST_DATA_P uint8_t bins[_countof(ae_test_data_t::bins)] = { 0, 8, 22, 97 };
  TEST_DATA_P uint8_t values[_countof(ae_test_data_t::values)] = { 70, 103, 124, 136 };
  populate_2dtable_P(&taeLookup, values, bins); 
}

static void setup_TAE(ae_test_data_t &testData)
{
  setup_TAE(testData.current, testData.page2, testData.lookupTable);
}

extern uint16_t correctionAccel(statuses &current, const config2 &page2, const table2D &aeLookup, const table2D &mapLookupTable);

static void disable_AE_taper(statuses &current, config2 &page2) {
  //Disable the taper
  current.RPM = 2000;
  page2.aeTaperMin = 50; //5000
  page2.aeTaperMax = 60; //6000
}

static void test_corrections_TAE_no_rpm_taper()
{
  ae_test_data_t testData;
  setup_TAE(testData);
  disable_AE_taper(testData.current, testData.page2);

  testData.current.TPSlast = 0;
  testData.current.TPS = 50; //25% actual value

  uint16_t accelValue = correctionAccel(testData.current, testData.page2, testData.lookupTable, table2D()); //Run the AE calcs

  TEST_ASSERT_EQUAL(750, testData.current.tpsDOT); //DOT is 750%/s (25 * 30)
  TEST_ASSERT_EQUAL((100+132), accelValue);
  TEST_ASSERT_BIT_HIGH(BIT_ENGINE_ACC, testData.current.engine); //Confirm AE is flagged on
  TEST_ASSERT_BIT_LOW(BIT_ENGINE_DCC, testData.current.engine); //Confirm AE is flagged on
  
  // No change
  reset_AE(testData.current);
  testData.current.TPSlast = 50;
  testData.current.TPS = 50;
  accelValue = correctionAccel(testData.current, testData.page2, testData.lookupTable, table2D()); //Run the AE calcs
  TEST_ASSERT_EQUAL(0, testData.current.tpsDOT);
  TEST_ASSERT_EQUAL(100, accelValue);
  TEST_ASSERT_BIT_LOW(BIT_ENGINE_ACC, testData.current.engine); //Confirm AE is flagged off
  TEST_ASSERT_BIT_LOW(BIT_ENGINE_DCC, testData.current.engine); //Confirm AE is flagged on

  // Small change   
  reset_AE(testData.current);
  testData.current.TPSlast = 50;
  testData.current.TPS = 51;
  accelValue = correctionAccel(testData.current, testData.page2, testData.lookupTable, table2D()); //Run the AE calcs
  TEST_ASSERT_EQUAL(15, testData.current.tpsDOT);
  TEST_ASSERT_EQUAL(100+74, accelValue);
  TEST_ASSERT_BIT_HIGH(BIT_ENGINE_ACC, testData.current.engine); //Confirm AE is flagged on
  TEST_ASSERT_BIT_LOW(BIT_ENGINE_DCC, testData.current.engine); //Confirm AE is flagged on

  // Large change
  reset_AE(testData.current);
  testData.current.TPSlast = 0;
  testData.current.TPS = 100;
  accelValue = correctionAccel(testData.current, testData.page2, testData.lookupTable, table2D()); //Run the AE calcs
  TEST_ASSERT_EQUAL(1500, testData.current.tpsDOT);
  TEST_ASSERT_EQUAL(100+136, accelValue);
  TEST_ASSERT_BIT_HIGH(BIT_ENGINE_ACC, testData.current.engine); //Confirm AE is flagged on
  TEST_ASSERT_BIT_LOW(BIT_ENGINE_DCC, testData.current.engine); //Confirm AE is flagged on
}

static void test_corrections_TAE_negative_tpsdot()
{
  ae_test_data_t testData;
  setup_TAE(testData);
  disable_AE_taper(testData.current, testData.page2);

  testData.page2.decelAmount = 50;
  testData.current.TPSlast = 50;
  testData.current.TPS = 0;

  uint16_t accelValue = correctionAccel(testData.current, testData.page2, testData.lookupTable, table2D());  //Run the AE calcs

  TEST_ASSERT_EQUAL(-750, testData.current.tpsDOT); //DOT is 750%/s (25 * 30)
  TEST_ASSERT_EQUAL(testData.page2.decelAmount, accelValue);
  TEST_ASSERT_BIT_LOW(BIT_ENGINE_ACC, testData.current.engine); //Confirm AE is flagged on
  TEST_ASSERT_BIT_HIGH(BIT_ENGINE_DCC, testData.current.engine); //Confirm AE is flagged on
}

static void test_corrections_TAE_50pc_rpm_taper()
{
  ae_test_data_t testData;
  setup_TAE(testData);

  //RPM is 50% of the way through the taper range
  testData.current.RPM = 3000;
  testData.page2.aeTaperMin = 10; //1000
  testData.page2.aeTaperMax = 50; //5000

  testData.current.TPSlast = 0;
  testData.current.TPS = 50; //25% actual value

  uint16_t accelValue = correctionAccel(testData.current, testData.page2, testData.lookupTable, table2D());  //Run the AE calcs

  TEST_ASSERT_EQUAL(750, testData.current.tpsDOT); //DOT is 750%/s (25 * 30)
  TEST_ASSERT_EQUAL((100+66), accelValue);
  TEST_ASSERT_BIT_HIGH(BIT_ENGINE_ACC, testData.current.engine); //Confirm AE is flagged on
  TEST_ASSERT_BIT_LOW(BIT_ENGINE_DCC, testData.current.engine); //Confirm AE is flagged on
}

static void test_corrections_TAE_110pc_rpm_taper()
{
  ae_test_data_t testData;
  setup_TAE(testData);

  //RPM is 110% of the way through the taper range, which should result in no additional AE
  testData.current.RPM = 5400;
  testData.page2.aeTaperMin = 10; //1000
  testData.page2.aeTaperMax = 50; //5000

  testData.current.TPSlast = 0;
  testData.current.TPS = 50; //25% actual value

  uint16_t accelValue = correctionAccel(testData.current, testData.page2, testData.lookupTable, table2D());  //Run the AE calcs

  TEST_ASSERT_EQUAL(750, testData.current.tpsDOT); //DOT is 750%/s (25 * 30)
  TEST_ASSERT_EQUAL(100, accelValue); //Should be no AE as we're above the RPM taper end point
  TEST_ASSERT_BIT_HIGH(BIT_ENGINE_ACC, testData.current.engine); //Confirm AE is flagged on
  TEST_ASSERT_BIT_LOW(BIT_ENGINE_DCC, testData.current.engine); //Confirm AE is flagged on
}

static void test_corrections_TAE_under_threshold()
{
  ae_test_data_t testData;
  setup_TAE(testData);

  //RPM is 50% of the way through the taper range, but TPS value will be below threshold
  testData.current.RPM = 3000;
  testData.page2.aeTaperMin = 10; //1000
  testData.page2.aeTaperMax = 50; //5000

  testData.current.TPSlast = 0;
  testData.current.TPS = 6; //3% actual value. TPSDot should be 90%/s
	testData.page2.taeThresh = 100; //Above the reading of 90%/s

  uint16_t accelValue = correctionAccel(testData.current, testData.page2, testData.lookupTable, table2D());  //Run the AE calcs

  TEST_ASSERT_EQUAL(90, testData.current.tpsDOT); //DOT is 90%/s (3% * 30)
  TEST_ASSERT_EQUAL(100, accelValue); //Should be no AE as we're above the RPM taper end point
  TEST_ASSERT_BIT_LOW(BIT_ENGINE_ACC, testData.current.engine); //Confirm AE is flagged off
  TEST_ASSERT_BIT_LOW(BIT_ENGINE_DCC, testData.current.engine); //Confirm AE is flagged on
}

static void test_corrections_TAE_50pc_warmup_taper()
{
  ae_test_data_t testData;
  setup_TAE(testData);
  disable_AE_taper(testData.current, testData.page2);

  testData.current.TPSlast = 0;
  testData.current.TPS = 50; //25% actual value
	
	//Set a cold % of 50% increase
	testData.page2.aeColdPct = 150;
	testData.page2.aeColdTaperMax = toStorageTemperature(60);
	testData.page2.aeColdTaperMin = toStorageTemperature(0);
	//Set the coolant to be 50% of the way through the warmup range
	testData.current.coolant = 30;

  uint16_t accelValue = correctionAccel(testData.current, testData.page2, testData.lookupTable, table2D());  //Run the AE calcs

  TEST_ASSERT_EQUAL(750, testData.current.tpsDOT); //DOT is 750%/s (25 * 30)
  TEST_ASSERT_EQUAL((100+165), accelValue); //Total AE should be 132 + (50% * 50%) = 132 * 1.25 = 165
  TEST_ASSERT_BIT_HIGH(BIT_ENGINE_ACC, testData.current.engine); //Confirm AE is flagged on
  TEST_ASSERT_BIT_LOW(BIT_ENGINE_DCC, testData.current.engine); //Confirm AE is flagged on
}

static void test_corrections_TAE_timout()
{
  ae_test_data_t testData;
  setup_TAE(testData);
  disable_AE_taper(testData.current, testData.page2);

  // Confirm TAE is on
  testData.current.TPSlast = 0;
  testData.current.TPS = 50; //25% actual value
  testData.page2.aeTime = 0; // This should cause the testData.current cycle to expire & the next one to not occur.

  TEST_ASSERT_EQUAL((100+132), correctionAccel(testData.current, testData.page2, testData.lookupTable, table2D()));
  TEST_ASSERT_EQUAL(750, testData.current.tpsDOT); //DOT is 750%/s (25 * 30)
  TEST_ASSERT_BIT_HIGH(BIT_ENGINE_ACC, testData.current.engine); //Confirm AE is flagged on
  TEST_ASSERT_BIT_LOW(BIT_ENGINE_DCC, testData.current.engine); //Confirm AE is flagged on
  
  // TAE should have timed out
  TEST_ASSERT_EQUAL(100, correctionAccel(testData.current, testData.page2, testData.lookupTable, table2D()));
  TEST_ASSERT_EQUAL(0, testData.current.tpsDOT);
  TEST_ASSERT_BIT_LOW(BIT_ENGINE_ACC, testData.current.engine); //Confirm AE is flagged off
  TEST_ASSERT_BIT_LOW(BIT_ENGINE_DCC, testData.current.engine); //Confirm AE is flagged off

  // But TPS hasn't changed position so another cycle should begin
  TEST_ASSERT_EQUAL((100+132), correctionAccel(testData.current, testData.page2, testData.lookupTable, table2D()));
  TEST_ASSERT_EQUAL(750, testData.current.tpsDOT); //DOT is 750%/s (25 * 30)
  TEST_ASSERT_BIT_HIGH(BIT_ENGINE_ACC, testData.current.engine); //Confirm AE is flagged on
  TEST_ASSERT_BIT_LOW(BIT_ENGINE_DCC, testData.current.engine); //Confirm AE is flagged on
}

static void test_corrections_TAE()
{
  RUN_TEST_P(test_corrections_TAE_negative_tpsdot);
  RUN_TEST_P(test_corrections_TAE_no_rpm_taper);
  RUN_TEST_P(test_corrections_TAE_50pc_rpm_taper);
  RUN_TEST_P(test_corrections_TAE_110pc_rpm_taper);
  RUN_TEST_P(test_corrections_TAE_under_threshold);
  RUN_TEST_P(test_corrections_TAE_50pc_warmup_taper);
  RUN_TEST_P(test_corrections_TAE_timout);
}


//**********************************************************************************************************************
//Setup a basic MAE enrichment curve, threshold etc that are common to all tests. Specifica values maybe updated in each individual test
static void setup_MAE(ae_test_data_t &testData)
{
  setup_AE(testData.current, testData.page2);

  testData.page2.aeMode = AE_MODE_MAP; //Set AE to MP
  LOOP_TIMER = 0;
  BIT_SET(LOOP_TIMER, MAP_READ_TIMER_BIT);

  TEST_DATA_P uint8_t bins[_countof(ae_test_data_t::bins)] = { 0, 15, 19, 50 };
  TEST_DATA_P uint8_t values[_countof(ae_test_data_t::values)] = { 70, 103, 124, 136 };
  populate_2dtable_P(&testData.lookupTable, values, bins); 

  testData.page2.maeThresh = 0;
  testData.page2.maeMinChange = 0;
}

static void test_corrections_MAE_negative_mapdot()
{
  ae_test_data_t testData;
  setup_MAE(testData);
  disable_AE_taper(testData.current, testData.page2);

  testData.page2.decelAmount = 50;
  MAPlast_time = UINT16_MAX*2UL;
  MAP_time = MAPlast_time + 25000UL; 
  MAPlast = 50;
  testData.current.MAP = 40;

  uint16_t accelValue = correctionAccel(testData.current, testData.page2, table2D(), testData.lookupTable); //Run the AE calcs

  TEST_ASSERT_EQUAL(-400, testData.current.mapDOT);
  TEST_ASSERT_EQUAL(testData.page2.decelAmount, accelValue);
	TEST_ASSERT_BIT_LOW(BIT_ENGINE_ACC, testData.current.engine); //Confirm AE is flagged on
	TEST_ASSERT_BIT_HIGH(BIT_ENGINE_DCC, testData.current.engine); //Confirm AE is flagged on
}

static void test_corrections_MAE_no_rpm_taper()
{
  ae_test_data_t testData;
  setup_MAE(testData);
  disable_AE_taper(testData.current, testData.page2);

  MAPlast_time = UINT16_MAX*2UL;
  MAP_time = MAPlast_time + 25000UL; 
  MAPlast = 40;
  testData.current.MAP = 50;

  uint16_t accelValue = correctionAccel(testData.current, testData.page2, table2D(), testData.lookupTable); //Run the AE calcs
  TEST_ASSERT_EQUAL(400, testData.current.mapDOT);
  TEST_ASSERT_EQUAL((100+132), accelValue);
	TEST_ASSERT_BIT_HIGH(BIT_ENGINE_ACC, testData.current.engine); //Confirm AE is flagged on
	TEST_ASSERT_BIT_LOW(BIT_ENGINE_DCC, testData.current.engine); //Confirm AE is flagged on

  // No change
  reset_AE(testData.current);
  MAPlast_time = UINT16_MAX*2UL;
  MAP_time = MAPlast_time + 1000UL; 
  MAPlast = 40;
  testData.current.MAP = 40;
  accelValue = correctionAccel(testData.current, testData.page2, table2D(), testData.lookupTable); //Run the AE calcs
  TEST_ASSERT_EQUAL(0, testData.current.mapDOT);
  TEST_ASSERT_EQUAL(100, accelValue);
	TEST_ASSERT_BIT_LOW(BIT_ENGINE_ACC, testData.current.engine); //Confirm AE is flagged off
	TEST_ASSERT_BIT_LOW(BIT_ENGINE_DCC, testData.current.engine); //Confirm AE is flagged on

  // Small change over small time period  
  reset_AE(testData.current);
  MAPlast_time = UINT16_MAX*2UL;
  MAP_time = MAPlast_time + 1000UL; 
  MAPlast = 40;
  testData.current.MAP = 41;
  accelValue = correctionAccel(testData.current, testData.page2, table2D(), testData.lookupTable); //Run the AE calcs
  TEST_ASSERT_EQUAL(1000, testData.current.mapDOT);
  TEST_ASSERT_EQUAL((100+136), accelValue);
	TEST_ASSERT_BIT_HIGH(BIT_ENGINE_ACC, testData.current.engine); //Confirm AE is flagged on
	TEST_ASSERT_BIT_LOW(BIT_ENGINE_DCC, testData.current.engine); //Confirm AE is flagged on

  // Small change over long (>UINT16_MAX) time period  
  reset_AE(testData.current);
  MAPlast_time = UINT16_MAX*2UL;
  MAP_time = MAPlast_time + UINT16_MAX*2UL; 
  MAPlast = 40;
  testData.current.MAP = 41;
  accelValue = correctionAccel(testData.current, testData.page2, table2D(), testData.lookupTable); //Run the AE calcs
  TEST_ASSERT_EQUAL(7, testData.current.mapDOT);
  TEST_ASSERT_EQUAL(100+70, accelValue);
	TEST_ASSERT_BIT_HIGH(BIT_ENGINE_ACC, testData.current.engine); //Confirm AE is flagged on
	TEST_ASSERT_BIT_LOW(BIT_ENGINE_DCC, testData.current.engine); //Confirm AE is flagged on

  // Large change over small time period  
  reset_AE(testData.current);
  MAPlast_time = UINT16_MAX*2UL;
  MAP_time = MAPlast_time + 1000UL; 
  MAPlast = 10;
  testData.current.MAP = 1000;
  accelValue = correctionAccel(testData.current, testData.page2, table2D(), testData.lookupTable); //Run the AE calcs
  TEST_ASSERT_EQUAL(2550, testData.current.mapDOT);
  TEST_ASSERT_EQUAL((100+136), accelValue);
	TEST_ASSERT_BIT_HIGH(BIT_ENGINE_ACC, testData.current.engine); //Confirm AE is flagged on
	TEST_ASSERT_BIT_LOW(BIT_ENGINE_DCC, testData.current.engine); //Confirm AE is flagged on

  // Large change over long (>UINT16_MAX) time period  
  reset_AE(testData.current);
  MAPlast_time = UINT16_MAX*2UL;
  MAP_time = MAPlast_time + UINT16_MAX*2; 
  MAPlast = 10;
  testData.current.MAP = 1000;
  accelValue = correctionAccel(testData.current, testData.page2, table2D(), testData.lookupTable); //Run the AE calcs
  TEST_ASSERT_EQUAL(2550, testData.current.mapDOT);
  TEST_ASSERT_EQUAL(100+136, accelValue);
	TEST_ASSERT_BIT_HIGH(BIT_ENGINE_ACC, testData.current.engine); //Confirm AE is flagged pn  
	TEST_ASSERT_BIT_LOW(BIT_ENGINE_DCC, testData.current.engine); //Confirm AE is flagged on
}

static void test_corrections_MAE_50pc_rpm_taper()
{
  ae_test_data_t testData;
  setup_MAE(testData);

  //RPM is 50% of the way through the taper range
  testData.current.RPM = 3000;
  testData.page2.aeTaperMin = 10; //1000
  testData.page2.aeTaperMax = 50; //5000

  MAPlast_time = UINT16_MAX*2UL;
  MAP_time = MAPlast_time + 25000UL; 
  MAPlast = 40;
  testData.current.MAP = 50;

  uint16_t accelValue = correctionAccel(testData.current, testData.page2, table2D(), testData.lookupTable); //Run the AE calcs

  TEST_ASSERT_EQUAL(400, testData.current.mapDOT);
  TEST_ASSERT_EQUAL((100+66), accelValue);
	TEST_ASSERT_BIT_HIGH(BIT_ENGINE_ACC, testData.current.engine); //Confirm AE is flagged on
}

static void test_corrections_MAE_110pc_rpm_taper()
{
  ae_test_data_t testData;
  setup_MAE(testData);

  //RPM is 110% of the way through the taper range, which should result in no additional AE
  testData.current.RPM = 5400;
  testData.page2.aeTaperMin = 10; //1000
  testData.page2.aeTaperMax = 50; //5000

  MAPlast_time = UINT16_MAX*2UL;
  MAP_time = MAPlast_time + 25000UL; 
  MAPlast = 40;
  testData.current.MAP = 50;

  uint16_t accelValue = correctionAccel(testData.current, testData.page2, table2D(), testData.lookupTable); //Run the AE calcs

  TEST_ASSERT_EQUAL(400, testData.current.mapDOT);
  TEST_ASSERT_EQUAL(100, accelValue); //Should be no AE as we're above the RPM taper end point
	TEST_ASSERT_BIT_HIGH(BIT_ENGINE_ACC, testData.current.engine); //Confirm AE is flagged on
}

static void test_corrections_MAE_under_threshold()
{
  ae_test_data_t testData;
  setup_MAE(testData);

  //RPM is 50% of the way through the taper range, but TPS value will be below threshold
  testData.current.RPM = 3000;
  testData.page2.aeTaperMin = 10; //1000
  testData.page2.aeTaperMax = 50; //5000

  MAPlast_time = UINT16_MAX*2UL;
  MAP_time = MAPlast_time + 25000UL; 
  MAPlast = 0;
  testData.current.MAP = 6; 
	testData.page2.maeThresh = 241; //Above the reading of 240%/s

  uint16_t accelValue = correctionAccel(testData.current, testData.page2, table2D(), testData.lookupTable); //Run the AE calcs

  TEST_ASSERT_EQUAL(240, testData.current.mapDOT);
  TEST_ASSERT_EQUAL(100, accelValue); //Should be no AE as we're above the RPM taper end point
	TEST_ASSERT_BIT_LOW(BIT_ENGINE_ACC, testData.current.engine); //Confirm AE is flagged off
}

static void test_corrections_MAE_50pc_warmup_taper()
{
  ae_test_data_t testData;
  setup_MAE(testData);
  disable_AE_taper(testData.current, testData.page2);

  MAPlast_time = UINT16_MAX*2UL;
  MAP_time = MAPlast_time + 25000UL; 
  MAPlast = 40;
  testData.current.MAP = 50;

	//Set a cold % of 50% increase
	testData.page2.aeColdPct = 150;
	testData.page2.aeColdTaperMax = toStorageTemperature(60);
	testData.page2.aeColdTaperMin = toStorageTemperature(0);
	//Set the coolant to be 50% of the way through the warmup range
	testData.current.coolant = 30;

  uint16_t accelValue = correctionAccel(testData.current, testData.page2, table2D(), testData.lookupTable); //Run the AE calcs

  TEST_ASSERT_EQUAL(400, testData.current.mapDOT);
  TEST_ASSERT_EQUAL((100+165), accelValue); 
	TEST_ASSERT_BIT_HIGH(BIT_ENGINE_ACC, testData.current.engine); //Confirm AE is flagged on
}

static void test_corrections_MAE_timeout()
{
  ae_test_data_t testData;
  setup_MAE(testData);
  disable_AE_taper(testData.current, testData.page2);

  // Confirm MAE is on
  MAPlast_time = UINT16_MAX*2UL;
  MAP_time = MAPlast_time + 25000UL; 
  MAPlast = 40;
  testData.current.MAP = 50;
  testData.page2.aeTime = 0; // This should cause the testData.current cycle to expire & the next one to not occur.
  TEST_ASSERT_EQUAL((100+132), correctionAccel(testData.current, testData.page2, table2D(), testData.lookupTable));
  TEST_ASSERT_EQUAL(400, testData.current.mapDOT);
	TEST_ASSERT_BIT_HIGH(BIT_ENGINE_ACC, testData.current.engine); //Confirm AE is flagged on
	TEST_ASSERT_BIT_LOW(BIT_ENGINE_DCC, testData.current.engine); //Confirm AE is flagged on

  // Timeout TAE
  TEST_ASSERT_EQUAL(100, correctionAccel(testData.current, testData.page2, table2D(), testData.lookupTable));
  TEST_ASSERT_EQUAL(0, testData.current.mapDOT);
	TEST_ASSERT_BIT_LOW(BIT_ENGINE_ACC, testData.current.engine); //Confirm AE is flagged on
  TEST_ASSERT_BIT_LOW(BIT_ENGINE_DCC, testData.current.engine); //Confirm AE is flagged on

  // But we haven't changed MAP readings so another cycle should begin
  TEST_ASSERT_EQUAL((100+132), correctionAccel(testData.current, testData.page2, table2D(), testData.lookupTable));
  TEST_ASSERT_EQUAL(400, testData.current.mapDOT);
	TEST_ASSERT_BIT_HIGH(BIT_ENGINE_ACC, testData.current.engine); //Confirm AE is flagged on
	TEST_ASSERT_BIT_LOW(BIT_ENGINE_DCC, testData.current.engine); //Confirm AE is flagged on
}

static void test_corrections_MAE()
{
  RUN_TEST_P(test_corrections_MAE_negative_mapdot);
  RUN_TEST_P(test_corrections_MAE_no_rpm_taper);
  RUN_TEST_P(test_corrections_MAE_50pc_rpm_taper);
  RUN_TEST_P(test_corrections_MAE_110pc_rpm_taper);
  RUN_TEST_P(test_corrections_MAE_under_threshold);
  RUN_TEST_P(test_corrections_MAE_50pc_warmup_taper);
  RUN_TEST_P(test_corrections_MAE_timeout);
}

static void setup_afrtarget(table3d16RpmLoad &afrLookUpTable,
                            statuses &current,
                            config2 &page2,
                            config6 &page6) {
  TEST_DATA_P table3d_value_t values[] = {
    //0    1    2   3     4    5    6    7    8    9   10   11   12   13    14   15
    34,  34,  34,  34,  34,  34,  34,  34,  34,  35,  35,  35,  35,  35,  35,  35, 
    34,  35,  36,  37,  39,  41,  42,  43,  43,  44,  44,  44,  44,  44,  44,  44, 
    35,  36,  38,  41,  44,  46,  47,  48,  48,  49,  49,  49,  49,  49,  49,  49, 
    36,  39,  42,  46,  50,  51,  52,  53,  53,  53,  53,  53,  53,  53,  53,  53, 
    38,  43,  48,  52,  55,  56,  57,  58,  58,  58,  58,  58,  58,  58,  58,  58, 
    42,  49,  54,  58,  61,  62,  62,  63,  63,  63,  63,  63,  63,  63,  63,  63, 
    48,  56,  60,  64,  66,  66,  68,  68,  68,  68,  68,  68,  68,  68,  68,  68, 
    54,  62,  66,  69,  71,  71,  72,  72,  72,  72,  72,  72,  72,  72,  72,  72, 
    61,  69,  72,  74,  76,  76,  77,  77,  77,  77,  77,  77,  77,  77,  77,  77, 
    68,  75,  78,  79,  81,  81,  81,  82,  82,  82,  82,  82,  82,  82,  82,  82, 
    74,  80,  83,  84,  85,  86,  86,  86,  87,  87,  87,  87,  87,  87,  87,  87, 
    81,  86,  88,  89,  90,  91,  91,  91,  91,  91,  91,  91,  91,  91,  91,  91, 
    93,  96,  98,  99,  99,  100, 100, 101, 101, 101, 101, 101, 101, 101, 101, 101, 
    98,  101, 103, 103, 104, 105, 105, 105, 105, 105, 105, 105, 105, 105, 105, 105, 
    104, 106, 107, 108, 109, 109, 110, 110, 110, 110, 110, 110, 110, 110, 110, 110, 
    109, 111, 112, 113, 114, 114, 114, 115, 115, 115, 114, 114, 114, 114, 114, 114, 
    };
  TEST_DATA_P table3d_axis_t xAxis[] = {500, 700, 900, 1200, 1600, 2000, 2500, 3100, 3500, 4100, 4700, 5300, 5900, 6500, 6750, 7000};
  TEST_DATA_P table3d_axis_t yAxis[] = { 16, 26, 30, 36, 40, 46, 50, 56, 60, 66, 70, 76, 86, 90, 96, 100};  
  populate_table_P(afrLookUpTable, xAxis, yAxis, values);

  memset(&page2, 0, sizeof(page2));
  page2.incorporateAFR = true;

  memset(&page6, 0, sizeof(page6));
  page6.egoType = EGO_TYPE_NARROW;
  page6.ego_sdelay = 10;

  memset(&current, 0, sizeof(current));
  current.runSecs = page6.ego_sdelay + 2U;
  current.fuelLoad = 60;
  current.RPM = 3100;
  current.O2 = 75U;
}


static void test_corrections_afrtarget_no_compute(void) {
  table3d16RpmLoad afrLookUpTable;
  statuses current;
  config2 page2;
  config6 page6;
  setup_afrtarget(afrLookUpTable, current, page2, page6);

  page2.incorporateAFR = false;
  page6.egoType = EGO_TYPE_OFF;
  current.afrTarget = 111;

  TEST_ASSERT_EQUAL(current.afrTarget, calculateAfrTarget(afrLookUpTable, current, page2, page6));
}

static void test_corrections_afrtarget_no_compute_egodelay(void) {
  table3d16RpmLoad afrLookUpTable;
  statuses current;
  config2 page2;
  config6 page6;
  setup_afrtarget(afrLookUpTable, current, page2, page6);

  page2.incorporateAFR = false;
  current.runSecs = page6.ego_sdelay - 2U;
  current.afrTarget = current.O2/2U;

  TEST_ASSERT_EQUAL(current.O2, calculateAfrTarget(afrLookUpTable, current, page2, page6));
}

static void test_corrections_afrtarget_incorporteafr(void) {
  table3d16RpmLoad afrLookUpTable;
  statuses current;
  config2 page2;
  config6 page6;
  setup_afrtarget(afrLookUpTable, current, page2, page6);

  page2.incorporateAFR = true;
  page6.egoType = EGO_TYPE_OFF;

  TEST_ASSERT_EQUAL(77U, calculateAfrTarget(afrLookUpTable, current, page2, page6));
}

static void test_corrections_afrtarget_ego(void) {
  table3d16RpmLoad afrLookUpTable;
  statuses current;
  config2 page2;
  config6 page6;
  setup_afrtarget(afrLookUpTable, current, page2, page6);

  page2.incorporateAFR = false;
  page6.egoType = EGO_TYPE_NARROW;

  TEST_ASSERT_EQUAL(77U, calculateAfrTarget(afrLookUpTable, current, page2, page6));
}

static void test_corrections_afrtarget(void) {
  RUN_TEST_P(test_corrections_afrtarget_no_compute);
  RUN_TEST_P(test_corrections_afrtarget_no_compute_egodelay);
  RUN_TEST_P(test_corrections_afrtarget_incorporteafr);
  RUN_TEST_P(test_corrections_afrtarget_ego);
}

extern byte correctionIATDensity(const statuses &current, const table2D &lookupTable);

#if !defined(_countof)
#define _countof(x) (sizeof(x) / sizeof (x[0]))
#endif
 
struct baro_test_data_t : public test_2dtable_t<8> {
  statuses current;
};

 
extern byte correctionBaro(const statuses &current, const table2D &lookupTable);

static void setup_baro_correction(baro_test_data_t &testData) {
  LOOP_TIMER = 0;
  BIT_SET(LOOP_TIMER, BARO_READ_TIMER_BIT);

  TEST_DATA_P uint8_t bins[] = { 60, 70, 80, 90, 100, 110 };
  TEST_DATA_P uint8_t values[] = { 120, 110, 100, 90, 80, 70 };
  populate_2dtable_P(&testData.lookupTable, values, bins);
}

// Battery correction will recalculates at 10Hz, otherwise it will re-use cached values. 
static void test_corrections_baro_lookup(void) {
  baro_test_data_t testData;
  setup_baro_correction(testData);

  testData.current.baro = 65;
  testData.current.baroCorrection = 1U;
  TEST_ASSERT_NOT_EQUAL(testData.current.baroCorrection, correctionBaro(testData.current, testData.lookupTable));
  TEST_ASSERT_EQUAL(115, correctionBaro(testData.current, testData.lookupTable));

  testData.current.baro = 105;
  testData.current.baroCorrection = 1U;
  TEST_ASSERT_EQUAL(75, correctionBaro(testData.current, testData.lookupTable));
  TEST_ASSERT_EQUAL(75, correctionBaro(testData.current, testData.lookupTable));
}

static void test_corrections_baro(void)
{
  RUN_TEST_P(test_corrections_baro_lookup);
}

static void test_corrections_correctionsFuel_ae_modes(void) {
  construct2dTables();
  initialiseCorrections();

  setup_TAE(currentStatus, configPage2, taeTable);
  //Disable the taper
  disable_AE_taper(currentStatus, configPage2);
  configPage2.decelAmount = 33U;
  configPage2.aseTaperTime = 0U;

  // Makes no sense in real life, but this is an artifical test
  BIT_SET(LOOP_TIMER, BIT_TIMER_4HZ);
  BIT_SET(LOOP_TIMER, BIT_TIMER_10HZ);
  populate_2dtable(&injectorVCorrectionTable, 100, 100);
  populate_2dtable(&baroFuelTable, 100, 100);
  populate_2dtable(&IATDensityCorrectionTable, 100, 100);
  populate_2dtable(&flexFuelTable, 100, 100);
  populate_2dtable(&fuelTempTable, 100, 100);

  currentStatus.TPSlast = 0;
  currentStatus.TPS = 50; //25% actual value
  currentStatus.coolant = 212;
  currentStatus.runSecs = 255; 
  currentStatus.battery10 = 90;  
  currentStatus.IAT = 100;
  BIT_CLEAR(currentStatus.status2, BIT_STATUS2_HLAUNCH);
  BIT_CLEAR(currentStatus.status2, BIT_STATUS2_SLAUNCH);
  BIT_CLEAR(currentStatus.status1, BIT_STATUS1_DFCO);
  BIT_CLEAR(currentStatus.engine, BIT_ENGINE_CRANK);
  currentStatus.ASEValue = 100U;

  configPage2.battVCorMode = BATTV_COR_MODE_WHOLE;
  configPage2.dfcoEnabled = 0;

  configPage4.dfcoRPM = 100;
  configPage4.wueBins[9] = 100;
  configPage2.wueValues[9] = 100; //Use a value other than 100 here to ensure we are using the non-default value
  WUETable.cacheTime = currentStatus.secl - 1;

  configPage4.floodClear = 100;

  configPage6.egoType = 0;
  configPage6.egoAlgorithm = EGO_ALGORITHM_SIMPLE;

  TEST_ASSERT_EQUAL_MESSAGE(100, correctionWUE(currentStatus, WUETable), "correctionWUE");
  TEST_ASSERT_EQUAL_MESSAGE(100, correctionASE(currentStatus, ASECountTable, ASETable, configPage2), "correctionASE");
  TEST_ASSERT_EQUAL_MESSAGE(100, correctionCranking(currentStatus, crankingEnrichTable, configPage10), "correctionCranking");
  TEST_ASSERT_EQUAL_MESSAGE(100, correctionFloodClear(currentStatus, configPage4), "correctionFloodClear");
  TEST_ASSERT_EQUAL_MESSAGE(100, correctionAFRClosedLoop(), "correctionAFRClosedLoop");
  TEST_ASSERT_EQUAL_MESSAGE(100, correctionBatVoltage(currentStatus, injectorVCorrectionTable, configPage2), "correctionBatVoltage");
  TEST_ASSERT_EQUAL_MESSAGE(100, correctionIATDensity(currentStatus, IATDensityCorrectionTable), "correctionIATDensity");
  TEST_ASSERT_EQUAL_MESSAGE(100, correctionBaro(currentStatus, baroFuelTable), "correctionBaro");
  TEST_ASSERT_EQUAL_MESSAGE(100, correctionFlex(currentStatus, configPage2, flexFuelTable), "correctionFlex");
  TEST_ASSERT_EQUAL_MESSAGE(100, correctionFuelTemp(), "correctionFuelTemp");
  TEST_ASSERT_EQUAL_MESSAGE(100, correctionLaunch(currentStatus, configPage6), "correctionLaunch");
  TEST_ASSERT_FALSE(correctionDFCO(currentStatus, configPage2, configPage4));
  TEST_ASSERT_EQUAL_MESSAGE(100, correctionDFCOfuel(currentStatus, configPage9), "correctionDFCOfuel");

  // Acceeleration
  configPage2.aeApplyMode = AE_MODE_MULTIPLIER;
  currentStatus.TPSlast = 0;
  currentStatus.TPS = 50; //25% actual value

  reset_AE(currentStatus);
  TEST_ASSERT_EQUAL(232U, correctionsFuel());

  configPage2.aeApplyMode = AE_MODE_ADDER;
  currentStatus.TPSlast = 0;
  currentStatus.TPS = 50;
  reset_AE(currentStatus);
  TEST_ASSERT_EQUAL(100U, correctionsFuel());

  // Deceeleration
  configPage2.aeApplyMode = AE_MODE_MULTIPLIER;
  currentStatus.TPSlast = 50;
  currentStatus.TPS = 45; 
  reset_AE(currentStatus);
  TEST_ASSERT_EQUAL(configPage2.decelAmount, correctionsFuel());
  TEST_ASSERT_LESS_THAN(0U, currentStatus.tpsDOT); 

  configPage2.aeApplyMode = AE_MODE_ADDER;
  currentStatus.TPSlast = 50;
  currentStatus.TPS = 45;
  reset_AE(currentStatus);
  TEST_ASSERT_EQUAL(configPage2.decelAmount, correctionsFuel());
  TEST_ASSERT_LESS_THAN(0U, currentStatus.tpsDOT); 
}

static void test_corrections_correctionsFuel_clip_limit(void) {
  construct2dTables();
  initialiseCorrections();

  // setup_TAE(currentStatus, configPage2, taeTable);

  populate_2dtable(&injectorVCorrectionTable, 255, 100);
  populate_2dtable(&baroFuelTable, 255, 100);
  populate_2dtable(&IATDensityCorrectionTable, 255, 100);
  populate_2dtable(&flexFuelTable, 255, 100);
  populate_2dtable(&fuelTempTable, 255, 100);

  LOOP_TIMER = 0;
  BIT_SET(LOOP_TIMER, IAT_READ_TIMER_BIT);
  BIT_SET(LOOP_TIMER, BARO_READ_TIMER_BIT);

  configPage2.flexEnabled = 1;
  configPage2.battVCorMode = BATTV_COR_MODE_WHOLE;
  configPage2.dfcoEnabled = 0;
  configPage2.aseTaperTime = 0U;
  configPage2.taeThresh = UINT8_MAX;
  configPage2.taeMinChange = UINT8_MAX;
  configPage2.aeMode = AE_MODE_TPS; //Set AE to TPS
  currentStatus.coolant = 212;
  currentStatus.runSecs = 255; 
  currentStatus.battery10 = 100;  
  currentStatus.IAT = toWorkingTemperature(100);
  currentStatus.baro = 100;
  currentStatus.ethanolPct = 100;
  BIT_CLEAR(currentStatus.status2, BIT_STATUS2_HLAUNCH);
  BIT_CLEAR(currentStatus.status2, BIT_STATUS2_SLAUNCH);
  currentStatus.AEamount = 100U;
  currentStatus.ASEValue = 100U;
  currentStatus.TPSlast = 0;
  currentStatus.TPS = currentStatus.TPSlast;
  currentStatus.AEamount = 100;

  configPage4.wueBins[9] = 100;
  configPage2.wueValues[9] = 100; //Use a value other than 100 here to ensure we are using the non-default value

  TEST_ASSERT_EQUAL_MESSAGE(100, correctionWUE(currentStatus, WUETable), "correctionWUE");
  TEST_ASSERT_EQUAL_MESSAGE(100, correctionASE(currentStatus, ASECountTable, ASETable, configPage2), "correctionASE");
  TEST_ASSERT_EQUAL_MESSAGE(100, correctionCranking(currentStatus, crankingEnrichTable, configPage10), "correctionCranking");
  TEST_ASSERT_EQUAL_MESSAGE(100, correctionAccel(currentStatus, configPage2, taeTable, maeTable), "correctionAccel");
  TEST_ASSERT_EQUAL_MESSAGE(100, correctionFloodClear(currentStatus, configPage4), "correctionFloodClear");
  TEST_ASSERT_EQUAL_MESSAGE(100, correctionAFRClosedLoop(), "correctionAFRClosedLoop");
  TEST_ASSERT_EQUAL_MESSAGE(255, correctionBatVoltage(currentStatus, injectorVCorrectionTable, configPage2), "correctionBatVoltage");
  TEST_ASSERT_EQUAL_MESSAGE(255, correctionIATDensity(currentStatus, IATDensityCorrectionTable), "correctionIATDensity");
  TEST_ASSERT_EQUAL_MESSAGE(255, correctionBaro(currentStatus, baroFuelTable), "correctionBaro");
  TEST_ASSERT_EQUAL_MESSAGE(255, correctionFlex(currentStatus, configPage2, flexFuelTable), "correctionFlex");
  TEST_ASSERT_EQUAL_MESSAGE(255, correctionFuelTemp(), "correctionFuelTemp");
  TEST_ASSERT_EQUAL_MESSAGE(100, correctionLaunch(currentStatus, configPage6), "correctionLaunch");
  TEST_ASSERT_FALSE(correctionDFCO(currentStatus, configPage2, configPage4));
  TEST_ASSERT_EQUAL_MESSAGE(100, correctionDFCOfuel(currentStatus, configPage9), "correctionDFCOfuel");

  TEST_ASSERT_EQUAL(1500U, correctionsFuel());
}

static void test_corrections_correctionsFuel(void) {
  RUN_TEST_P(test_corrections_correctionsFuel_ae_modes);
  RUN_TEST_P(test_corrections_correctionsFuel_clip_limit);
}

void testCorrections()
{
  SET_UNITY_FILENAME() {
    test_corrections_WUE();
    test_corrections_dfco();
    test_corrections_TAE(); //TPS based accel enrichment corrections
    test_corrections_MAE(); //MAP based accel enrichment corrections
    test_corrections_cranking();
    test_corrections_ASE();
    test_corrections_floodclear();
    test_corrections_bat();
    test_corrections_launch();
    test_corrections_flex();
    test_corrections_fueltemp();
    test_corrections_afrtarget();
    test_corrections_closedloop();
    test_corrections_correctionsFuel();
    test_corrections_baro();
  }
}