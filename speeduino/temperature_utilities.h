#pragma once

#include <stdint.h>

/**
 * @brief All temperature measurements are stored offset by 40 degrees.
 * This is so we can use an unsigned byte (0-255) to represent temperature ranges from -40 to 215
 */
#define CALIBRATION_TEMPERATURE_OFFSET 40

/**
 * @brief Convert from a working temperature (-40, 215) to storage (0, 255)
 * 
 * @param temp Working temperature (-40, 215)
 */
static inline constexpr uint8_t toStorageTemperature(int16_t temp) {
    return temp>-CALIBRATION_TEMPERATURE_OFFSET ? 
                temp + CALIBRATION_TEMPERATURE_OFFSET : 0U;
}

/**
 * @brief Convert from a storage temperature (0, 255) to working (-40, 215)
 * 
 * @param temp Storage temperature (0, 255)
 */
static inline constexpr int16_t toWorkingTemperature(uint8_t temp) {
    return (int16_t)temp - CALIBRATION_TEMPERATURE_OFFSET;
}