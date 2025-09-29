#include "pw_calcs.h"
#include "maths.h"
#include "unit_testing.h"

uint16_t calculateRequiredFuel(const config2 &page2, const statuses &current) {
  uint16_t reqFuel = page2.reqFuel * 100U; //Convert to uS and an int. This is the only variable to be used in calculations
  if ((page2.strokes == FOUR_STROKE) && ((page2.injLayout != INJ_SEQUENTIAL) || BIT_CHECK(current.status3, BIT_STATUS3_HALFSYNC)))
  {
    //Default is 1 squirt per revolution, so we halve the given req-fuel figure (Which would be over 2 revolutions)
    //The req_fuel calculation above gives the total required fuel (At VE 100%) in the full cycle.
    //If we're doing more than 1 squirt per cycle then we need to split the amount accordingly.
    //(Note that in a non-sequential 4-stroke setup you cannot have less than 2 squirts as you cannot determine the
    //stroke to make the single squirt on)
    reqFuel = reqFuel / 2U; 
  }

  return reqFuel;
}

static inline uint16_t calcNitrousStagePulseWidth(uint8_t minRPM, uint8_t maxRPM, uint8_t adderMin, uint8_t adderMax, const statuses &current)
{
  int16_t adderRange = (maxRPM - minRPM) * 100;
  int16_t adderPercent = ((current.RPM - (minRPM * 100)) * 100) / adderRange; //The percentage of the way through the RPM range
  adderPercent = 100 - adderPercent; //Flip the percentage as we go from a higher adder to a lower adder as the RPMs rise
  return (adderMax + percentage(adderPercent, (adderMin - adderMax))) * 100; //Calculate the above percentage of the calculated ms value.
}

//Manual adder for nitrous. These are not in correctionsFuel() because they are direct adders to the ms value, not % based
TESTABLE_INLINE_STATIC uint16_t pwApplyNitrous(uint16_t pw, const config10 &page10, const statuses &current)
{
  if (current.nitrous_status!=NITROUS_OFF && pw!=0U)
  {
    if( (current.nitrous_status == NITROUS_STAGE1) || (current.nitrous_status == NITROUS_BOTH) )
    {
      pw = pw + calcNitrousStagePulseWidth(page10.n2o_stage1_minRPM, page10.n2o_stage1_maxRPM, page10.n2o_stage1_adderMin, page10.n2o_stage1_adderMax, current);
    }
    if( (current.nitrous_status == NITROUS_STAGE2) || (current.nitrous_status == NITROUS_BOTH) )
    {
      pw = pw + calcNitrousStagePulseWidth(page10.n2o_stage2_minRPM, page10.n2o_stage2_maxRPM, page10.n2o_stage2_adderMin, page10.n2o_stage2_adderMax, current);
    }
  }

  return pw;
}

uint16_t calculatePWLimit(const config2 &page2, const statuses &current, uint32_t revTime)
{
  uint32_t tempLimit = percentageApprox(page2.dutyLim, revTime); //The pulsewidth limit is determined to be the duty cycle limit (Eg 85%) by the total time it takes to perform 1 revolution
  //Handle multiple squirts per rev
  if (page2.strokes == FOUR_STROKE) { tempLimit = tempLimit * 2; }
  //Optimise for power of two divisions where possible
  switch(current.nSquirts)  {
    case 1:
      //No action needed
      break;
    case 2:
      tempLimit = tempLimit / 2;
      break;
    case 4:
      tempLimit = tempLimit / 4;
      break;
    case 8:
      tempLimit = tempLimit / 8;
      break;
    default:
      //Non-PoT squirts value. Perform (slow) uint32_t division
      tempLimit = tempLimit / current.nSquirts;
      break;
  }
  if(tempLimit > (uint32_t)UINT16_MAX) { tempLimit = UINT16_MAX; }

  return tempLimit;
}

