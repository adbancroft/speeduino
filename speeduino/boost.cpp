#include <SimplyAtomic.h>
#include "src/PID_v1/PID_v1.h"
#include "board_definition.h"
#include "port_pin.h"
#include "boost.h"
#include "unit_testing.h"
#include "table2d.h"
#include "config_pages.h"
#include "maths.h"
#include "timers.h"
#include "globals.h"

#define SIMPLE_BOOST_P  1
#define SIMPLE_BOOST_I  1
#define SIMPLE_BOOST_D  1

#if(defined(CORE_TEENSY) || defined(CORE_STM32))
#define BOOST_PIN_LOW()         (digitalWrite(pinBoost, LOW))
#define BOOST_PIN_HIGH()        (digitalWrite(pinBoost, HIGH))
#else
#define BOOST_PIN_LOW()         ATOMIC() { *boost_pin_port &= ~(boost_pin_mask); }
#define BOOST_PIN_HIGH()        ATOMIC() { *boost_pin_port |= (boost_pin_mask);  }
#endif

static uint8_t boostCounter;
static PORT_TYPE boost_pin_port;
static PINMASK_TYPE boost_pin_mask;
static long boost_pwm_target_value;
static volatile bool boost_pwm_state;
static volatile unsigned int boost_pwm_cur_value = 0;
uint16_t boost_pwm_max_count; //Used for variable PWM frequency
TESTABLE_STATIC table2D_u8_s16_6 flexBoostTable(&configPage10.flexBoostBins, &configPage10.flexBoostAdj);

//Old PID method. Retained in case the new one has issues
//integerPID boostPID(&MAPx100, &boost_pwm_target_value, &boostTargetx100, configPage6.boostKP, configPage6.boostKI, configPage6.boostKD, DIRECT);
static integerPID_ideal boostPID; //This is the PID object if that algorithm is used. Needs to be global as it maintains state outside of each function call

static void configureBoostPid(const config2 &page2, const config6 &page6, const config10 &page10)
{
  boostPID.SetSampleInterval(page10.boostIntv);
  boostPID.SetControllerDirection(DIRECT);
  boostPID.SetOutputLimits(page2.boostMinDuty, page2.boostMaxDuty);

  if(page6.boostMode == BOOST_MODE_SIMPLE) { 
    boostPID.SetTunings(SIMPLE_BOOST_P, SIMPLE_BOOST_I, SIMPLE_BOOST_D); 
  } else { 
    boostPID.SetTunings(page6.boostKP, page6.boostKI, page6.boostKD); 
  }
}

void initialiseBoost(void)
{
  boost_pin_port = portOutputRegister(digitalPinToPort(pinBoost));
  boost_pin_mask = digitalPinToBitMask(pinBoost);

  configureBoostPid(configPage2, configPage6, configPage10);

  boostCounter = 0;
  currentStatus.boostDuty = 0;
}


TESTABLE_STATIC bool isBoostByGear(const config2 &page2, const config9 &page9)
{
  return (page9.boostByGearEnabled!=BOOST_BY_GEAR_OFF) && (page2.vssMode > 1);
}

TESTABLE_STATIC uint8_t gearToBoostFactor(uint8_t gear, const config9 &page9)
{
  switch (gear)
  {
    case 1: return page9.boostByGear1; break;
    case 2: return page9.boostByGear2; break;
    case 3: return page9.boostByGear3; break;
    case 4: return page9.boostByGear4; break;
    case 5: return page9.boostByGear5; break;
    case 6: return page9.boostByGear6; break;
    default: return 0U; break;
  }
  return 0U;
}

static uint8_t lookupBoostTarget(const statuses &current)
{
  return get3DTableValue(&boostTable, (current.TPS * 2U), current.RPM);
}

