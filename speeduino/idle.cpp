/*
Speeduino - Simple engine management for the Arduino Mega 2560 platform
Copyright (C) Josh Stewart
A full copy of the license may be found in the projects root directory
*/
#include "idle.h"
#include "maths.h"
#include "timers.h"
#include "port_pin.h"
#include "table2d.h"
#include "src/PID_v1/PID_v1.h"

#define STEPPER_LESS_AIR_DIRECTION() ((configPage9.iacStepperInv == 0) ? STEPPER_BACKWARD : STEPPER_FORWARD)
#define STEPPER_MORE_AIR_DIRECTION() ((configPage9.iacStepperInv == 0) ? STEPPER_FORWARD : STEPPER_BACKWARD)

static byte idleCounter; //Used for tracking the number of calls to the idle control function
static uint8_t idleTaper;

static struct StepperIdle idleStepper;
static bool idleOn; //Simply tracks whether idle was on last time around
static byte idleInitComplete = 99; //Tracks which idle method was initialised. 99 is a method that will never exist
static unsigned int iacStepTime_uS;
static unsigned int iacCoolTime_uS;
static unsigned int completedHomeSteps;

static volatile bool idle_pwm_state;
static bool lastDFCOValue;
static uint16_t idle_pwm_max_count; //Used for variable PWM frequency
static volatile unsigned int idle_pwm_cur_value;
static long idle_pid_target_value;
static long FeedForwardTerm;
static unsigned long idle_pwm_target_value;
static long idle_cl_target_rpm;

static struct table2D<uint8_t, uint8_t, 10> iacPWMTable(configPage6.iacBins, configPage6.iacOLPWMVal);
static struct table2D<uint8_t, uint8_t, 10> iacStepTable(configPage6.iacBins, configPage6.iacOLStepVal);
//Open loop tables specifically for cranking
static struct table2D<uint8_t, uint8_t, 4> iacCrankStepsTable(configPage6.iacCrankBins, configPage6.iacCrankSteps);
static struct table2D<uint8_t, uint8_t, 4> iacCrankDutyTable(configPage6.iacCrankBins, configPage6.iacCrankDuty);

/*
These functions cover the PWM and stepper idle control
*/

/*
Idle Control
Currently limited to on/off control and open loop PWM and stepper drive
*/
static integerPID idlePID(&currentStatus.longRPM, &idle_pid_target_value, &idle_cl_target_rpm, configPage6.idleKP, configPage6.idleKI, configPage6.idleKD, DIRECT); //This is the PID object if that algorithm is used. Needs to be global as it maintains state outside of each function call

//Any common functions associated with starting the Idle
//Typically this is enabling the PWM interrupt
static inline void enableIdle(void)
{
  if( isIdlePwm() )
  {
    IDLE_TIMER_ENABLE();
  }
}

static uint8_t pinIdle1;
static uint8_t pinIdle2;
static uint8_t pinIdleUp;
static uint8_t pinIdleUpOutput;
static uint8_t pinStepperStep;
static uint8_t pinStepperDir;
static uint8_t pinStepperEnable;

static inline uint8_t getIdleUpOutputHIGH(void) {
  return !configPage2.idleUpOutputInv;
}

static inline uint8_t getIdleUpOutputLOW(void) {
  return configPage2.idleUpOutputInv;
}

void initialiseIdleUpOutput(void)
{
  if (isIdleUpOutputEnabled()) {
    setPinState(pinIdleUpOutput, getIdleUpOutputLOW()); //Initialise program with the idle up output in the off state if it is enabled. 
  }
  currentStatus.idleUpOutputActive = false;
}

void initialiseIdle(bool forcehoming, const pin_mapping_t &pins) {
  //Pin masks must always be initialised, regardless of whether PWM idle is used. This is required for STM32 to prevent issues if the IRQ function fires on restart/overflow
  MATCH_PIN_TO_FEATURE(isIdlePwm, pins.outputs.pinIdle1, OUTPUT, configPage6.iacAlgorithm)
  pinIdle1 = pins.outputs.pinIdle1;
  MATCH_PIN_TO_FEATURE(isIdle2PwmEnabled, pins.outputs.pinIdle2, OUTPUT, configPage6.iacAlgorithm)
  pinIdle2 = pins.outputs.pinIdle2;
  MATCH_PIN_TO_FEATURE(isIdleUpOutputEnabled, pins.outputs.pinIdleUpOutput, OUTPUT, configPage2.idleUpOutputEnabled)
  pinIdleUpOutput = pins.outputs.pinIdleUpOutput;
  MATCH_PIN_TO_FEATURE(isIdleStepper, pins.outputs.pinStepperDir, OUTPUT, configPage6.iacAlgorithm)
  pinStepperDir = pins.outputs.pinStepperDir;
  MATCH_PIN_TO_FEATURE(isIdleStepper, pins.outputs.pinStepperStep, OUTPUT, configPage6.iacAlgorithm)
  pinStepperStep = pins.outputs.pinStepperStep;
  MATCH_PIN_TO_FEATURE(isIdleStepper, pins.outputs.pinStepperEnable, OUTPUT, configPage6.iacAlgorithm)
  pinStepperEnable = pins.outputs.pinStepperEnable;
  MATCH_PIN_TO_FEATURE(isIdleUpEnabled, pins.inputs.pinIdleUp, INPUT, configPage2.idleUpEnabled)
  pinIdleUp = pins.inputs.pinIdleUp;

  initialiseIdle(forcehoming);
}

