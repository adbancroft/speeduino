#ifndef CRANKMATHS_H
#define CRANKMATHS_H

#include "maths.h"
#include "bit_shifts.h"

/**
 * @file
 * 
 * @brief Crank revolution based mathmatical functions. 
 * 
 */

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
static constexpr uint16_t MIN_REVOLUTION_TIME = (uint16_t)(MICROS_PER_MIN/MAX_RPM);

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
static inline uint16_t injectorLimits(int16_t angle)
{
    if (unlikely(angle < 0)) { angle = angle + CRANK_ANGLE_MAX_INJ; }
    while(angle > CRANK_ANGLE_MAX_INJ ) { angle -= CRANK_ANGLE_MAX_INJ; }
    return angle;
}

/// @cond 
// Private definitions - not for use external to the crank math code

namespace _crank_math_detail {
    typedef uint32_t UQ24X8_t;
    static constexpr uint8_t UQ24X8_Shift = 8U;
    static constexpr uint8_t microsPerDegree_Shift = UQ24X8_Shift;
    extern UQ24X8_t microsPerDegree;

    typedef uint16_t UQ1X15_t;
    static constexpr uint8_t UQ1X15_Shift = 15U;
    static constexpr uint8_t degreesPerMicro_Shift = UQ1X15_Shift;
    extern UQ1X15_t degreesPerMicro;
}

/// @endcond

/**
 * @brief Set the revolution time, from which some of the degree<-->angle conversions are derived
 * 
 * @param revolutionTime The crank revolution time in uS
 */
static inline void setAngleConverterRevolutionTime(uint32_t revolutionTime) {
    _crank_math_detail::microsPerDegree = div360(lshift<_crank_math_detail::microsPerDegree_Shift>(revolutionTime));
    _crank_math_detail::degreesPerMicro = (uint16_t)UDIV_ROUND_CLOSEST(lshift<_crank_math_detail::degreesPerMicro_Shift>(UINT32_C(360)), revolutionTime, uint32_t);  
}

/**
 * @brief Converts angular degrees to the time interval that amount of rotation will take at current RPM.
 * 
 * Based on angle of [0,720] and min/max RPM, result ranges from 9 (MAX_RPM, 1 deg) to 2926828 (MIN_RPM, 720 deg)
 *
 * @param angle Angle in degrees
 * @return Time interval in uS
 */
static inline uint32_t angleToTimeMicroSecPerDegree(uint16_t angle) {
    _crank_math_detail::UQ24X8_t micros = (uint32_t)angle * (uint32_t)_crank_math_detail::microsPerDegree;
    return rshift_round<_crank_math_detail::microsPerDegree_Shift>(micros);
}

/**
 * @brief Converts a time interval to the equivalent degrees of angular (crank) rotation at current RPM.
 *
 * Inverse of angleToTimeMicroSecPerDegree
 *
 * @param time Time interval in uS
 * @return Angle in degrees
 */
static inline uint16_t timeToAngleDegPerMicroSec(uint32_t time){
    uint32_t degFixed = time * (uint32_t)_crank_math_detail::degreesPerMicro;
    return rshift_round<_crank_math_detail::degreesPerMicro_Shift>(degFixed);
}

#endif