#include <unity.h>
#include "globals.h"
#include "corrections.h"
#include "idle.h"
#include "../test_utils.h"
#include "sensors.h"
#include "scale_translate.h"

extern int8_t correctionFixedTiming(int8_t advance, const config2 &page2, const config4 &page4);

static void test_correctionFixedTiming_inactive(void) {
    config2 page2;
    config4 page4;
    page2.fixAngEnable = 0;
    page4.FixAng = 13;

    TEST_ASSERT_EQUAL(8, correctionFixedTiming(8, page2, page4));
    TEST_ASSERT_EQUAL(-3, correctionFixedTiming(-3, page2, page4));
}

static void test_correctionFixedTiming_active(void) {
    config2 page2;
    config4 page4;
    page2.fixAngEnable = 1;
    page4.FixAng = 13;

    TEST_ASSERT_EQUAL(page4.FixAng, correctionFixedTiming(8, page2, page4));
    TEST_ASSERT_EQUAL(page4.FixAng, correctionFixedTiming(-3, page2, page4));
}

static void test_correctionFixedTiming(void) {
    RUN_TEST_P(test_correctionFixedTiming_inactive);
    RUN_TEST_P(test_correctionFixedTiming_active);
}


extern int8_t correctionCLTadvance(int8_t advance, const statuses &current, const table2D &cltAdvanceLUT);

struct cltadv_test_data_t {
    statuses current;
    test_2dtable_t<6> cltAdvLookUp;
};

static void setup_clt_advance_table(cltadv_test_data_t &testData) {
  initialiseIgnCorrections(testData.current);
  LOOP_TIMER = 0;
  BIT_SET(LOOP_TIMER, CLT_READ_TIMER_BIT);
  TEST_DATA_P uint8_t bins[] = { 60, 70, 80, 90, 100, 110 };
  TEST_DATA_P uint8_t values[] = { 30, 25, 20, 15, 10, 5 };
  populate_2dtable_P(&testData.cltAdvLookUp.lookupTable, values, bins);
}

static void test_correctionCLTadvance_lookup(void) {
    cltadv_test_data_t testData;
    setup_clt_advance_table(testData);

    testData.current.coolant = toWorkingTemperature(105);
    TEST_ASSERT_EQUAL(8 + 8 - 15, correctionCLTadvance(8, testData.current, testData.cltAdvLookUp.lookupTable));

    testData.current.coolant = toWorkingTemperature(65);
    TEST_ASSERT_EQUAL(1 + 28 - 15, correctionCLTadvance(1, testData.current, testData.cltAdvLookUp.lookupTable));

    testData.current.coolant = toWorkingTemperature(105);
    TEST_ASSERT_EQUAL(-3 + 8 - 15, correctionCLTadvance(-3, testData.current, testData.cltAdvLookUp.lookupTable));
}

static void test_correctionCLTadvance(void) {
    RUN_TEST_P(test_correctionCLTadvance_lookup);
}

struct crank_fixed_timing_t : public cltadv_test_data_t {
    config2 page2;
    config4 page4;
};

static void test_correctionCrankingFixedTiming_nocrank_inactive(void) {
    crank_fixed_timing_t testData;
    setup_clt_advance_table(testData);
    BIT_CLEAR(testData.current.engine, BIT_ENGINE_CRANK);
    testData.page2.crkngAddCLTAdv = 0;
    testData.page4.CrankAng = 8;

    TEST_ASSERT_EQUAL(-7, correctionCrankingFixedTiming(-7, testData.current, testData.page2, testData.page4, testData.cltAdvLookUp.lookupTable));
}

static void test_correctionCrankingFixedTiming_crank_fixed(void) {
    crank_fixed_timing_t testData;
    setup_clt_advance_table(testData);
    BIT_SET(testData.current.engine, BIT_ENGINE_CRANK);
    testData.page2.crkngAddCLTAdv = 0;

    testData.page4.CrankAng = 8;
    TEST_ASSERT_EQUAL(testData.page4.CrankAng, correctionCrankingFixedTiming(-7, testData.current, testData.page2, testData.page4, testData.cltAdvLookUp.lookupTable));

    testData.page4.CrankAng = -8;
    TEST_ASSERT_EQUAL(testData.page4.CrankAng, correctionCrankingFixedTiming(-7, testData.current, testData.page2, testData.page4, testData.cltAdvLookUp.lookupTable));
}

static void test_correctionCrankingFixedTiming_crank_coolant(void) {
    crank_fixed_timing_t testData;
    setup_clt_advance_table(testData);
    BIT_SET(testData.current.engine, BIT_ENGINE_CRANK);
    testData.page2.crkngAddCLTAdv = 1;
    
    testData.page4.CrankAng = 8;
    testData.current.coolant = toWorkingTemperature(65);
    TEST_ASSERT_EQUAL(1 + 28 - 15, correctionCLTadvance(1, testData.current, testData.cltAdvLookUp.lookupTable));
}

static void test_correctionCrankingFixedTiming(void) {
    RUN_TEST_P(test_correctionCrankingFixedTiming_nocrank_inactive);
    RUN_TEST_P(test_correctionCrankingFixedTiming_crank_fixed);
    RUN_TEST_P(test_correctionCrankingFixedTiming_crank_coolant);
}

extern int8_t correctionFlexTiming(int8_t advance, statuses &current, const config2 &page2, const table2D &lookupTable);

struct flex_adv_t {
    statuses current;
    config2 page2;
    test_2dtable_t<6> flexAdvLUT; 
};

static void setup_flexAdv(flex_adv_t &testData) {
  initialiseIgnCorrections(testData.current);
  TEST_DATA_P uint8_t bins[] = { 30, 40, 50, 60, 70, 80 };
  TEST_DATA_P uint8_t values[] = { 30, 25, 20, 15, 10, 5 };
  populate_2dtable_P(&testData.flexAdvLUT.lookupTable, values, bins);

  testData.page2.flexEnabled = 1;
  testData.current.ethanolPct = 55;
}

static void test_correctionFlexTiming_inactive(void) {
    flex_adv_t testData;
    setup_flexAdv(testData);
    testData.page2.flexEnabled = 0;

    TEST_ASSERT_EQUAL(-7, correctionFlexTiming(-7, testData.current, testData.page2, testData.flexAdvLUT.lookupTable));
    TEST_ASSERT_EQUAL(3, correctionFlexTiming(3, testData.current, testData.page2, testData.flexAdvLUT.lookupTable));
}

