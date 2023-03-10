#pragma once

#include <stdint.h>
#include "scheduler.h"

static inline uint16_t calculateInjectorStartAngle(uint16_t PWdivTimerPerDegree, int16_t injChannelDegrees, uint16_t injAngle);

static inline uint32_t calculateInjectorTimeout(const FuelSchedule &schedule, int16_t injectorStartAngle, int16_t crankAngle);

static inline void calculateIgnitionAngles(IgnitionSchedule &schedule, int16_t dwellAngle, int8_t advance);

// Ignition for rotary.
static inline void calculateIgnitionTrailingRotary(IgnitionSchedule &leading, int16_t dwellAngle, int16_t rotarySplitDegrees, IgnitionSchedule &trailing);

static inline uint32_t calculateIgnitionTimeout(const IgnitionSchedule &schedule, int16_t crankAngle);

static inline void setIgnitionSchedule(IgnitionSchedule &schedule, int16_t crankAngle, uint32_t dwellDuration) {
  // Do not override the per-tooth timing
  if (schedule.Status!=PENDING_WITH_OVERRIDE) {
    uint32_t delay = calculateIgnitionTimeout(schedule, crankAngle);

    if (delay > 0U) {
      _setSchedule(schedule, delay, dwellDuration);
    }
  }
}
#include "schedule_calcs.hpp"