void initialiseIdle(bool forcehoming)
{
  //By default, turn off the PWM interrupt (It gets turned on below if needed)
  IDLE_TIMER_DISABLE();

  //Initialising comprises of setting the 2D tables with the relevant values from the config pages
  switch(configPage6.iacAlgorithm)
  {
    case IAC_ALGORITHM_NONE:       
      //Case 0 is no idle control ('None')
      break;

    case IAC_ALGORITHM_ONOFF:
      //Case 1 is on/off idle control
      if ((currentStatus.coolant + CALIBRATION_TEMPERATURE_OFFSET) < configPage6.iacFastTemp)
      {
        setPin_High(pinIdle1);
        idleOn = true;
      }
      break;

    case IAC_ALGORITHM_PWM_OL:
      //Case 2 is PWM open loop
      idle_pwm_max_count = frequencyToTimerTicks(configPage6.idleFreq * 2U); //Converts the frequency in Hz to the number of ticks (at 16uS) it takes to complete 1 cycle. Note that the frequency is divided by 2 coming from TS to allow for up to 512hz
      enableIdle();
      break;

    case IAC_ALGORITHM_PWM_OLCL:
      //Case 6 is PWM closed loop with open loop table used as feed forward
      idle_pwm_max_count = frequencyToTimerTicks(configPage6.idleFreq * 2U); //Converts the frequency in Hz to the number of ticks (at 16uS) it takes to complete 1 cycle. Note that the frequency is divided by 2 coming from TS to allow for up to 512hz
      idlePID.SetOutputLimits(percentage(configPage2.iacCLminValue, idle_pwm_max_count<<2), percentage(configPage2.iacCLmaxValue, idle_pwm_max_count<<2));
      idlePID.SetTunings(configPage6.idleKP, configPage6.idleKI, configPage6.idleKD);
      idlePID.SetMode(AUTOMATIC); //Turn PID on
      idle_pid_target_value = 0;
      idlePID.Initialize();
      idleCounter = 0;

      break;

    case IAC_ALGORITHM_PWM_CL:
      //Case 3 is PWM closed loop
      idle_pwm_max_count = frequencyToTimerTicks(configPage6.idleFreq * 2U); //Converts the frequency in Hz to the number of ticks (at 16uS) it takes to complete 1 cycle. Note that the frequency is divided by 2 coming from TS to allow for up to 512hz
      idlePID.SetOutputLimits(percentage(configPage2.iacCLminValue, idle_pwm_max_count<<2), percentage(configPage2.iacCLmaxValue, idle_pwm_max_count<<2));
      idlePID.SetTunings(configPage6.idleKP, configPage6.idleKI, configPage6.idleKD);
      idlePID.SetMode(AUTOMATIC); //Turn PID on
      idle_pid_target_value = table2D_getValue(&iacCrankDutyTable, currentStatus.coolant + CALIBRATION_TEMPERATURE_OFFSET);
      idlePID.Initialize();
      idleCounter = 0;

      break;

    case IAC_ALGORITHM_STEP_OL:
      //Case 2 is Stepper open loop
      iacStepTime_uS = configPage6.iacStepTime * 1000;
      iacCoolTime_uS = configPage9.iacCoolTime * 1000;

      if (forcehoming)
      {
        //Change between modes running make engine stall
        completedHomeSteps = 0;
        idleStepper.curIdleStep = 0;
        idleStepper.stepperStatus = SOFF;
      }

      configPage6.iacPWMrun = false; // just in case. This needs to be false with stepper idle
      break;

    case IAC_ALGORITHM_STEP_CL:
      //Case 5 is Stepper closed loop
      iacStepTime_uS = configPage6.iacStepTime * 1000;
      iacCoolTime_uS = configPage9.iacCoolTime * 1000;

      if (forcehoming)
      {
        //Change between modes running make engine stall
        completedHomeSteps = 0;
        idleStepper.curIdleStep = 0;
        idleStepper.stepperStatus = SOFF;
      }

      idlePID.SetSampleTime(250); //4Hz means 250ms
      idlePID.SetOutputLimits((configPage2.iacCLminValue * 3)<<2, (configPage2.iacCLmaxValue * 3)<<2); //Maximum number of steps; always less than home steps count.
      idlePID.SetTunings(configPage6.idleKP, configPage6.idleKI, configPage6.idleKD);
      idlePID.SetMode(AUTOMATIC); //Turn PID on
      configPage6.iacPWMrun = false; // just in case. This needs to be false with stepper idle
      idle_pid_target_value = currentStatus.CLIdleTarget * 3;
      idlePID.Initialize();
      break;

    case IAC_ALGORITHM_STEP_OLCL:
      //Case 7 is Stepper closed loop with open loop table used as feed forward
      iacStepTime_uS = configPage6.iacStepTime * 1000;
      iacCoolTime_uS = configPage9.iacCoolTime * 1000;

      if (forcehoming)
      {
        //Change between modes running make engine stall
        completedHomeSteps = 0;
        idleStepper.curIdleStep = 0;
        idleStepper.stepperStatus = SOFF;
      }

      idlePID.SetSampleTime(250); //4Hz means 250ms
      idlePID.SetOutputLimits((configPage2.iacCLminValue * 3)<<2, (configPage2.iacCLmaxValue * 3)<<2); //Maximum number of steps; always less than home steps count.
      idlePID.SetTunings(configPage6.idleKP, configPage6.idleKI, configPage6.idleKD);
      idlePID.SetMode(AUTOMATIC); //Turn PID on
      configPage6.iacPWMrun = false; // just in case. This needs to be false with stepper idle
      idle_pid_target_value = 0;
      idlePID.Initialize();
      break;

    default:
      //Well this just shouldn't happen
      break;
  }

  initialiseIdleUpOutput();

  idleInitComplete = configPage6.iacAlgorithm; //Sets which idle method was initialised
  currentStatus.idleLoad = 0;
}

