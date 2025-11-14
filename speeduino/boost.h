#pragma once

#include <stdint.h>

void initialiseBoost(void);
void boostControl(void);
void boostDisable(void);
void boostInterrupt(void);

extern uint16_t boost_pwm_max_count; //Used for variable PWM frequency

