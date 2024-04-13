#ifndef CRANKMATHS_H
#define CRANKMATHS_H

#include "maths.h"
#include "globals.h"

/**
 * @brief Makes one pass at nudging the angle to within [0,CRANK_ANGLE_MAX_IGN]
 * 
 * @param angle A crank angle in degrees
 * @return int16_t 
 */
static inline int16_t ignitionLimits(int16_t angle)
{
    return nudge(0, CRANK_ANGLE_MAX_IGN, angle, CRANK_ANGLE_MAX_IGN);
}

/**
 * @brief Makes one pass at nudging the angle to within [0,CRANK_ANGLE_MAX_INJ]
 * 
 * @param angle A crank angle in degrees
 * @return int16_t 
 */
static inline int16_t injectorLimits(int16_t angle)
{
    int16_t tempAngle = angle;
    if(tempAngle < 0) { tempAngle = tempAngle + CRANK_ANGLE_MAX_INJ; }
    while(tempAngle > CRANK_ANGLE_MAX_INJ ) { tempAngle -= CRANK_ANGLE_MAX_INJ; }
    return tempAngle;
}

/** @brief At 1 RPM, each degree of angular rotation takes this many microseconds */
static constexpr uint32_t MICROS_PER_DEG_1_RPM = MICROS_PER_MIN/360UL;

/** @brief The maximum rpm that the ECU will attempt to run at. 
 * 
 * It is NOT related to the rev limiter, but is instead dictates how fast certain operations will be
 * allowed to run. Lower number gives better performance 
 **/
static constexpr uint16_t MAX_RPM = 18000U;

/** @brief Absolute minimum RPM that the crank math (& therefore all of Speeduino) can be used with 
 * 
 * This is dictated by the use of uint16_t as the base type for storing
 * angle<->time conversion factor (degreesPerMicro)
*/
static constexpr uint16_t MIN_RPM = (MICROS_PER_DEG_1_RPM/(UINT16_MAX/16U))+1U;

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
 * @name Converts angular degrees to the time interval that amount of rotation
 * will take at current RPM.
 * 
 * Based on angle of [0,720] and min/max RPM, result ranges from
 * 9 (MAX_RPM, 1 deg) to 2926828 (MIN_RPM, 720 deg)
 *
 * @param angle Angle in degrees
 * @return Time interval in uS
 */
///@{
/** @brief Converts based on the time one degree of rotation takes 
 * 
 * Inverse of timeToAngleDegPerMicroSec
*/
uint32_t angleToTimeMicroSecPerDegree(uint16_t angle);

/** @brief Converts based on the time interval between the 2 most recently detected decoder teeth 
 * 
 * Inverse of timeToAngleIntervalTooth
*/
uint32_t angleToTimeIntervalTooth(uint16_t angle);
///@}

/**
 * @name Converts a time interval in microsecods to the equivalent degrees of angular (crank)
 * rotation at current RPM.
 *
 * @param time Time interval in uS
 * @return Angle in degrees
 */
///@{
/** @brief Converts based on the the interval on time one degree of rotation takes 
 * 
 * Inverse of angleToTimeMicroSecPerDegree
*/
uint16_t timeToAngleDegPerMicroSec(uint32_t time);

/** @brief Converts based on the time interval between the 2 most recently detected decoder teeth 
 * 
 * Inverse of angleToTimeIntervalTooth
*/
uint16_t timeToAngleIntervalTooth(uint32_t time);
///@}

#endif