/*
Checks whether a step is currently underway or whether the motor is in 'cooling' state (ie whether it's ready to begin another step or not)
Returns:
True: If a step is underway or motor is 'cooling'
False: If the motor is ready for another step
*/
static inline byte checkForStepping(void)
{
  bool isStepping = false;
  unsigned int timeCheck;
  
  if( (idleStepper.stepperStatus == STEPPING) || (idleStepper.stepperStatus == COOLING) )
  {
    if (idleStepper.stepperStatus == STEPPING)
    {
      timeCheck = iacStepTime_uS;
    }
    else 
    {
      timeCheck = iacCoolTime_uS;
    }

    if(micros_safe() > (idleStepper.stepStartTime + timeCheck) )
    {         
      if(idleStepper.stepperStatus == STEPPING)
      {
        //Means we're currently in a step, but it needs to be turned off
        setPin_Low(pinStepperStep); //Turn off the step
        idleStepper.stepStartTime = micros_safe();
        
        // if there is no cool time we can miss that step out completely.
        if (iacCoolTime_uS > 0)
        {
          idleStepper.stepperStatus = COOLING; //'Cooling' is the time the stepper needs to sit in LOW state before the next step can be made
        }
        else
        {
          idleStepper.stepperStatus = SOFF;  
        }
          
        isStepping = true;
      }
      else
      {
        //Means we're in COOLING status but have been in this state long enough. Go into off state
        idleStepper.stepperStatus = SOFF;
        if(configPage9.iacStepperPower == STEPPER_POWER_WHEN_ACTIVE) 
        { 
          //Disable the DRV8825, but only if we're at the final step in this cycle. 
          if(idleStepper.targetIdleStep == idleStepper.curIdleStep) { setPin_High(pinStepperEnable); } 
        }
      }
    }
    else
    {
      //Means we're in a step, but it doesn't need to turn off yet. No further action at this time
      isStepping = true;
    }
  }
  return isStepping;
}

/*
Performs a step
*/
static inline void doStep(void)
{
  int16_t error = idleStepper.targetIdleStep - idleStepper.curIdleStep;
  if ( (error < -((int8_t)configPage6.iacStepHyster)) || (error > configPage6.iacStepHyster) ) //Hysteresis check
  {
    // the home position for a stepper is pintle fully seated, i.e. no airflow.
    if (error < 0)
    {
      // we are moving toward the home position (reducing air)
      setPinState(pinStepperDir, STEPPER_LESS_AIR_DIRECTION());
      idleStepper.curIdleStep--;
    }
    else
    {
      // we are moving away from the home position (adding air).
      setPinState(pinStepperDir, STEPPER_MORE_AIR_DIRECTION());
      idleStepper.curIdleStep++;
    }

    setPin_Low(pinStepperEnable); //Enable the DRV8825
    setPin_High(pinStepperStep);
    idleStepper.stepStartTime = micros_safe();
    idleStepper.stepperStatus = STEPPING;
    idleOn = true;

    BIT_SET(currentStatus.spark, BIT_SPARK_IDLE);
  }
  else
    BIT_CLEAR(currentStatus.spark, BIT_SPARK_IDLE);
}

/*
Checks whether the stepper has been homed yet. If it hasn't, will handle the next step
Returns:
True: If the system has been homed. No other action is taken
False: If the motor has not yet been homed. Will also perform another homing step.
*/
static inline byte isStepperHomed(void)
{
  bool isHomed = true; //As it's the most common scenario, default value is true
  if( completedHomeSteps < (configPage6.iacStepHome * 3) ) //Home steps are divided by 3 from TS
  {
    setPinState(pinStepperDir, STEPPER_LESS_AIR_DIRECTION()); //homing the stepper closes off the air bleed
    setPin_Low(pinStepperEnable); //Enable the DRV8825
    setPin_High(pinStepperStep);
    idleStepper.stepStartTime = micros_safe();
    idleStepper.stepperStatus = STEPPING;
    completedHomeSteps++;
    idleOn = true;
    isHomed = false;
  }
  return isHomed;
}