static void test_correctionFlexTiming_table_lookup(void) {
    flex_adv_t testData;
    setup_flexAdv(testData);

    TEST_ASSERT_EQUAL(8 + 18 - OFFSET_IGNITION, correctionFlexTiming(8, testData.current, testData.page2, testData.flexAdvLUT.lookupTable));
    TEST_ASSERT_EQUAL(18 - OFFSET_IGNITION, testData.current.flexIgnCorrection);    

    testData.current.ethanolPct = 35;
    TEST_ASSERT_EQUAL(-4 + 28 - OFFSET_IGNITION, correctionFlexTiming(-4, testData.current, testData.page2, testData.flexAdvLUT.lookupTable));
    TEST_ASSERT_EQUAL(28 - OFFSET_IGNITION, testData.current.flexIgnCorrection);    
}

static void test_correctionFlexTiming(void) {
    RUN_TEST_P(test_correctionFlexTiming_inactive);
    RUN_TEST_P(test_correctionFlexTiming_table_lookup);
}

extern int8_t correctionWMITiming(int8_t advance, const statuses &current, const config10 &page10, const table2D &lookupTable);

struct wmi_advance_t {
    statuses current;
    config10 page10;
    test_2dtable_t<6> flexAdvLUT;
};

static void setup_WMIAdv(wmi_advance_t &testData) {
    initialiseIgnCorrections(testData.current);

    testData.page10.wmiEnabled= 1;
    testData.page10.wmiAdvEnabled = 1;
    BIT_CLEAR(testData.current.status4, BIT_STATUS4_WMI_EMPTY);
    testData.page10.wmiTPS = 50;
    testData.current.TPS = testData.page10.wmiTPS + 1;
    testData.page10.wmiRPM = 30;
    testData.current.RPM = testData.page10.wmiRPM + 1U;
    testData.page10.wmiMAP = 35;
    testData.current.MAP = (testData.page10.wmiMAP*2L)+1L;
    testData.page10.wmiIAT = 155;
    testData.current.IAT = toWorkingTemperature(testData.page10.wmiIAT) + 1;

    TEST_DATA_P uint8_t bins[] = { 30, 40, 50, 60, 70, 80 };
    TEST_DATA_P uint8_t values[] = { 30, 25, 20, 15, 10, 5 };
    populate_2dtable_P(&testData.flexAdvLUT.lookupTable, values, bins);
}

static void test_correctionWMITiming_table_lookup(void) {
    wmi_advance_t testData;
    setup_WMIAdv(testData);

    testData.current.MAP = (55*2U)+1U;
    TEST_ASSERT_EQUAL(8 + 18 - OFFSET_IGNITION, correctionWMITiming(8, testData.current, testData.page10, testData.flexAdvLUT.lookupTable));

    testData.current.MAP = (35*2U)+1U;
    TEST_ASSERT_EQUAL(-4 + 28 - OFFSET_IGNITION, correctionWMITiming(-4, testData.current, testData.page10, testData.flexAdvLUT.lookupTable));
}

static void test_correctionWMITiming_wmidisabled_inactive(void) {
    wmi_advance_t testData;
    setup_WMIAdv(testData);
    testData.page10.wmiEnabled= 0;

    TEST_ASSERT_EQUAL(8, correctionWMITiming(8, testData.current, testData.page10, testData.flexAdvLUT.lookupTable));
    TEST_ASSERT_EQUAL(-3, correctionWMITiming(-3, testData.current, testData.page10, testData.flexAdvLUT.lookupTable));
}


static void test_correctionWMITiming_wmiadvdisabled_inactive(void) {
    wmi_advance_t testData;
    setup_WMIAdv(testData);
    testData.page10.wmiAdvEnabled = 0;

    TEST_ASSERT_EQUAL(8, correctionWMITiming(8, testData.current, testData.page10, testData.flexAdvLUT.lookupTable));
    TEST_ASSERT_EQUAL(-3, correctionWMITiming(-3, testData.current, testData.page10, testData.flexAdvLUT.lookupTable));
}

static void test_correctionWMITiming_empty_inactive(void) {
    wmi_advance_t testData;
    setup_WMIAdv(testData);
    BIT_SET(testData.current.status4, BIT_STATUS4_WMI_EMPTY);

    TEST_ASSERT_EQUAL(8, correctionWMITiming(8, testData.current, testData.page10, testData.flexAdvLUT.lookupTable));
    TEST_ASSERT_EQUAL(-3, correctionWMITiming(-3, testData.current, testData.page10, testData.flexAdvLUT.lookupTable));
}

static void test_correctionWMITiming_tpslow_inactive(void) {
    wmi_advance_t testData;
    setup_WMIAdv(testData);
    testData.current.TPS = testData.page10.wmiTPS - 1;

    TEST_ASSERT_EQUAL(8, correctionWMITiming(8, testData.current, testData.page10, testData.flexAdvLUT.lookupTable));
    TEST_ASSERT_EQUAL(-3, correctionWMITiming(-3, testData.current, testData.page10, testData.flexAdvLUT.lookupTable));
}

static void test_correctionWMITiming_rpmlow_inactive(void) {
    wmi_advance_t testData;
    setup_WMIAdv(testData);
    testData.current.RPM = testData.page10.wmiRPM - 1U;

    TEST_ASSERT_EQUAL(8, correctionWMITiming(8, testData.current, testData.page10, testData.flexAdvLUT.lookupTable));
    TEST_ASSERT_EQUAL(-3, correctionWMITiming(-3, testData.current, testData.page10, testData.flexAdvLUT.lookupTable));
}
   
static void test_correctionWMITiming_maplow_inactive(void) {
    wmi_advance_t testData;
    setup_WMIAdv(testData);
    testData.current.MAP = (testData.page10.wmiMAP*2)-1;

    TEST_ASSERT_EQUAL(8, correctionWMITiming(8, testData.current, testData.page10, testData.flexAdvLUT.lookupTable));
    TEST_ASSERT_EQUAL(-3, correctionWMITiming(-3, testData.current, testData.page10, testData.flexAdvLUT.lookupTable));
}
    