TESTABLE_STATIC uint16_t calcBoostByGearDuty(const statuses &current, const config9 &page9)
{
  if( page9.boostByGearEnabled == BOOST_BY_GEAR_MULTIPLIED )
  {
    return (uint16_t)gearToBoostFactor(current.gear, page9) * (uint16_t)lookupBoostTarget(current) * 4U;
  }
  if( page9.boostByGearEnabled == BOOST_BY_GEAR_CONSTANT ) 
  {
    return (uint16_t)gearToBoostFactor(current.gear, page9) * 2U * 100U;
  }
  return 0U;
}

TESTABLE_STATIC uint16_t calcBoostByGearTarget(const statuses &current, const config9 &page9)
{
  if( page9.boostByGearEnabled == BOOST_BY_GEAR_MULTIPLIED )
  {
    return (((uint16_t)gearToBoostFactor(current.gear, page9) * (uint16_t)lookupBoostTarget(current)) / 100U ) * 4U;
  }
  if( page9.boostByGearEnabled == BOOST_BY_GEAR_CONSTANT ) 
  {
    return (uint16_t)gearToBoostFactor(current.gear, page9) * 2U;
  }
  return 0U;
}

TESTABLE_STATIC uint16_t calcOLBoostDuty(const statuses &current, const config2 &page2, const config9 &page9)
{
  if ( isBoostByGear(page2, page9) ) { 
    return calcBoostByGearDuty(current, page9); 
  }
  
  return lookupBoostTarget(current) * 2U * 100U; 
}

TESTABLE_STATIC int16_t lookupFlexBoostCorrection(const statuses &current, const config2 &page2)
{
  //If flex fuel is enabled, there can be an adder to the boost target based on ethanol content
  if( page2.flexEnabled == 1U )
  {
    return table2D_getValue(&flexBoostTable, current.ethanolPct);
  }
  return 0;  
}

TESTABLE_STATIC uint16_t getCLBoostTarget(const statuses &current, const config2 &page2, const config9 &page9)
{
  if ( isBoostByGear(page2, page9) ) { 
    return calcBoostByGearTarget(current, page9); 
  }
  
  return lookupBoostTarget(current) << 1U; //Boost target table is in kpa and divided by 2
}

static uint32_t boostDutyToPwm(uint16_t duty)
{
  return ((uint32_t)(duty) * boost_pwm_max_count) / 10000UL; //Convert boost duty (Which is a % multiplied by 100) to a pwm count
}

static bool isBaroBoostControlEnabled(const statuses &current, const config15 &page15)
{
  return (page15.boostControlEnable == EN_BOOST_CONTROL_BARO) && (current.MAP >= (long)current.baro);
}

static bool isFixedBoostControlEnabled(const statuses &current, const config15 &page15)
{
  return (page15.boostControlEnable == EN_BOOST_CONTROL_FIXED) && (current.MAP >= (long)page15.boostControlEnableThreshold);
}

TESTABLE_STATIC bool isBoostControlEnabled(const statuses &current, const config15 &page15)
{
  //Only enables boost control above baro pressure or above user defined threshold (User defined level is usually set to boost with wastegate actuator only boost level)
  return isBaroBoostControlEnabled(current, page15) 
      || isFixedBoostControlEnabled(current, page15);
}

TESTABLE_STATIC uint16_t boostTargetToDuty(uint16_t target, const statuses &current, const config2 &page2, const config6 &page6, const config10 &page10)
{
  uint16_t boostDuty = 0U;

  if(target > 0U)
  {
    //This only needs to be run very infrequently, once every 16 calls to boostControl().
    if( (boostCounter & 15U) == 1U)
    {
      configureBoostPid(page2, page6, page10);
    }

    if (!boostPID.Compute( current.MAP, target, page10.boostSens,
                            get3DTableValue(&boostTableLookupDuty, target, current.RPM) * 100U/2U, 
                            &boostDuty))
    {
      boostDuty = current.boostDuty;
    }
  }
  return boostDuty;
}

