#ifndef MC33810_H
#define MC33810_H

void initMC33810(   uint8_t pinMC33810_1, uint8_t pinMC33810_2,
                    const uint8_t (&injBits)[8],
                    const uint8_t (&ignBits)[8]);

void openInjector_MC33810(uint8_t channel);
void closeInjector_MC33810(uint8_t channel);

void coilCharging_MC33810(uint8_t channel);
void coilStopCharging_MC33810(uint8_t channel);

#endif