static void test_correctionWMITiming_iatlow_inactive(void) {
    wmi_advance_t testData;
    setup_WMIAdv(testData);
    testData.current.IAT = toWorkingTemperature(testData.page10.wmiIAT) - 1;

    TEST_ASSERT_EQUAL(8, correctionWMITiming(8, testData.current, testData.page10, testData.flexAdvLUT.lookupTable));
    TEST_ASSERT_EQUAL(-3, correctionWMITiming(-3, testData.current, testData.page10, testData.flexAdvLUT.lookupTable));
}   

static void test_correctionWMITiming(void) {
    RUN_TEST_P(test_correctionWMITiming_table_lookup);
    RUN_TEST_P(test_correctionWMITiming_tpslow_inactive);
    RUN_TEST_P(test_correctionWMITiming_wmidisabled_inactive);
    RUN_TEST_P(test_correctionWMITiming_wmiadvdisabled_inactive);
    RUN_TEST_P(test_correctionWMITiming_empty_inactive);
    RUN_TEST_P(test_correctionWMITiming_tpslow_inactive);
    RUN_TEST_P(test_correctionWMITiming_rpmlow_inactive);
    RUN_TEST_P(test_correctionWMITiming_maplow_inactive);
    RUN_TEST_P(test_correctionWMITiming_iatlow_inactive);
}

extern int8_t correctionIATretard(int8_t advance, const statuses &current, table2D &lookupTable);

struct iat_test_data_t {
    statuses current;
    test_2dtable_t<6> iatAdvLUT;
};

static void setup_IATRetard(iat_test_data_t &testData) {
  initialiseIgnCorrections(testData.current);
  LOOP_TIMER = 0;
  BIT_SET(LOOP_TIMER, IAT_READ_TIMER_BIT);
  TEST_DATA_P uint8_t bins[] = { 30, 40, 50, 60, 70, 80 };
  TEST_DATA_P uint8_t values[] = { 30, 25, 20, 15, 10, 5 };
  populate_2dtable_P(&testData.iatAdvLUT.lookupTable, values, bins);

  testData.current.IAT = 75;
}

static void test_correctionIATretard_table_lookup(void) {
    iat_test_data_t testData;
    setup_IATRetard(testData);

    testData.current.IAT = 75;
    TEST_ASSERT_EQUAL(-11-8, correctionIATretard(-11, testData.current, testData.iatAdvLUT.lookupTable));

    testData.current.IAT = 45;
    TEST_ASSERT_EQUAL(-11-23, correctionIATretard(-11, testData.current, testData.iatAdvLUT.lookupTable));
}

static void test_correctionIATretard(void) {
    RUN_TEST_P(test_correctionIATretard_table_lookup);
}

extern int8_t correctionIdleAdvance(int8_t advance, const statuses &current, const config2 &page2, const config6 &page6, const config9 &page9, table2D &lookupTable);

struct idle_advance_test_data_t {
    statuses current;
    config2 page2;
    config6 page6;
    config9 page9;
    test_2dtable_t<6> lookupTable;
};

static void setup_idleadv_tps(idle_advance_test_data_t &testData) {
    testData.page2.idleAdvAlgorithm = IDLEADVANCE_ALGO_TPS;
    testData.page2.idleAdvTPS = 30;
    testData.current.TPS = testData.page2.idleAdvTPS - 1U;
}

static void setup_idleadv_ctps(idle_advance_test_data_t &testData) {
    testData.page2.idleAdvAlgorithm = IDLEADVANCE_ALGO_CTPS;
    testData.current.CTPSActive = 1;
}

static void setup_correctionIdleAdvance(idle_advance_test_data_t &testData) {
    initialiseIgnCorrections(testData.current);

    TEST_DATA_P uint8_t bins[] = { 30, 40, 50, 60, 70, 80 };
    TEST_DATA_P uint8_t values[] = { 30, 25, 20, 15, 10, 5 };
    populate_2dtable_P(&testData.lookupTable.lookupTable, values, bins);
  
    testData.page2.idleAdvEnabled = IDLEADVANCE_MODE_ADDED;
    testData.page2.idleAdvDelay = 5;
    testData.page2.idleAdvRPM = 20;
    testData.page2.vssMode = VSS_MODE_OFF;
    testData.page6.iacAlgorithm = IAC_ALGORITHM_NONE;
    testData.page9.idleAdvStartDelay = 0U;

    runSecsX10 = testData.page2.idleAdvDelay * 5;
    BIT_SET(testData.current.engine, BIT_ENGINE_RUN);
    // int idleRPMdelta = (testData.current.CLIdleTarget - (testData.current.RPM / 10) ) + 50;
    testData.current.CLIdleTarget = 100;
    testData.current.RPM = (testData.page2.idleAdvRPM * 100) - 1U;
    
    setup_idleadv_tps(testData);
    // Run once to initialise internal state
    correctionIdleAdvance(8, testData.current, testData.page2, testData.page6, testData.page9, testData.lookupTable.lookupTable);
}

static void assert_correctionIdleAdvance(int8_t advance, uint8_t expectedLookupValue, idle_advance_test_data_t &testData) {
    testData.page2.idleAdvEnabled = IDLEADVANCE_MODE_ADDED;
    TEST_ASSERT_EQUAL(advance + expectedLookupValue - 15, correctionIdleAdvance(advance, testData.current, testData.page2, testData.page6, testData.page9, testData.lookupTable.lookupTable));

    testData.page2.idleAdvEnabled = IDLEADVANCE_MODE_SWITCHED;
    TEST_ASSERT_EQUAL(expectedLookupValue - 15, correctionIdleAdvance(advance, testData.current, testData.page2, testData.page6, testData.page9, testData.lookupTable.lookupTable));
}

static void test_correctionIdleAdvance_tps_lookup_nodelay(void) {
    idle_advance_test_data_t testData;
    setup_correctionIdleAdvance(testData);

    setup_idleadv_tps(testData);

    testData.current.RPM = (testData.current.CLIdleTarget * 10) + 100;
    assert_correctionIdleAdvance(8, 25, testData);

    testData.current.RPM = (testData.current.CLIdleTarget * 10) - 100;
    assert_correctionIdleAdvance(-3, 15, testData);
}