/**
 * @brief This function calculates the required pulsewidth time (in us) given the current system state
 * 
 * @param REQ_FUEL The required fuel value in uS, as calculated by TunerStudio
 * @param VE Lookup from the main fuel table. This can either have been MAP or TPS based, depending on the algorithm used
 * @param MAP In KPa, read from the sensor (This is used when performing a multiply of the map only. It is applicable in both Speed density and Alpha-N)
 * @param corrections Sum of Enrichment factors (Cold start, acceleration). This is a multiplication factor (Eg to add 10%, this should be 110)
 * @param injOpen Injector opening time. The time the injector take to open minus the time it takes to close (Both in uS)
 * @return uint16_t The injector pulse width in uS
 */
uint16_t PW(int REQ_FUEL, byte VE, long MAP, uint16_t corrections, int injOpen, const config10 &page10, const statuses &current)
{
  //Standard float version of the calculation
  //return (REQ_FUEL * (float)(VE/100.0) * (float)(MAP/100.0) * (float)(TPS/100.0) * (float)(corrections/100.0) + injOpen);
  //Note: The MAP and TPS portions are currently disabled, we use VE and corrections only
  uint16_t iMAP = 100;
  uint16_t iAFR = 147;

  //100% float free version, does sacrifice a little bit of accuracy, but not much.
 
  //Check whether either of the multiply MAP modes is turned on
  //if ( configPage2.multiplyMAP == MULTIPLY_MAP_MODE_100) { iMAP = ((unsigned int)MAP << 7) / 100; }
  if ( configPage2.multiplyMAP == MULTIPLY_MAP_MODE_100) { iMAP = div100( ((uint16_t)MAP << 7U) ); }
  else if( configPage2.multiplyMAP == MULTIPLY_MAP_MODE_BARO) { iMAP = ((unsigned int)MAP << 7U) / currentStatus.baro; }
  
  if ( (configPage2.includeAFR == true) && (configPage6.egoType == EGO_TYPE_WIDE) && (currentStatus.runSecs > configPage6.ego_sdelay) ) {
    iAFR = ((unsigned int)currentStatus.O2 << 7U) / currentStatus.afrTarget;  //Include AFR (vs target) if enabled
  }
  if ( (configPage2.incorporateAFR == true) && (configPage2.includeAFR == false) ) {
    iAFR = ((unsigned int)configPage2.stoich << 7U) / currentStatus.afrTarget;  //Incorporate stoich vs target AFR, if enabled.
  }

  uint32_t intermediate = percentageApprox(VE, REQ_FUEL); //Need to use an intermediate value to avoid overflowing the long
  if ( configPage2.multiplyMAP > 0 ) { intermediate = rshift<7U>(intermediate * (uint32_t)iMAP); }
  
  if ( (configPage2.includeAFR == true) && (configPage6.egoType == EGO_TYPE_WIDE) && (currentStatus.runSecs > configPage6.ego_sdelay) ) {
    //EGO type must be set to wideband and the AFR warmup time must've elapsed for this to be used
    intermediate = rshift<7U>(intermediate * (uint32_t)iAFR);  
  }
  if ( (configPage2.incorporateAFR == true) && (configPage2.includeAFR == false) ) {
    intermediate = rshift<7U>(intermediate * (uint32_t)iAFR);
  }

  //If corrections are huge, use less bitshift to avoid overflow. Sacrifices a bit more accuracy (basically only during very cold temp cranking)
  intermediate = percentageApprox(corrections, intermediate);

  if (intermediate != 0)
  {
    //If intermediate is not 0, we need to add the opening time (0 typically indicates that one of the full fuel cuts is active)
    intermediate += injOpen; //Add the injector opening time
    //AE calculation only when ACC is active.
    if ( BIT_CHECK(currentStatus.engine, BIT_ENGINE_ACC) )
    {
      //AE Adds % of req_fuel
      if ( configPage2.aeApplyMode == AE_MODE_ADDER )
        {
          intermediate += div100(((uint32_t)REQ_FUEL) * (currentStatus.AEamount - 100U));
        }
    }

    if ( intermediate > UINT16_MAX)
    {
      intermediate = UINT16_MAX;  //Make sure this won't overflow when we convert to uInt. This means the maximum pulsewidth possible is 65.535mS
    }
  }
  return pwApplyNitrous((unsigned int)intermediate, page10, current);
}
