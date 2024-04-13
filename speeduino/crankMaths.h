#ifndef CRANKMATHS_H
#define CRANKMATHS_H

/**
 * @file
 * 
 * @brief Crank revolution based mathemtical functions. 
 * 
 */

#include "maths.h"

/** @brief At 1 RPM, each degree of angular rotation takes this many microseconds */
static constexpr uint32_t MICROS_PER_DEG_1_RPM = UDIV_ROUND_CLOSEST(MICROS_PER_MIN, 360UL, uint32_t);

/** @brief The maximum rpm that the ECU will attempt to run at. 
 * 
 * It is NOT related to the rev limiter, but is instead dictates how fast certain operations will be
 * allowed to run. Lower number gives better performance 
 **/
static constexpr uint16_t MAX_RPM = 18000U;

/** @brief Absolute minimum RPM that the crank math (& therefore all of Speeduino) can be used with.
 * 
 * This is dictated by the use of uint16_t as the base type for storing
 * time --> angle conversion factor (degreesPerMicro)
*/
static constexpr uint16_t MIN_RPM = (uint16_t)UDIV_ROUND_UP(MICROS_PER_DEG_1_RPM, (uint32_t)UINT16_MAX/16UL, uint32_t);

/**
 * @brief Minumum time in µS that one crank revolution can take.
 * 
 * @note: many calculations are done over 2 revolutions (cycles), in which case this would be doubled 
 */
static constexpr uint16_t MIN_REVOLUTION_TIME = MICROS_PER_MIN/MAX_RPM;

/**
 * @brief Maximum time in µS that one crank revolution can take.
 * 
 * @note: many calculations are done over 2 revolutions (cycles), in which case this would be doubled 
 */
static constexpr uint32_t MAX_REVOLUTION_TIME = MICROS_PER_MIN/MIN_RPM;

/**
 * @brief Makes one pass at nudging the angle to within [0,CRANK_ANGLE_MAX_IGN]
 * 
 * @param angle A crank angle in degrees
 * @return int16_t 
 */
static inline int16_t ignitionLimits(int16_t angle) {
    return nudge(0, CRANK_ANGLE_MAX_IGN-1, angle, CRANK_ANGLE_MAX_IGN);
}

/**
 * @brief Makes one pass at nudging the angle to within [0,CRANK_ANGLE_MAX_INJ]
 * 
 * @param angle A crank angle in degrees
 * @return int16_t 
 */
static inline uint16_t injectorLimits(uint16_t angle)
{
    while(angle > (uint16_t)CRANK_ANGLE_MAX_INJ ) { angle -= (uint16_t)CRANK_ANGLE_MAX_INJ; }
    return angle;
}

/**
 * @brief Set the revolution time, from which the degree<-->angle conversions are derived
 * 
 * If the return value is true, then the calculated RPM (@ref rpmFromRevolutionTime) and 
 * degree<-->angle conversions (@ref angleToTimeMicroSecPerDegree, @ref timeToAngleDegPerMicroSec) will change.
 * 
 * @param revTime The crank revolution time in µS. *Must be between MIN_REVOLUTION_TIME and MAX_REVOLUTION_TIME*
 * @return true If the revolution time has changed
 * @return false If the new time is the same as the current revolution time.
 */
bool setRevolutionTime(uint32_t revTime, statuses &current);

/**
 * @brief Get the latest revolution time
 * 
 * @return uint32_t The revolution time, limited to [MIN_REVOLUTION_TIME, MAX_REVOLUTION_TIME)
 */
static inline uint32_t getRevolutionTime(const statuses &current) {
  return current.revolutionTime;
}


/**
 * @brief Converts angular degrees to the time interval that amount of rotation
 * will take at the current crank revolution time (@ref setRevolutionTime).
 * 
 * Based on angle of [0,720] and min/max RPM, result ranges from
 * 9 (MAX_RPM, 1 deg) to 2926828 (MIN_RPM, 720 deg). I.e. 24 bits
 *
 * @param angle Angle in degrees
 * @return Time interval in µS
 */
uint32_t angleToTimeMicroSecPerDegree(uint16_t angle);

/**
 * @brief Converts a time interval in µS to the equivalent degrees of angular (crank)
 * rotation at the current crank revolution time (@ref setRevolutionTime).
 *
 * Inverse of angleToTimeMicroSecPerDegree
 *
 * @param time Time interval in µS
 * @return Angle in degrees
 */
uint16_t timeToAngleDegPerMicroSec(uint32_t time);

/** @brief Calculate RPM based on the current crank revolution time (@ref setRevolutionTime). */

/**
 * @brief Calculate RPM based on the revolution time
 * 
 * @param revTime Time for one 360° rotation in µS. Typically the result of a call to getRevolutionTime()
 * @return uint16_t 
 */
static inline uint16_t rpmFromRevolutionTime(uint32_t revTime) {
  if (revTime<=(uint32_t)UINT16_MAX) {
    return udiv_32_16_closest(MICROS_PER_MIN, revTime);
  } else {
    return UDIV_ROUND_CLOSEST(MICROS_PER_MIN, revTime, uint32_t);
  }
}

#endif