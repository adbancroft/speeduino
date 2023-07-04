#include <unity.h>
#include "test_staging.h"
#include "../test_utils.h"
#include "globals.h"
#include "scheduler.h"
#include "speeduino.h"

void testStaging(void)
{
  SET_UNITY_FILENAME() {

  RUN_TEST(test_Staging_Off);
  RUN_TEST(test_Staging_4cyl_Auto_Inactive);
  RUN_TEST(test_Staging_4cyl_Table_Inactive);
  RUN_TEST(test_Staging_4cyl_Auto_50pct);
  RUN_TEST(test_Staging_4cyl_Auto_33pct);
  RUN_TEST(test_Staging_4cyl_Table_50pct);
  }
}

void test_Staging_setCommon()
{
  configPage2.nCylinders = 4;
  currentStatus.RPM = 3000;
  currentStatus.fuelLoad = 50;
  inj_opentime_uS = 1000; //1ms inj open time
  injectorChannels.primary = 2;
  injectorChannels.secondary = 2;

  /*
      These values are a percentage of the req_fuel value that would be required for each injector channel to deliver that much fuel.
      Eg:
      Pri injectors are 250cc
      Sec injectors are 500cc
      Total injector capacity = 750cc

      staged_req_fuel_mult_pri = 300% (The primary injectors would have to run 3x the overall PW in order to be the equivalent of the full 750cc capacity
      staged_req_fuel_mult_sec = 150% (The secondary injectors would have to run 1.5x the overall PW in order to be the equivalent of the full 750cc capacity
  */
  configPage10.stagedInjSizePri = 250;
  configPage10.stagedInjSizeSec = 500;
  uint32_t totalInjector = configPage10.stagedInjSizePri + configPage10.stagedInjSizeSec;

  staged_req_fuel_mult_pri = (100 * totalInjector) / configPage10.stagedInjSizePri;
  staged_req_fuel_mult_sec = (100 * totalInjector) / configPage10.stagedInjSizeSec;
}

extern pulseWidths applyStagingToPW(uint16_t pwLimit, uint16_t pwPrimary);

void test_Staging_Off(void)
{
  test_Staging_setCommon();

  BIT_SET(currentStatus.status4, BIT_STATUS4_STAGING_ACTIVE);
  configPage10.stagingEnabled = false;

  pulseWidths pw =applyStagingToPW(9000, 3000); //90% duty cycle at 6000rpm
  TEST_ASSERT_EQUAL(3000, pw.primary);
  TEST_ASSERT_EQUAL(0, pw.secondary);  
  TEST_ASSERT_FALSE(BIT_CHECK(currentStatus.status4, BIT_STATUS4_STAGING_ACTIVE));
}

void test_Staging_4cyl_Auto_Inactive(void)
{
  test_Staging_setCommon();

  BIT_SET(currentStatus.status4, BIT_STATUS4_STAGING_ACTIVE);
  configPage2.injLayout = INJ_PAIRED;
  configPage10.stagingEnabled = true;
  configPage10.stagingMode = STAGING_MODE_AUTO;

  pulseWidths pw = applyStagingToPW(9000, 3000); 
  //Primary should be ((PW - openTime) * staged_req_fuel_mult_pri) + openTime = ((3000 - 1000) * 3.0) + 1000 = 7000
  //Seconary should be 0 as that testPW is below the pwLimit
  TEST_ASSERT_EQUAL(7000, pw.primary);
  TEST_ASSERT_EQUAL(0, pw.secondary);  
  TEST_ASSERT_FALSE(BIT_CHECK(currentStatus.status4, BIT_STATUS4_STAGING_ACTIVE));
}

void test_Staging_4cyl_Table_Inactive(void)
{
  test_Staging_setCommon();

  BIT_SET(currentStatus.status4, BIT_STATUS4_STAGING_ACTIVE);
  configPage2.injLayout = INJ_PAIRED;
  configPage10.stagingEnabled = true;
  configPage10.stagingMode = STAGING_MODE_TABLE;

  //Load the staging table with all 0
  //For this test it doesn't matter what the X and Y axis are, as the table is all 0 values
  for(byte x=0; x<64; x++) { stagingTable.values.values[x] = 0; }

  pulseWidths pw = applyStagingToPW(9000, 3000);
  //Primary should be ((PW - openTime) * staged_req_fuel_mult_pri) + openTime = ((3000 - 1000) * 3.0) + 1000 = 7000
  //Seconary should be 0 as that testPW is below the pwLimit
  TEST_ASSERT_EQUAL(7000, pw.primary);
  TEST_ASSERT_EQUAL(0, pw.secondary);  
  TEST_ASSERT_FALSE(BIT_CHECK(currentStatus.status4, BIT_STATUS4_STAGING_ACTIVE));
}

void test_Staging_4cyl_Auto_50pct(void)
{
  test_Staging_setCommon();

  BIT_CLEAR(currentStatus.status4, BIT_STATUS4_STAGING_ACTIVE);
  configPage2.injLayout = INJ_PAIRED;
  configPage10.stagingEnabled = true;
  configPage10.stagingMode = STAGING_MODE_AUTO;

  pulseWidths pw = applyStagingToPW(9000, 9000);
  //Primary should be maxed out at the pwLimit, secondary should be based on their relative size
  TEST_ASSERT_EQUAL(9000, pw.primary);
  TEST_ASSERT_EQUAL(9000, pw.secondary);  
  TEST_ASSERT_TRUE(BIT_CHECK(currentStatus.status4, BIT_STATUS4_STAGING_ACTIVE));
}

void test_Staging_4cyl_Auto_33pct(void)
{
  test_Staging_setCommon();

  BIT_CLEAR(currentStatus.status4, BIT_STATUS4_STAGING_ACTIVE);
  configPage2.injLayout = INJ_PAIRED;
  configPage10.stagingEnabled = true;
  configPage10.stagingMode = STAGING_MODE_AUTO;

  pulseWidths pw = applyStagingToPW(9000, 7000);
  //Primary should be maxed out at the pwLimit, secondary should be based on their relative size
  TEST_ASSERT_EQUAL(9000, pw.primary);
  TEST_ASSERT_EQUAL(6000, pw.secondary);  
  TEST_ASSERT_TRUE(BIT_CHECK(currentStatus.status4, BIT_STATUS4_STAGING_ACTIVE));
}

void test_Staging_4cyl_Table_50pct(void)
{
  test_Staging_setCommon();

  BIT_CLEAR(currentStatus.status4, BIT_STATUS4_STAGING_ACTIVE);
  configPage2.injLayout = INJ_PAIRED;
  configPage10.stagingEnabled = true;
  configPage10.stagingMode = STAGING_MODE_TABLE;

  //Load the staging table with all 0
  //For this test it doesn't matter what the X and Y axis are, as the table is all 50 values
  for(byte x=0; x<64; x++) { stagingTable.values.values[x] = 50; }

  //Need to change the lookup values so we don't get a cached value
  currentStatus.RPM += 1;
  currentStatus.fuelLoad += 1;

  pulseWidths pw = applyStagingToPW(9000, 3000);
  TEST_ASSERT_EQUAL(4000, pw.primary);
  TEST_ASSERT_EQUAL(2500, pw.secondary);  
  TEST_ASSERT_TRUE(BIT_CHECK(currentStatus.status4, BIT_STATUS4_STAGING_ACTIVE));
}