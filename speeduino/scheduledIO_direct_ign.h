#pragma once

void initIgnDirectIO(const uint8_t (&pins)[IGN_CHANNELS]);

void coilCharging_DIRECT(uint8_t channel);
void coilStopCharging_DIRECT(uint8_t channel);