enum IdlePwmSignal {
  Positive,
  Negative,
};

static inline void setIdlePwmPinsState(IdlePwmSignal signal) {
  static_assert(HIGH==1 && LOW==0, "This only works when HIGH==1 && LOW==0");
  uint8_t pinSignal = signal==Positive ? !configPage6.iacPWMdir : configPage6.iacPWMdir;
  setPinState(pinIdle1, pinSignal);
  if(isIdle2PwmEnabled()) {
    setPinState(pinIdle2, !pinSignal); //If 2 idle channels are in use, flip idle2 to be the opposite of idle1
  }
}

void idleControl(void)
{
  if( idleInitComplete != configPage6.iacAlgorithm) { initialiseIdle(false); }
  if( (currentStatus.RPM > 0) || (configPage6.iacPWMrun == true) ) { enableIdle(); }

  //Check whether the idleUp is active
  if (isIdleUpEnabled())
  {
    currentStatus.idleUpActive = readPin(pinIdleUp)==configPage2.idleUpPolarity;
    if (isIdleUpOutputEnabled())
    {
      currentStatus.idleUpOutputActive = currentStatus.idleUpActive;
      setPinState( pinIdleUpOutput, 
                    currentStatus.idleUpOutputActive ? getIdleUpOutputHIGH() : getIdleUpOutputLOW());
    }
  }
  else { currentStatus.idleUpActive = false; }

  bool PID_computed = false;
  switch(configPage6.iacAlgorithm)
  {
    case IAC_ALGORITHM_NONE:       //Case 0 is no idle control ('None')
      break;

    case IAC_ALGORITHM_ONOFF:      //Case 1 is on/off idle control
      if ( (currentStatus.coolant + CALIBRATION_TEMPERATURE_OFFSET) < configPage6.iacFastTemp) //All temps are offset by 40 degrees
      {
        setPin_High(pinIdle1);
        idleOn = true;
        BIT_SET(currentStatus.spark, BIT_SPARK_IDLE); //Turn the idle control flag on
		    currentStatus.idleLoad = 100;
      }
      else if (idleOn)
      {
        setPin_Low(pinIdle1);
        idleOn = false; 
        BIT_CLEAR(currentStatus.spark, BIT_SPARK_IDLE); //Turn the idle control flag on
		    currentStatus.idleLoad = 0;
      }
      break;

    case IAC_ALGORITHM_PWM_OL:      //Case 2 is PWM open loop
      //Check for cranking pulsewidth
      if( BIT_CHECK(currentStatus.engine, BIT_ENGINE_CRANK) )
      {
        //Currently cranking. Use the cranking table
        currentStatus.idleLoad = table2D_getValue(&iacCrankDutyTable, currentStatus.coolant + CALIBRATION_TEMPERATURE_OFFSET); //All temps are offset by 40 degrees
        idleTaper = 0;
      }
      else if ( !BIT_CHECK(currentStatus.engine, BIT_ENGINE_RUN))
      {
        if( configPage6.iacPWMrun == true)
        {
          //Engine is not running or cranking, but the run before crank flag is set. Use the cranking table
          currentStatus.idleLoad = table2D_getValue(&iacCrankDutyTable, currentStatus.coolant + CALIBRATION_TEMPERATURE_OFFSET); //All temps are offset by 40 degrees
          idleTaper = 0;
        }
      }
      else
      {
        if ( idleTaper < configPage2.idleTaperTime )
        {
          //Tapering between cranking IAC value and running
          currentStatus.idleLoad = map(idleTaper, 0, configPage2.idleTaperTime,\
          table2D_getValue(&iacCrankDutyTable, currentStatus.coolant + CALIBRATION_TEMPERATURE_OFFSET),\
          table2D_getValue(&iacPWMTable, currentStatus.coolant + CALIBRATION_TEMPERATURE_OFFSET));
          if( BIT_CHECK(LOOP_TIMER, BIT_TIMER_10HZ) ) { idleTaper++; }
        }
        else
        {
          //Standard running
          currentStatus.idleLoad = table2D_getValue(&iacPWMTable, currentStatus.coolant + CALIBRATION_TEMPERATURE_OFFSET); //All temps are offset by 40 degrees
        }
        // Add air conditioning idle-up - we only do this if the engine is running (A/C should never engage with engine off).
        if(configPage15.airConIdleSteps>0 && BIT_CHECK(currentStatus.airConStatus, BIT_AIRCON_TURNING_ON) == true) { currentStatus.idleLoad += configPage15.airConIdleSteps; }
      }

      if(currentStatus.idleUpActive == true) { currentStatus.idleLoad += configPage2.idleUpAdder; } //Add Idle Up amount if active
      
      if( currentStatus.idleLoad > 100 ) { currentStatus.idleLoad = 100; } //Safety Check
      idle_pwm_target_value = percentage(currentStatus.idleLoad, idle_pwm_max_count);
      
      break;

    case IAC_ALGORITHM_PWM_CL:    //Case 3 is PWM closed loop
        //No cranking specific value for closed loop (yet?)
      if( BIT_CHECK(currentStatus.engine, BIT_ENGINE_CRANK) )
      {
        //Currently cranking. Use the cranking table
        currentStatus.idleLoad = table2D_getValue(&iacCrankDutyTable, currentStatus.coolant + CALIBRATION_TEMPERATURE_OFFSET); //All temps are offset by 40 degrees
        idle_pwm_target_value = percentage(currentStatus.idleLoad, idle_pwm_max_count);
        idle_pid_target_value = idle_pwm_target_value << 2; //Resolution increased
        idlePID.Initialize(); //Update output to smooth transition
      }
      else if ( !BIT_CHECK(currentStatus.engine, BIT_ENGINE_RUN))
      {
        if( configPage6.iacPWMrun == true)
        {
          //Engine is not running or cranking, but the run before crank flag is set. Use the cranking table
          currentStatus.idleLoad = table2D_getValue(&iacCrankDutyTable, currentStatus.coolant + CALIBRATION_TEMPERATURE_OFFSET); //All temps are offset by 40 degrees
          idle_pwm_target_value = percentage(currentStatus.idleLoad, idle_pwm_max_count);
        }
      }
      else
      {
        idle_cl_target_rpm = (uint16_t)currentStatus.CLIdleTarget * 10; //Multiply the byte target value back out by 10
        if( BIT_CHECK(LOOP_TIMER, BIT_TIMER_1HZ) ) { idlePID.SetTunings(configPage6.idleKP, configPage6.idleKI, configPage6.idleKD); } //Re-read the PID settings once per second
        
        PID_computed = idlePID.Compute(true);
        long TEMP_idle_pwm_target_value;
        if(PID_computed == true)
        {
          TEMP_idle_pwm_target_value = idle_pid_target_value;
          
          // Add an offset to the duty cycle, outside of the closed loop. When tuned correctly, the extra load from
          // the air conditioning should exactly cancel this out and the PID loop will be relatively unaffected.
          if(configPage15.airConIdleSteps>0 && BIT_CHECK(currentStatus.airConStatus, BIT_AIRCON_TURNING_ON) == true)
          {
            // Add air conditioning idle-up
            // We are adding percentage steps, but the loop doesn't operate in percentage steps - it works in PWM count
            TEMP_idle_pwm_target_value += percentage(configPage15.airConIdleSteps, idle_pwm_max_count<<2);
            if(TEMP_idle_pwm_target_value > (idle_pwm_max_count<<2)) { TEMP_idle_pwm_target_value = (idle_pwm_max_count<<2); }
          }

          // Fixed this by putting it here, however I have not tested it. It used to be after the calculation of idle_pwm_target_value, meaning the percentage would update in currentStatus, but the idle would not actually increase.
          if(currentStatus.idleUpActive == true)
          { 
            // Add Idle Up amount if active
            // Again, we use configPage15.airConIdleSteps * idle_pwm_max_count / 100 because we are adding percentage steps, but the loop doesn't operate in percentage steps - it works in PWM count
            TEMP_idle_pwm_target_value += percentage(configPage2.idleUpAdder, idle_pwm_max_count<<2);
            if(TEMP_idle_pwm_target_value > (idle_pwm_max_count<<2)) { TEMP_idle_pwm_target_value = (idle_pwm_max_count<<2); }
          }

          // Now assign the real PWM value
          idle_pwm_target_value = TEMP_idle_pwm_target_value>>2; //increased resolution
          currentStatus.idleLoad = udiv_32_16(idle_pwm_target_value * 100UL, idle_pwm_max_count);
        }
        idleCounter++;
      }
      break;


    case IAC_ALGORITHM_PWM_OLCL: //case 6 is PWM Open Loop table as feedforward term plus closed loop. 
      //No cranking specific value for closed loop (yet?)
      if( BIT_CHECK(currentStatus.engine, BIT_ENGINE_CRANK) )
      {
        //Currently cranking. Use the cranking table
        currentStatus.idleLoad = table2D_getValue(&iacCrankDutyTable, currentStatus.coolant + CALIBRATION_TEMPERATURE_OFFSET); //All temps are offset by 40 degrees
        idle_pwm_target_value = percentage(currentStatus.idleLoad, idle_pwm_max_count);
        idle_pid_target_value = idle_pwm_target_value << 2; //Resolution increased
        idlePID.Initialize(); //Update output to smooth transition
      }
      else if ( !BIT_CHECK(currentStatus.engine, BIT_ENGINE_RUN))
      {
        if( configPage6.iacPWMrun == true)
        {
          //Engine is not running or cranking, but the run before crank flag is set. Use the cranking table
          currentStatus.idleLoad = table2D_getValue(&iacCrankDutyTable, currentStatus.coolant + CALIBRATION_TEMPERATURE_OFFSET); //All temps are offset by 40 degrees
          idle_pwm_target_value = percentage(currentStatus.idleLoad, idle_pwm_max_count);
        }
      }
      else
      {
        //Read the OL table as feedforward term
        FeedForwardTerm = percentage(table2D_getValue(&iacPWMTable, currentStatus.coolant + CALIBRATION_TEMPERATURE_OFFSET), idle_pwm_max_count<<2); //All temps are offset by 40 degrees
        
        // Add an offset to the feed forward term. When tuned correctly, the extra load from the air conditioning
        // should exactly cancel this out and the PID loop will be relatively unaffected.
        if(configPage15.airConIdleSteps>0 && BIT_CHECK(currentStatus.airConStatus, BIT_AIRCON_TURNING_ON) == true)
        {
          // Add air conditioning idle-up
          // We are adding percentage steps, but the loop doesn't operate in percentage steps - it works in PWM count <<2 (PWM count * 4)
          FeedForwardTerm += percentage(configPage15.airConIdleSteps, (idle_pwm_max_count<<2));
          if(FeedForwardTerm > (idle_pwm_max_count<<2)) { FeedForwardTerm = (idle_pwm_max_count<<2); }
        }
        
        // Fixed this by putting it here, however I have not tested it. It used to be after the calculation of idle_pwm_target_value, meaning the percentage would update in currentStatus, but the idle would not actually increase.
        if(currentStatus.idleUpActive == true)
        { 
          // Add Idle Up amount if active
          // Again, we are adding percentage steps, but the loop doesn't operate in percentage steps - it works in PWM count <<2 (PWM count * 4)
          FeedForwardTerm += percentage(configPage2.idleUpAdder, (idle_pwm_max_count<<2));
          if(FeedForwardTerm > (idle_pwm_max_count<<2)) { FeedForwardTerm = (idle_pwm_max_count<<2); }
        }
        
    
        idle_cl_target_rpm = (uint16_t)currentStatus.CLIdleTarget * 10; //Multiply the byte target value back out by 10
        if( BIT_CHECK(LOOP_TIMER, BIT_TIMER_1HZ) ) { idlePID.SetTunings(configPage6.idleKP, configPage6.idleKI, configPage6.idleKD); } //Re-read the PID settings once per second
        if((currentStatus.RPM - idle_cl_target_rpm > configPage2.iacRPMlimitHysteresis*10) || (currentStatus.TPS > configPage2.iacTPSlimit)){ //reset integral to zero when TPS is bigger than set value in TS (opening throttle so not idle anymore). OR when RPM higher than Idle Target + RPM Histeresis (coming back from high rpm with throttle closed)
          idlePID.ResetIntegeral();
        }
        
        PID_computed = idlePID.Compute(true, FeedForwardTerm);

        if(PID_computed == true)
        {
          idle_pwm_target_value = idle_pid_target_value>>2; //increased resolution
          currentStatus.idleLoad = ((unsigned long)(idle_pwm_target_value * 100UL) / idle_pwm_max_count);
        }
        idleCounter++;
      }
        
    break;


    case IAC_ALGORITHM_STEP_OL:    //Case 4 is open loop stepper control
      //First thing to check is whether there is currently a step going on and if so, whether it needs to be turned off
      if( (checkForStepping() == false) && (isStepperHomed() == true) ) //Check that homing is complete and that there's not currently a step already taking place. MUST BE IN THIS ORDER!
      {
        //Check for cranking pulsewidth
        if( !BIT_CHECK(currentStatus.engine, BIT_ENGINE_RUN) ) //If ain't running it means off or cranking
        {
          //Currently cranking. Use the cranking table
          idleStepper.targetIdleStep = table2D_getValue(&iacCrankStepsTable, (currentStatus.coolant + CALIBRATION_TEMPERATURE_OFFSET)) * 3; //All temps are offset by 40 degrees. Step counts are divided by 3 in TS. Multiply back out here
          if(currentStatus.idleUpActive == true) { idleStepper.targetIdleStep += configPage2.idleUpAdder; } //Add Idle Up amount if active
          idleTaper = 0;
        }
        else
        {
          //Standard running
          if (BIT_CHECK(LOOP_TIMER, BIT_TIMER_10HZ) && (currentStatus.RPM > 0))
          {
            if ( idleTaper < configPage2.idleTaperTime )
            {
              //Tapering between cranking IAC value and running
              idleStepper.targetIdleStep = map(idleTaper, 0, configPage2.idleTaperTime,\
              table2D_getValue(&iacCrankStepsTable, (currentStatus.coolant + CALIBRATION_TEMPERATURE_OFFSET)) * 3,\
              table2D_getValue(&iacStepTable, (currentStatus.coolant + CALIBRATION_TEMPERATURE_OFFSET)) * 3);
              if( BIT_CHECK(LOOP_TIMER, BIT_TIMER_10HZ) ) { idleTaper++; }
            }
            else
            {
              //Standard running
              idleStepper.targetIdleStep = table2D_getValue(&iacStepTable, (currentStatus.coolant + CALIBRATION_TEMPERATURE_OFFSET)) * 3; //All temps are offset by 40 degrees. Step counts are divided by 3 in TS. Multiply back out here
            }
            if(currentStatus.idleUpActive == true) { idleStepper.targetIdleStep += configPage2.idleUpAdder; } //Add Idle Up amount if active
            
            // Add air conditioning idle-up - we only do this if the engine is running (A/C should never engage with engine off).
            if(configPage15.airConIdleSteps>0 && BIT_CHECK(currentStatus.airConStatus, BIT_AIRCON_TURNING_ON) == true) { idleStepper.targetIdleStep += configPage15.airConIdleSteps; }
            
            iacStepTime_uS = configPage6.iacStepTime * 1000;
            iacCoolTime_uS = configPage9.iacCoolTime * 1000;
          }
        }
        //limit to the configured max steps. This must include any idle up adder, to prevent over-opening.
        if (idleStepper.targetIdleStep > (configPage9.iacMaxSteps * 3) )
        {
          idleStepper.targetIdleStep = configPage9.iacMaxSteps * 3;
        }
        if( ((uint16_t)configPage9.iacMaxSteps * 3) > 255 ) { currentStatus.idleLoad = idleStepper.curIdleStep / 2; }//Current step count (Divided by 2 for byte)
        else { currentStatus.idleLoad = idleStepper.curIdleStep; }
        doStep();
      }
      break;

    case IAC_ALGORITHM_STEP_OLCL:  //Case 7 is closed+open loop stepper control
    case IAC_ALGORITHM_STEP_CL:    //Case 5 is closed loop stepper control
      //First thing to check is whether there is currently a step going on and if so, whether it needs to be turned off
      if( (checkForStepping() == false) && (isStepperHomed() == true) ) //Check that homing is complete and that there's not currently a step already taking place. MUST BE IN THIS ORDER!
      {
        if( !BIT_CHECK(currentStatus.engine, BIT_ENGINE_RUN) ) //If ain't running it means off or cranking
        {
          //Currently cranking. Use the cranking table
          idleStepper.targetIdleStep = table2D_getValue(&iacCrankStepsTable, (currentStatus.coolant + CALIBRATION_TEMPERATURE_OFFSET)) * 3; //All temps are offset by 40 degrees. Step counts are divided by 3 in TS. Multiply back out here
          if(currentStatus.idleUpActive == true) { idleStepper.targetIdleStep += configPage2.idleUpAdder; } //Add Idle Up amount if active

          //limit to the configured max steps. This must include any idle up adder, to prevent over-opening.
          if (idleStepper.targetIdleStep > (configPage9.iacMaxSteps * 3) )
          {
            idleStepper.targetIdleStep = configPage9.iacMaxSteps * 3;
          }
          
          idleTaper = 0;
          idle_pid_target_value = idleStepper.targetIdleStep << 2; //Resolution increased
          idlePID.ResetIntegeral();
          FeedForwardTerm = idle_pid_target_value;
        }
        else 
        {
          if( BIT_CHECK(LOOP_TIMER, BIT_TIMER_10HZ) )
          {
            idle_cl_target_rpm = (uint16_t)currentStatus.CLIdleTarget * 10; //Multiply the byte target value back out by 10
            if( idleTaper < configPage2.idleTaperTime )
            {
              uint16_t minValue = table2D_getValue(&iacCrankStepsTable, (currentStatus.coolant + CALIBRATION_TEMPERATURE_OFFSET)) * 3;
              if( idle_pid_target_value < minValue<<2 ) { idle_pid_target_value = minValue<<2; }
              uint16_t maxValue = idle_pid_target_value>>2;
              if( configPage6.iacAlgorithm == IAC_ALGORITHM_STEP_OLCL ) { maxValue = table2D_getValue(&iacStepTable, (currentStatus.coolant + CALIBRATION_TEMPERATURE_OFFSET)) * 3; }

              //Tapering between cranking IAC value and running
              FeedForwardTerm = map(idleTaper, 0, configPage2.idleTaperTime, minValue, maxValue)<<2;
              idleTaper++;
              idle_pid_target_value = FeedForwardTerm;
            }
            else if (configPage6.iacAlgorithm == IAC_ALGORITHM_STEP_OLCL)
            {
              //Standard running
              FeedForwardTerm = (table2D_getValue(&iacStepTable, (currentStatus.coolant + CALIBRATION_TEMPERATURE_OFFSET)) * 3)<<2; //All temps are offset by 40 degrees. Step counts are divided by 3 in TS. Multiply back out here
              //reset integral to zero when TPS is bigger than set value in TS (opening throttle so not idle anymore). OR when RPM higher than Idle Target + RPM Hysteresis (coming back from high rpm with throttle closed) 
              if (((currentStatus.RPM - idle_cl_target_rpm) > configPage2.iacRPMlimitHysteresis*10) || (currentStatus.TPS > configPage2.iacTPSlimit) || lastDFCOValue )
              {
                idlePID.ResetIntegeral();
              }
            }
            else { FeedForwardTerm = idle_pid_target_value; }
          }

          PID_computed = idlePID.Compute(true, FeedForwardTerm);

          //If DFCO conditions are met keep output from changing
          if( (currentStatus.TPS > configPage2.iacTPSlimit) || lastDFCOValue
          || ((configPage6.iacAlgorithm == IAC_ALGORITHM_STEP_OLCL) && (idleTaper < configPage2.idleTaperTime)) )
          {
            idle_pid_target_value = FeedForwardTerm;
          }
          idleStepper.targetIdleStep = idle_pid_target_value>>2; //Increase resolution

          // Add air conditioning idle-up - we only do this if the engine is running (A/C should never engage with engine off).
          if(configPage15.airConIdleSteps>0 && BIT_CHECK(currentStatus.airConStatus, BIT_AIRCON_TURNING_ON) == true) { idleStepper.targetIdleStep += configPage15.airConIdleSteps; }
        }
        
        if(currentStatus.idleUpActive == true) { idleStepper.targetIdleStep += configPage2.idleUpAdder; } //Add Idle Up amount if active
        
        //limit to the configured max steps. This must include any idle up adder, to prevent over-opening.
        if (idleStepper.targetIdleStep > (configPage9.iacMaxSteps * 3) )
        {
          idleStepper.targetIdleStep = configPage9.iacMaxSteps * 3;
        }
        if( ( (uint16_t)configPage9.iacMaxSteps * 3) > 255 ) { currentStatus.idleLoad = idleStepper.curIdleStep / 2; }//Current step count (Divided by 2 for byte)
        else { currentStatus.idleLoad = idleStepper.curIdleStep; }
        doStep();
      }
      if (BIT_CHECK(LOOP_TIMER, BIT_TIMER_1HZ)) //Use timer flag instead idle count
      {
        //This only needs to be run very infrequently, once per second
        idlePID.SetTunings(configPage6.idleKP, configPage6.idleKI, configPage6.idleKD);
        iacStepTime_uS = configPage6.iacStepTime * 1000;
        iacCoolTime_uS = configPage9.iacCoolTime * 1000;
      }
      break;

    default:
      //There really should be a valid idle type
      break;
  }
  lastDFCOValue = BIT_CHECK(currentStatus.status1, BIT_STATUS1_DFCO);

  //Check for 100% and 0% DC on PWM idle
  if( isIdlePwm() )
  {
    if(currentStatus.idleLoad >= 100)
    {
      BIT_SET(currentStatus.spark, BIT_SPARK_IDLE); //Turn the idle control flag on
      IDLE_TIMER_DISABLE();
      setIdlePwmPinsState(Positive);
    }
    else if (currentStatus.idleLoad == 0)
    {
      disableIdle();
    }
    else
    {
      BIT_SET(currentStatus.spark, BIT_SPARK_IDLE); //Turn the idle control flag on
      IDLE_TIMER_ENABLE();
    }
  }
}