static void test_correctionIdleAdvance_ctps_lookup_nodelay(void) {
    idle_advance_test_data_t testData;
    setup_correctionIdleAdvance(testData);

    setup_idleadv_ctps(testData);

    testData.current.RPM = (testData.current.CLIdleTarget * 10) + 100;
    assert_correctionIdleAdvance(8, 25, testData);

    testData.current.RPM = (testData.current.CLIdleTarget * 10) - 100;
    assert_correctionIdleAdvance(-3, 15, testData);
}

static void test_correctionIdleAdvance_inactive_notrunning(void) {
    idle_advance_test_data_t testData;
    setup_correctionIdleAdvance(testData);
    
    TEST_ASSERT_EQUAL(23, correctionIdleAdvance(8, testData.current, testData.page2, testData.page6, testData.page9, testData.lookupTable.lookupTable));
    BIT_CLEAR(testData.current.engine, BIT_ENGINE_RUN);
    TEST_ASSERT_EQUAL(8, correctionIdleAdvance(8, testData.current, testData.page2, testData.page6, testData.page9, testData.lookupTable.lookupTable));
}

static void test_correctionIdleAdvance_noadvance_modeoff(void) {
    idle_advance_test_data_t testData;
    setup_correctionIdleAdvance(testData);
    TEST_ASSERT_EQUAL(23, correctionIdleAdvance(8, testData.current, testData.page2, testData.page6, testData.page9, testData.lookupTable.lookupTable));
    testData.page2.idleAdvEnabled = IDLEADVANCE_MODE_OFF;
    TEST_ASSERT_EQUAL(8, correctionIdleAdvance(8, testData.current, testData.page2, testData.page6, testData.page9, testData.lookupTable.lookupTable));
}

static void test_correctionIdleAdvance_noadvance_rpmtoohigh(void) {
    idle_advance_test_data_t testData;
    setup_correctionIdleAdvance(testData);
    TEST_ASSERT_EQUAL(23, correctionIdleAdvance(8, testData.current, testData.page2, testData.page6, testData.page9, testData.lookupTable.lookupTable));
    testData.current.RPM = (testData.page2.idleAdvRPM * 100)+1;
    TEST_ASSERT_EQUAL(8, correctionIdleAdvance(8, testData.current, testData.page2, testData.page6, testData.page9, testData.lookupTable.lookupTable));
}

static void test_correctionIdleAdvance_noadvance_vsslimit(void) {
    idle_advance_test_data_t testData;
    setup_correctionIdleAdvance(testData);
    TEST_ASSERT_EQUAL(23, correctionIdleAdvance(8, testData.current, testData.page2, testData.page6, testData.page9, testData.lookupTable.lookupTable));
    testData.page2.vssMode = VSS_MODE_INTERNAL_PIN;
    testData.page2.idleAdvVss = 15;
    testData.current.vss = testData.page2.idleAdvVss + 1;
    TEST_ASSERT_EQUAL(8, correctionIdleAdvance(8, testData.current, testData.page2, testData.page6, testData.page9, testData.lookupTable.lookupTable));
}

static void test_correctionIdleAdvance_noadvance_tpslimit(void) {
    idle_advance_test_data_t testData;
    setup_correctionIdleAdvance(testData);
    setup_idleadv_tps(testData);
    TEST_ASSERT_EQUAL(23, correctionIdleAdvance(8, testData.current, testData.page2, testData.page6, testData.page9, testData.lookupTable.lookupTable));
    testData.current.TPS = testData.page2.idleAdvTPS + 1U;
    TEST_ASSERT_EQUAL(8, correctionIdleAdvance(8, testData.current, testData.page2, testData.page6, testData.page9, testData.lookupTable.lookupTable));
}

static void test_correctionIdleAdvance_noadvance_ctpsinactive(void) {
    idle_advance_test_data_t testData;
    setup_correctionIdleAdvance(testData);
    setup_idleadv_ctps(testData);
    TEST_ASSERT_EQUAL(23, correctionIdleAdvance(8, testData.current, testData.page2, testData.page6, testData.page9, testData.lookupTable.lookupTable));
    testData.current.CTPSActive = 0;
    TEST_ASSERT_EQUAL(8, correctionIdleAdvance(8, testData.current, testData.page2, testData.page6, testData.page9, testData.lookupTable.lookupTable));
}

static void test_correctionIdleAdvance_noadvance_rundelay(void) {
    idle_advance_test_data_t testData;
    setup_correctionIdleAdvance(testData);
    TEST_ASSERT_EQUAL(23, correctionIdleAdvance(8, testData.current, testData.page2, testData.page6, testData.page9, testData.lookupTable.lookupTable));
    runSecsX10 = (testData.page2.idleAdvDelay * 5)-1;
    TEST_ASSERT_EQUAL(8, correctionIdleAdvance(8, testData.current, testData.page2, testData.page6, testData.page9, testData.lookupTable.lookupTable));
}

static void test_correctionIdleAdvance_delay(void) {
    idle_advance_test_data_t testData;
    setup_correctionIdleAdvance(testData);
    testData.page9.idleAdvStartDelay = 3;
    BIT_SET(LOOP_TIMER, BIT_TIMER_10HZ);
    TEST_ASSERT_EQUAL(8, correctionIdleAdvance(8, testData.current, testData.page2, testData.page6, testData.page9, testData.lookupTable.lookupTable));
    TEST_ASSERT_EQUAL(8, correctionIdleAdvance(8, testData.current, testData.page2, testData.page6, testData.page9, testData.lookupTable.lookupTable));
    TEST_ASSERT_EQUAL(8, correctionIdleAdvance(8, testData.current, testData.page2, testData.page6, testData.page9, testData.lookupTable.lookupTable));
    TEST_ASSERT_EQUAL(23, correctionIdleAdvance(8, testData.current, testData.page2, testData.page6, testData.page9, testData.lookupTable.lookupTable));
}

static void test_correctionIdleAdvance(void) {
    RUN_TEST_P(test_correctionIdleAdvance_tps_lookup_nodelay);
    RUN_TEST_P(test_correctionIdleAdvance_ctps_lookup_nodelay);
    RUN_TEST_P(test_correctionIdleAdvance_inactive_notrunning);
    RUN_TEST_P(test_correctionIdleAdvance_noadvance_modeoff);
    RUN_TEST_P(test_correctionIdleAdvance_noadvance_rpmtoohigh);
    RUN_TEST_P(test_correctionIdleAdvance_noadvance_vsslimit);
    RUN_TEST_P(test_correctionIdleAdvance_noadvance_tpslimit);
    RUN_TEST_P(test_correctionIdleAdvance_noadvance_ctpsinactive);
    RUN_TEST_P(test_correctionIdleAdvance_noadvance_rundelay);
    RUN_TEST_P(test_correctionIdleAdvance_delay);
}

