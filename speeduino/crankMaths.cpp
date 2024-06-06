#include "crankMaths.h"
#include "bit_shifts.h"

#define SECOND_DERIV_ENABLED                0          

//These are only part of the experimental 2nd deriv calcs
#if SECOND_DERIV_ENABLED!=0
byte deltaToothCount = 0; //The last tooth that was used with the deltaV calc
int rpmDelta;
#endif

/** @brief µS per degree at current RPM in UQ24.8 fixed point 
 * 
 * Ranges between 
 *   * 1040649 (4065.039 µS/°) at MIN_RPM/MIN_REVOLUTION_TIME
 *   * 2370    (9.258333 µS/°) at MAX_RPM/MAX_REVOLUTION_TIME
*/
crank_math_detail::UQ24X8_t microsPerDegree; // cppcheck-suppress misra-c2012-8.4 ; To allow inlining in header

/** @brief Degrees per µS in UQ1.15 fixed point.
 * 
 * Ranges between 
 *   * 8    (0.000246 °/µS) at MIN_RPM/MIN_REVOLUTION_TIME
 *   * 3539 (0.108011 °/µS) at MAX_RPM/MAX_REVOLUTION_TIME
 */
crank_math_detail::UQ1X15_t degreesPerMicro; // cppcheck-suppress misra-c2012-8.4 ; To allow inlining in header

/** @brief The time in µS that one revolution would take at current speed */
uint32_t revolutionTime = MIN_REVOLUTION_TIME; // cppcheck-suppress misra-c2012-8.4 ; To allow inlining in header

bool setRevolutionTime(uint32_t revTime) {
  bool hasChanged = (revTime!=revolutionTime) && (revTime>=MIN_REVOLUTION_TIME) && (revTime<MAX_REVOLUTION_TIME);
  
  if (hasChanged) {
    revolutionTime = revTime;
    microsPerDegree = div360(lshift<crank_math_detail::microsPerDegree_Shift>(revTime));
    degreesPerMicro = (uint16_t)UDIV_ROUND_CLOSEST(lshift<crank_math_detail::degreesPerMicro_Shift>(UINT32_C(360)), revTime, uint32_t);
  }

  return hasChanged;
}

#if SECOND_DERIV_ENABLED!=0
void doCrankSpeedCalcs(void)
{
     //********************************************************
      //How fast are we going? Need to know how long (uS) it will take to get from one tooth to the next. We then use that to estimate how far we are between the last tooth and the next one
      //We use a 1st Deriv acceleration prediction, but only when there is an even spacing between primary sensor teeth
      //Any decoder that has uneven spacing has its triggerToothAngle set to 0
      //THIS IS CURRENTLY DISABLED FOR ALL DECODERS! It needs more work. 
      if( (BIT_CHECK(decoderState, BIT_DECODER_2ND_DERIV)) && (toothHistoryIndex >= 3) && (currentStatus.RPM < 2000) ) //toothHistoryIndex must be greater than or equal to 3 as we need the last 3 entries. Currently this mode only runs below 3000 rpm
      {
        //Only recalculate deltaV if the tooth has changed since last time (DeltaV stays the same until the next tooth)
        //if (deltaToothCount != toothCurrentCount)
        {
          deltaToothCount = toothCurrentCount;
          int angle1, angle2; //These represent the crank angles that are travelled for the last 2 pulses
          if(configPage4.TrigPattern == 4)
          {
            //Special case for 70/110 pattern on 4g63
            angle2 = triggerToothAngle; //Angle 2 is the most recent
            if (angle2 == 70) { angle1 = 110; }
            else { angle1 = 70; }
          }
          else if(configPage4.TrigPattern == 0)
          {
            //Special case for missing tooth decoder where the missing tooth was one of the last 2 seen
            if(toothCurrentCount == 1) { angle2 = 2*triggerToothAngle; angle1 = triggerToothAngle; }
            else if(toothCurrentCount == 2) { angle1 = 2*triggerToothAngle; angle2 = triggerToothAngle; }
            else { angle1 = triggerToothAngle; angle2 = triggerToothAngle; }
          }
          else { angle1 = triggerToothAngle; angle2 = triggerToothAngle; }

          uint32_t toothDeltaV = (MICROS_PER_SEC * angle2 / toothHistory[toothHistoryIndex]) - (MICROS_PER_SEC * angle1 / toothHistory[toothHistoryIndex-1]);
          uint32_t toothDeltaT = toothHistory[toothHistoryIndex];
          //long timeToLastTooth = micros() - toothLastToothTime;

          rpmDelta = lshift<10>(toothDeltaV) / (6 * toothDeltaT);
        }

          timePerDegreex16 = ldiv( 2666656L, currentStatus.RPM + rpmDelta).quot; //This gives accuracy down to 0.1 of a degree and can provide noticeably better timing results on low resolution triggers
      }
}
#endif
