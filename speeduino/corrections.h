/*
All functions in the gamma file return

*/
#ifndef CORRECTIONS_H
#define CORRECTIONS_H

#include <stdint.h>
using byte = uint8_t;

void initialiseCorrections(void);
uint16_t correctionsFuel(void);

int8_t correctionsIgn(int8_t advance);
int8_t correctionCrankingFixedTiming(int8_t advance);

uint16_t correctionsDwell(uint16_t dwell);

static inline int8_t correctionFixedTiming(int8_t advance) {
  int8_t ignFixValue = advance;
  if (configPage2.fixAngEnable == 1U) { ignFixValue = configPage4.FixAng; } //Check whether the user has set a fixed timing angle
  return ignFixValue;    
}

extern uint16_t AFRnextCycle;
extern unsigned long knockStartTime;

#endif // CORRECTIONS_H