extern int8_t correctionSoftRevLimit(int8_t advance, statuses &current, const config2 &page2, const config4 &page4, const config6 &page6);

struct soft_revlimit_testdata {
    statuses current;
    config2 page2;
    config4 page4;
    config6 page6;
};

static void setup_correctionSoftRevLimit(soft_revlimit_testdata &testData) {
    initialiseIgnCorrections(testData.current);

    testData.page6.engineProtectType = PROTECT_CUT_IGN;
    testData.page4.SoftRevLim = 50;
    testData.page4.SoftLimMax = 1;
    testData.page4.SoftLimRetard = 5;
    testData.page2.SoftLimitMode = SOFT_LIMIT_FIXED;

    testData.current.RPMdiv100 = testData.page4.SoftRevLim + 1;
    softLimitTime = 0;

    BIT_CLEAR(LOOP_TIMER, BIT_TIMER_10HZ);
}

static void assert_correctionSoftRevLimit(int8_t advance, soft_revlimit_testdata &testData) {
    testData.page2.SoftLimitMode = SOFT_LIMIT_FIXED;
    TEST_ASSERT_EQUAL(testData.page4.SoftLimRetard, correctionSoftRevLimit(advance, testData.current, testData.page2, testData.page4, testData.page6));
    TEST_ASSERT_BIT_HIGH(BIT_STATUS2_SFTLIM, testData.current.status2);

    BIT_CLEAR(testData.current.status2, BIT_STATUS2_SFTLIM);
    testData.page2.SoftLimitMode = SOFT_LIMIT_RELATIVE;
    TEST_ASSERT_EQUAL(advance-testData.page4.SoftLimRetard, correctionSoftRevLimit(advance, testData.current, testData.page2, testData.page4, testData.page6));
    TEST_ASSERT_BIT_HIGH(BIT_STATUS2_SFTLIM, testData.current.status2);
}

static void test_correctionSoftRevLimit_modes(void) {
    soft_revlimit_testdata testData;
    setup_correctionSoftRevLimit(testData);

    assert_correctionSoftRevLimit(8, testData);
    assert_correctionSoftRevLimit(-3, testData);
}

static void test_correctionSoftRevLimit_inactive_protecttype(void) {
    soft_revlimit_testdata testData;
    setup_correctionSoftRevLimit(testData);

    testData.page6.engineProtectType = PROTECT_CUT_OFF;
    BIT_SET(testData.current.status2, BIT_STATUS2_SFTLIM);
    TEST_ASSERT_EQUAL(8, correctionSoftRevLimit(8, testData.current, testData.page2, testData.page4, testData.page6));
    TEST_ASSERT_BIT_LOW(BIT_STATUS2_SFTLIM, testData.current.status2);

    testData.page6.engineProtectType = PROTECT_CUT_FUEL;
    BIT_SET(testData.current.status2, BIT_STATUS2_SFTLIM);
    TEST_ASSERT_EQUAL(8, correctionSoftRevLimit(8, testData.current, testData.page2, testData.page4, testData.page6));
    TEST_ASSERT_BIT_LOW(BIT_STATUS2_SFTLIM, testData.current.status2);
}

static void test_correctionSoftRevLimit_inactive_rpmtoohigh(void) {
    soft_revlimit_testdata testData;
    setup_correctionSoftRevLimit(testData);
    assert_correctionSoftRevLimit(8, testData);

    testData.current.RPMdiv100 = testData.page4.SoftRevLim-1;
    BIT_SET(testData.current.status2, BIT_STATUS2_SFTLIM);
    TEST_ASSERT_EQUAL(8, correctionSoftRevLimit(8, testData.current, testData.page2, testData.page4, testData.page6));
    TEST_ASSERT_BIT_LOW(BIT_STATUS2_SFTLIM, testData.current.status2);
}

static void test_correctionSoftRevLimit_timeout(void) {
    soft_revlimit_testdata testData;
    setup_correctionSoftRevLimit(testData);

    testData.page4.SoftLimMax = 3;
    testData.page2.SoftLimitMode = SOFT_LIMIT_RELATIVE;
    BIT_SET(LOOP_TIMER, BIT_TIMER_10HZ);
    TEST_ASSERT_EQUAL(8-testData.page4.SoftLimRetard, correctionSoftRevLimit(8, testData.current, testData.page2, testData.page4, testData.page6));
    TEST_ASSERT_EQUAL(-5-testData.page4.SoftLimRetard, correctionSoftRevLimit(-5, testData.current, testData.page2, testData.page4, testData.page6));
    TEST_ASSERT_EQUAL(23-testData.page4.SoftLimRetard, correctionSoftRevLimit(23, testData.current, testData.page2, testData.page4, testData.page6));
    TEST_ASSERT_EQUAL(-21, correctionSoftRevLimit(-21, testData.current, testData.page2, testData.page4, testData.page6));
    TEST_ASSERT_EQUAL(8, correctionSoftRevLimit(8, testData.current, testData.page2, testData.page4, testData.page6));
    TEST_ASSERT_EQUAL(0, correctionSoftRevLimit(0, testData.current, testData.page2, testData.page4, testData.page6));
}

static void test_correctionSoftRevLimit(void) {
    RUN_TEST_P(test_correctionSoftRevLimit_modes);
    RUN_TEST_P(test_correctionSoftRevLimit_inactive_protecttype);
    RUN_TEST_P(test_correctionSoftRevLimit_inactive_rpmtoohigh);
    RUN_TEST_P(test_correctionSoftRevLimit_timeout);
}

extern int8_t correctionNitrous(int8_t advance, const statuses &current, const config10 &page10);

static void test_correctionNitrous_disabled(void) {
    statuses current;
    config10 page10;
    page10.n2o_enable = 0;
    TEST_ASSERT_EQUAL(13, correctionNitrous(13, current, page10));
    TEST_ASSERT_EQUAL(-13, correctionNitrous(-13, current, page10));
}

