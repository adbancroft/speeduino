#pragma once
#include "pin_mapping.h"

void initialiseSecondaryTables(const pin_mapping_t &pins);

void calculateSecondaryFuel(void);
static inline bool isPinFuel2InputEnabled(void) {
  return configPage10.fuel2Mode == FUEL2_MODE_INPUT_SWITCH;
}

void calculateSecondarySpark(void);
static inline bool isPinSpark2InputEnabled(void) {
  return configPage10.spark2Mode == SPARK2_MODE_INPUT_SWITCH;
}