TESTABLE_STATIC uint16_t calcCLBoostDuty(statuses &current, const config2 &page2, const config6 &page6, const config9 &page9, const config10 &page10, const config15 &page15)
{
  if( isBoostControlEnabled(current, page15) )
  {
    current.flexBoostCorrection = lookupFlexBoostCorrection(current, page2);
    int16_t totalTarget = (int16_t)getCLBoostTarget(current, page2, page9) + current.flexBoostCorrection;
    current.boostTarget = (uint16_t)clamp(totalTarget, (int16_t)0, (int16_t)INT16_MAX); 
    return boostTargetToDuty(current.boostTarget, current, page2, page6, page10);
  }

  boostPID.Initialize(current.MAP); //This resets the ITerm value to prevent rubber banding
  // Boost control needs to have a high duty cycle if control is below threshold (baro or fixed value). 
  // This ensures the waste gate is closed as much as possible, this build boost as fast as possible.
  return page15.boostDCWhenDisabled*100U;
}

TESTABLE_STATIC uint16_t calcBoostDuty(uint8_t boostType, statuses &current, const config2 &page2, const config6 &page6, const config9 &page9, const config10 &page10, const config15 &page15)
{
  if(boostType == OPEN_LOOP_BOOST)
  {
    return calcOLBoostDuty(current, page2, page9);
  }
  
  if (boostType == CLOSED_LOOP_BOOST)
  {
    return calcCLBoostDuty(current, page2, page6, page9, page10, page15);
  } 

  return 0;
}

static void applyBoostDuty(uint16_t duty)
{
  //Check for 100% duty cycle
  if(duty >= 10000U)
  {
    DISABLE_BOOST_TIMER(); //Turn off the compare unit (ie turn off the interrupt) if boost duty is 100%
    BOOST_PIN_HIGH(); //Turn on boost pin if duty is 100%
  }
  else if(duty> 0U)
  {
    boost_pwm_target_value = boostDutyToPwm(duty);
    ENABLE_BOOST_TIMER(); //Turn on the compare unit (ie turn on the interrupt) if boost duty is > 0
  }
  else // duty== 0
  {
    boostDisable();
  }
}

void boostControl(void)
{
  if( configPage6.boostEnabled==1U )
  {
    currentStatus.boostDuty = calcBoostDuty(configPage4.boostType, currentStatus, configPage2, configPage6, configPage9, configPage10, configPage15);
    applyBoostDuty(currentStatus.boostDuty);
  }
  else { // Disable timer channel and zero the flex boost correction status
    DISABLE_BOOST_TIMER();
    currentStatus.flexBoostCorrection = 0;
  }

  boostCounter++;
}

void boostDisable(void)
{
  boostPID.Initialize(currentStatus.MAP); //This resets the ITerm value to prevent rubber banding
  currentStatus.boostDuty = 0;
  DISABLE_BOOST_TIMER(); //Turn off timer
  BOOST_PIN_LOW(); //Make sure solenoid is off (0% duty)
}

//The interrupt to control the Boost PWM
#if defined(CORE_AVR)
  ISR(TIMER1_COMPA_vect) //cppcheck-suppress misra-c2012-8.2
#else
  void boostInterrupt(void) //Most ARM chips can simply call a function
#endif
{
  if (boost_pwm_state == true)
  {
    #if defined(CORE_TEENSY41) //PIT TIMERS count down and have opposite effect on PWM
    BOOST_PIN_HIGH();
    #else
    BOOST_PIN_LOW();  // Switch pin to low
    #endif
    SET_COMPARE(BOOST_TIMER_COMPARE, BOOST_TIMER_COUNTER + (boost_pwm_max_count - boost_pwm_cur_value) );
    boost_pwm_state = false;
  }
  else
  {
    #if defined(CORE_TEENSY41) //PIT TIMERS count down and have opposite effect on PWM
    BOOST_PIN_LOW();
    #else
    BOOST_PIN_HIGH();  // Switch pin high
    #endif
    SET_COMPARE(BOOST_TIMER_COMPARE, BOOST_TIMER_COUNTER + boost_pwm_target_value);
    boost_pwm_cur_value = boost_pwm_target_value;
    boost_pwm_state = true;
  }
}