static void test_correctionNitrous_stage1(void) {
    statuses current;
    config10 page10;
    page10.n2o_enable = 1;
    page10.n2o_stage1_retard = 5;
    page10.n2o_stage2_retard = 0;
    
    current.nitrous_status = NITROUS_STAGE1;
    TEST_ASSERT_EQUAL(8, correctionNitrous(13, current, page10));
    TEST_ASSERT_EQUAL(-18, correctionNitrous(-13, current, page10));
    
    current.nitrous_status = NITROUS_BOTH;
    TEST_ASSERT_EQUAL(8, correctionNitrous(13, current, page10));
    TEST_ASSERT_EQUAL(-18, correctionNitrous(-13, current, page10));
}

static void test_correctionNitrous_stage2(void) {
    statuses current;
    config10 page10;
    page10.n2o_enable = 1;
    page10.n2o_stage1_retard = 0;
    page10.n2o_stage2_retard = 5;
    
    current.nitrous_status = NITROUS_STAGE2;
    TEST_ASSERT_EQUAL(8, correctionNitrous(13, current, page10));
    TEST_ASSERT_EQUAL(-18, correctionNitrous(-13, current, page10));
    
    current.nitrous_status = NITROUS_BOTH;
    TEST_ASSERT_EQUAL(8, correctionNitrous(13, current, page10));
    TEST_ASSERT_EQUAL(-18, correctionNitrous(-13, current, page10));
}

static void test_correctionNitrous_stageboth(void) {
    statuses current;
    config10 page10;
    page10.n2o_enable = 1;
    page10.n2o_stage1_retard = 3;
    page10.n2o_stage2_retard = 5;
      
    current.nitrous_status = NITROUS_BOTH;
    TEST_ASSERT_EQUAL(5, correctionNitrous(13, current, page10));
    TEST_ASSERT_EQUAL(-21, correctionNitrous(-13, current, page10));
}

static void test_correctionNitrous(void) {
    RUN_TEST_P(test_correctionNitrous_disabled);
    RUN_TEST_P(test_correctionNitrous_stage1);
    RUN_TEST_P(test_correctionNitrous_stage2);
    RUN_TEST_P(test_correctionNitrous_stageboth);
}

extern int8_t correctionSoftLaunch(int8_t advance, statuses &current, const config6 &page6, const config10 &page10);

struct soft_launch_data_t {
    statuses current;
    config6 page6;
    config10 page10;
};

static void setup_correctionSoftLaunch(soft_launch_data_t &testData) {
    testData.page6.launchEnabled = 1;
    testData.page6.flatSArm = 20;
    testData.page6.lnchSoftLim = 40;
    testData.page10.lnchCtrlTPS = 80;
    
    testData.current.clutchTrigger = 1;
    testData.current.clutchEngagedRPM = ((testData.page6.flatSArm) * 100) - 100;
    testData.current.RPM = ((testData.page6.lnchSoftLim) * 100) + 100;
    testData.current.TPS = testData.page10.lnchCtrlTPS + 1;
}

static void test_correctionSoftLaunch_on(void) {
    soft_launch_data_t testData;
    setup_correctionSoftLaunch(testData);

    testData.page6.lnchRetard = -3;
    TEST_ASSERT_EQUAL(testData.page6.lnchRetard, correctionSoftLaunch(-8, testData.current, testData.page6, testData.page10));
    TEST_ASSERT_BIT_HIGH(BIT_STATUS2_SLAUNCH, testData.current.status2);

    testData.page6.lnchRetard = 3;
    BIT_CLEAR(testData.current.status2, BIT_STATUS2_SLAUNCH);
    TEST_ASSERT_EQUAL(testData.page6.lnchRetard, correctionSoftLaunch(8, testData.current, testData.page6, testData.page10));
    TEST_ASSERT_BIT_HIGH(BIT_STATUS2_SLAUNCH, testData.current.status2);
}

static void test_correctionSoftLaunch_off_disabled(void) {
    soft_launch_data_t testData;
    setup_correctionSoftLaunch(testData);
    testData.page6.launchEnabled = 0;
    testData.page6.lnchRetard = -3;

    TEST_ASSERT_EQUAL(-8, correctionSoftLaunch(-8, testData.current, testData.page6, testData.page10));
    TEST_ASSERT_BIT_LOW(BIT_STATUS2_SLAUNCH, testData.current.status2);
}

static void test_correctionSoftLaunch_off_noclutchtrigger(void) {
    soft_launch_data_t testData;
    setup_correctionSoftLaunch(testData);
    testData.current.clutchTrigger = 0;
    testData.page6.lnchRetard = -3;

    TEST_ASSERT_EQUAL(-8, correctionSoftLaunch(-8, testData.current, testData.page6, testData.page10));
    TEST_ASSERT_BIT_LOW(BIT_STATUS2_SLAUNCH, testData.current.status2);
}

static void test_correctionSoftLaunch_off_clutchrpmlow(void) {
    soft_launch_data_t testData;
    setup_correctionSoftLaunch(testData);
    testData.current.clutchEngagedRPM = (testData.page6.flatSArm * 100) + 1;
    testData.page6.lnchRetard = -3;

    TEST_ASSERT_EQUAL(-8, correctionSoftLaunch(-8, testData.current, testData.page6, testData.page10));
    TEST_ASSERT_BIT_LOW(BIT_STATUS2_SLAUNCH, testData.current.status2);
}

static void test_correctionSoftLaunch_off_rpmlimit(void) {
    soft_launch_data_t testData;
    setup_correctionSoftLaunch(testData);
    testData.current.RPM = (testData.page6.lnchSoftLim * 100) - 1;
    testData.page6.lnchRetard = -3;

    TEST_ASSERT_EQUAL(-8, correctionSoftLaunch(-8, testData.current, testData.page6, testData.page10));
    TEST_ASSERT_BIT_LOW(BIT_STATUS2_SLAUNCH, testData.current.status2);
}

static void test_correctionSoftLaunch_off_tpslow(void) {
    soft_launch_data_t testData;
    setup_correctionSoftLaunch(testData);
    testData.current.TPS = testData.page10.lnchCtrlTPS - 1;
    testData.page6.lnchRetard = -3;

    TEST_ASSERT_EQUAL(-8, correctionSoftLaunch(-8, testData.current, testData.page6, testData.page10));
    TEST_ASSERT_BIT_LOW(BIT_STATUS2_SLAUNCH, testData.current.status2);
}