//This function simply turns off the idle PWM and sets the pin low
void disableIdle(void)
{
  if( (configPage6.iacAlgorithm == IAC_ALGORITHM_PWM_CL) || (configPage6.iacAlgorithm == IAC_ALGORITHM_PWM_OL) )
  {
    IDLE_TIMER_DISABLE();
    setIdlePwmPinsState(Negative);
  }
  else if( isIdleStepper() )
  {
    //Only disable the stepper motor if homing is completed
    if( (checkForStepping() == false) && (isStepperHomed() == true) )
    {
        /* for open loop stepper we should just move to the cranking position when
           disabling idle, since the only time this function is called in this scenario
           is if the engine stops.
        */
        idleStepper.targetIdleStep = table2D_getValue(&iacCrankStepsTable, (currentStatus.coolant + CALIBRATION_TEMPERATURE_OFFSET)) * 3; //All temps are offset by 40 degrees. Step counts are divided by 3 in TS. Multiply back out here
        if(currentStatus.idleUpActive == true) { idleStepper.targetIdleStep += configPage2.idleUpAdder; } //Add Idle Up amount if active?

        //limit to the configured max steps. This must include any idle up adder, to prevent over-opening.
        if (idleStepper.targetIdleStep > (configPage9.iacMaxSteps * 3) )
        {
          idleStepper.targetIdleStep = configPage9.iacMaxSteps * 3;
        }
        idle_pid_target_value = idleStepper.targetIdleStep<<2;
    }
  }
  BIT_CLEAR(currentStatus.spark, BIT_SPARK_IDLE); //Turn the idle control flag off
  currentStatus.idleLoad = 0;
}

#if defined(CORE_AVR) //AVR chips use the ISR for this
ISR(TIMER1_COMPC_vect) //cppcheck-suppress misra-c2012-8.2
#else
void idleInterrupt(void) //Most ARM chips can simply call a function
#endif
{
  if (idle_pwm_state)
  {
    SET_COMPARE(IDLE_COMPARE, IDLE_COUNTER + (idle_pwm_max_count - idle_pwm_cur_value) );
  }
  else
  {
    SET_COMPARE(IDLE_COMPARE, IDLE_COUNTER + idle_pwm_target_value);
    idle_pwm_cur_value = idle_pwm_target_value;
  }

  #if defined (CORE_TEENSY41) //PIT TIMERS count down and have opposite effect on PWM
  setIdlePwmPinsState(idle_pwm_state ? Positive : Negative);
  #else
  setIdlePwmPinsState(idle_pwm_state ? Negative : Positive);
  #endif
  idle_pwm_state = !idle_pwm_state;
}