static void test_correctionSoftLaunch(void) {
    RUN_TEST_P(test_correctionSoftLaunch_on);
    RUN_TEST_P(test_correctionSoftLaunch_off_disabled);
    RUN_TEST_P(test_correctionSoftLaunch_off_noclutchtrigger);
    RUN_TEST_P(test_correctionSoftLaunch_off_clutchrpmlow);
    RUN_TEST_P(test_correctionSoftLaunch_off_rpmlimit);
    RUN_TEST_P(test_correctionSoftLaunch_off_tpslow);
}

extern int8_t correctionSoftFlatShift(int8_t advance, statuses &current, const config6 &page6);

struct soft_flatshift_data_t {
    statuses current;
    config6 page6;
};

static void setup_correctionSoftFlatShift(soft_flatshift_data_t &testData) {
    testData.page6.flatSEnable = 1;
    testData.page6.flatSArm = 10;
    testData.page6.flatSSoftWin = 10;
    
    testData.current.clutchTrigger = 1;
    testData.current.clutchEngagedRPM = ((testData.page6.flatSArm) * 100) + 500;
    testData.current.RPM = testData.current.clutchEngagedRPM + 600;

    BIT_CLEAR(testData.current.status5, BIT_STATUS5_FLATSS);
}

static void test_correctionSoftFlatShift_on(void) {
    soft_flatshift_data_t testData;
    setup_correctionSoftFlatShift(testData);
    testData.page6.flatSRetard = -3;

    TEST_ASSERT_EQUAL(testData.page6.flatSRetard, correctionSoftFlatShift(-8, testData.current, testData.page6));
    TEST_ASSERT_BIT_HIGH(BIT_STATUS5_FLATSS, testData.current.status5);

    BIT_CLEAR(testData.current.status5, BIT_STATUS5_FLATSS);
    TEST_ASSERT_EQUAL(testData.page6.flatSRetard, correctionSoftFlatShift(3, testData.current, testData.page6));
    TEST_ASSERT_BIT_HIGH(BIT_STATUS5_FLATSS, testData.current.status5);
}

static void test_correctionSoftFlatShift_off_disabled(void) {
    soft_flatshift_data_t testData;
    setup_correctionSoftFlatShift(testData);
    testData.page6.flatSRetard = -3;
    testData.page6.flatSEnable = 0;

    BIT_SET(testData.current.status5, BIT_STATUS5_FLATSS);
    TEST_ASSERT_EQUAL(-8, correctionSoftFlatShift(-8, testData.current, testData.page6));
    TEST_ASSERT_BIT_LOW(BIT_STATUS5_FLATSS, testData.current.status5);
}

static void test_correctionSoftFlatShift_off_noclutchtrigger(void) {
    soft_flatshift_data_t testData;
    setup_correctionSoftFlatShift(testData);
    testData.page6.flatSRetard = -3;
    testData.current.clutchTrigger = 0;

    BIT_SET(testData.current.status5, BIT_STATUS5_FLATSS);
    TEST_ASSERT_EQUAL(-8, correctionSoftFlatShift(-8, testData.current, testData.page6));
    TEST_ASSERT_BIT_LOW(BIT_STATUS5_FLATSS, testData.current.status5);
}

static void test_correctionSoftFlatShift_off_clutchrpmtoolow(void) {
    soft_flatshift_data_t testData;
    setup_correctionSoftFlatShift(testData);
    testData.page6.flatSRetard = -3;
    testData.current.clutchEngagedRPM = ((testData.page6.flatSArm) * 100) - 500;

    BIT_SET(testData.current.status5, BIT_STATUS5_FLATSS);
    TEST_ASSERT_EQUAL(-8, correctionSoftFlatShift(-8, testData.current, testData.page6));
    TEST_ASSERT_BIT_LOW(BIT_STATUS5_FLATSS, testData.current.status5);
}

static void test_correctionSoftFlatShift_off_rpmnotinwindow(void) {
    soft_flatshift_data_t testData;
    setup_correctionSoftFlatShift(testData);
    testData.page6.flatSRetard = -3;
    testData.current.RPM = (testData.current.clutchEngagedRPM - (testData.page6.flatSSoftWin * 100) ) - 100;

    BIT_SET(testData.current.status5, BIT_STATUS5_FLATSS);
    TEST_ASSERT_EQUAL(-8, correctionSoftFlatShift(-8, testData.current, testData.page6));
    TEST_ASSERT_BIT_LOW(BIT_STATUS5_FLATSS, testData.current.status5);
}

static void test_correctionSoftFlatShift(void) {
    RUN_TEST_P(test_correctionSoftFlatShift_on);
    RUN_TEST_P(test_correctionSoftFlatShift_off_disabled);
    RUN_TEST_P(test_correctionSoftFlatShift_off_noclutchtrigger);
    RUN_TEST_P(test_correctionSoftFlatShift_off_clutchrpmtoolow);
    RUN_TEST_P(test_correctionSoftFlatShift_off_rpmnotinwindow);
}

#if 0 // Wait until Noisymime is done with knock implementation
extern int8_t correctionKnock(int8_t advance);

static void setup_correctionKnock(void) {
    configPage10.knock_mode = KNOCK_MODE_DIGITAL;
    configPage10.knock_count = 5U;
    configPage10.knock_firstStep = 3U;
    // knockCounter = configPage10.knock_count + 1;
//   TEST_DATA_P uint8_t startBins[] = { 30, 40, 50, 60, 70, 80 };
//   TEST_DATA_P uint8_t startValues[] = { 30, 25, 20, 15, 10, 5 };
//   populate_2dtable_P(&knockWindowStartTable, startValues, startBins);

//   TEST_DATA_P uint8_t durationBins[] = { 30, 40, 50, 60, 70, 80 };
//   TEST_DATA_P uint8_t durationValues[] = { 30, 25, 20, 15, 10, 5 };
//   populate_2dtable_P(&knockWindowDurationTable, durationValues, durationBins);
}

static void test_correctionKnock_firstStep(void) {
    setup_correctionKnock();

    TEST_ASSERT_EQUAL(-11, correctionKnock(-8));
}

static void test_correctionKnock_disabled_modeoff(void) {
    setup_correctionKnock();
    configPage10.knock_mode = KNOCK_MODE_OFF;
    TEST_ASSERT_EQUAL(-8, correctionKnock(-8));
}

static void test_correctionKnock_disabled_counttoolow(void) {
    setup_correctionKnock();
    knockCounter = configPage10.knock_count - 1;
    TEST_ASSERT_EQUAL(-8, correctionKnock(-8));
}

static void test_correctionKnock_disabled_knockactive(void) {
    setup_correctionKnock();
    currentStatus.knockActive = true;
    TEST_ASSERT_EQUAL(-8, correctionKnock(-8));
}
#endif

static void test_correctionKnock(void) {
}

struct dwell_correction_data_t {
    statuses current;
    config2 page2;
    config4 page4;
    config10 page10;
    test_2dtable_t<6> lookupTable;
};

static void setup_correctionsDwell(dwell_correction_data_t &testData) {
    initialiseIgnCorrections(testData.current);
    BIT_SET(LOOP_TIMER, BIT_TIMER_4HZ);

    testData.page4.sparkDur = 10;
    testData.page2.perToothIgn = false;
    testData.page4.dwellErrCorrect = 0;
    testData.page4.sparkMode = IGN_MODE_WASTED;

    testData.current.actualDwell = 770;
    testData.current.battery10 = 95;

    revolutionTime = 666;

    TEST_DATA_P uint8_t bins[] = { 60,  70,  80,  90,  100, 110 };
    TEST_DATA_P uint8_t values[] = { 130, 125, 120, 115, 110, 90 };
    populate_2dtable_P(&testData.lookupTable.lookupTable, values, bins);   
}

static void test_correctionsDwell_nopertooth(void) {
    dwell_correction_data_t testData;
    setup_correctionsDwell(testData);

    testData.current.battery10 = 105;
    testData.page2.nCylinders = 8;

    testData.page4.sparkMode = IGN_MODE_WASTED;
    TEST_ASSERT_EQUAL(296, correctionsDwell(800, testData.current, testData.page2, testData.page4, testData.page10, testData.lookupTable.lookupTable));

    testData.page4.sparkMode = IGN_MODE_SINGLE;
    TEST_ASSERT_EQUAL(74, correctionsDwell(800, testData.current, testData.page2, testData.page4, testData.page10, testData.lookupTable.lookupTable));

    testData.page4.sparkMode = IGN_MODE_ROTARY;
    testData.page10.rotaryType = ROTARY_IGN_RX8;
    TEST_ASSERT_EQUAL(296, correctionsDwell(800, testData.current, testData.page2, testData.page4, testData.page10, testData.lookupTable.lookupTable));

    testData.page4.sparkMode = IGN_MODE_ROTARY;
    testData.page10.rotaryType = ROTARY_IGN_FC;
    TEST_ASSERT_EQUAL(74, correctionsDwell(800, testData.current, testData.page2, testData.page4, testData.page10, testData.lookupTable.lookupTable));
}

static void test_correctionsDwell_pertooth(void) {
    dwell_correction_data_t testData;
    setup_correctionsDwell(testData);

    testData.current.battery10 = 105;
    testData.page2.perToothIgn = true;
    testData.page4.dwellErrCorrect = 1;
    testData.page4.sparkMode = IGN_MODE_WASTED;

    testData.current.actualDwell = 200;
    TEST_ASSERT_EQUAL(444, correctionsDwell(800, testData.current, testData.page2, testData.page4, testData.page10, testData.lookupTable.lookupTable));

    testData.current.actualDwell = 1400;
    TEST_ASSERT_EQUAL(296, correctionsDwell(800, testData.current, testData.page2, testData.page4, testData.page10, testData.lookupTable.lookupTable));
}

static void test_correctionsDwell_wasted_nopertooth_largerevolutiontime(void) {
    dwell_correction_data_t testData;
    setup_correctionsDwell(testData);

    testData.current.battery10 = 105;
    revolutionTime = 5000;
    TEST_ASSERT_EQUAL(800, correctionsDwell(800, testData.current, testData.page2, testData.page4, testData.page10, testData.lookupTable.lookupTable));
}

static void test_correctionsDwell_initialises_current_actualDwell(void) {
    dwell_correction_data_t testData;
    setup_correctionsDwell(testData);

    testData.current.actualDwell = 0;
    correctionsDwell(777, testData.current, testData.page2, testData.page4, testData.page10, testData.lookupTable.lookupTable);
    TEST_ASSERT_EQUAL(777, testData.current.actualDwell);
}

static void test_correctionsDwell_uses_batvcorrection(void) {
    dwell_correction_data_t testData;
    setup_correctionsDwell(testData);

    testData.page2.nCylinders = 8;
    testData.page4.sparkMode = IGN_MODE_WASTED;

    testData.current.battery10 = 105;
    TEST_ASSERT_EQUAL(296, correctionsDwell(800, testData.current, testData.page2, testData.page4, testData.page10, testData.lookupTable.lookupTable));

    testData.current.battery10 = 65;
    TEST_ASSERT_EQUAL(337, correctionsDwell(800, testData.current, testData.page2, testData.page4, testData.page10, testData.lookupTable.lookupTable));
}

static void test_correctionsDwell(void) {
    RUN_TEST_P(test_correctionsDwell_nopertooth);
    RUN_TEST_P(test_correctionsDwell_pertooth);
    RUN_TEST_P(test_correctionsDwell_wasted_nopertooth_largerevolutiontime);
    RUN_TEST_P(test_correctionsDwell_initialises_current_actualDwell);
    RUN_TEST_P(test_correctionsDwell_uses_batvcorrection);
}

void testIgnCorrections(void) {
    Unity.TestFile = __FILE__;

    test_correctionFixedTiming();
    test_correctionCLTadvance();
    test_correctionCrankingFixedTiming();
    test_correctionFlexTiming();
    test_correctionWMITiming();
    test_correctionIATretard();
    test_correctionIdleAdvance();
    test_correctionSoftRevLimit();
    test_correctionNitrous();
    test_correctionSoftLaunch();
    test_correctionSoftFlatShift();
    test_correctionKnock();
    // correctionDFCOignition() is tested in the fueling unit tests, since it is tightly coupled to fuel DFCO
    test_correctionsDwell();
}