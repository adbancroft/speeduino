#ifndef AUX_H
#define AUX_H

#include "atomic.h"
#include "globals.h"
#include "port_pin.h"
#include "pin_mapping.h"

void initialiseAuxPWM(const pin_mapping_t &pins);

void boostInterrupt(void);
void boostControl(void);
void boostDisable(void);
static inline bool isBoostEnabled(void) {
  return configPage6.boostEnabled==1U;
}

void vvtControl(void);
static inline bool isVVT_1Enabled(void) {
  return configPage6.vvtEnabled != 0U;
}
static inline bool isVVT_2Enabled(void) {
  return isVVT_1Enabled() && configPage10.vvt2Enabled != 0U;
}
void vvtInterrupt(void);

#define FAN_MODE_OFF                    0U
#define FAN_MODE_ONOFF                  1U
#define FAN_MODE_PWM                    2U

void initialiseFan(const pin_mapping_t &pins);
void fanControl(void);
static inline bool isFanEnabled(void) {
  return configPage2.fanEnable != FAN_MODE_OFF;
}

void nitrousControl(void);
static inline bool isNitrousEnabled(void) {
  return configPage10.n2o_enable != 0U;
}

void initialiseAirCon(const pin_mapping_t &pins);
void airConControl(void);
static inline bool isAirConEnabled(void) {
  return configPage15.airConEnable !=0U;
}
static inline bool isAirConFanEnabled(void) {
  return isAirConEnabled() && (configPage15.airConFanEnabled > 0U);
}


void initialiseWmi(const pin_mapping_t &pins);
void wmiControl(void);
void setWmiIndicator(void);
void toggleWmiIndicator(void);
static inline bool isWMIEnabled(void) {
  return configPage10.wmiEnabled != 0U;
}
static inline bool isWMIIndicatorEnabled(void) {
  return isWMIEnabled() && configPage10.wmiIndicatorEnabled != 0U;
}
static inline bool isWMIEmptyEnabled(void) {
  return isWMIEnabled() && configPage10.wmiEmptyEnabled != 0U;
}

void initialiseFuelPump(const pin_mapping_t &pins);
void beginPrimeFuelPump(void);
void FUEL_PUMP_ON(void);
void FUEL_PUMP_OFF(void);

void FAN_ON(void);
void FAN_OFF(void);

void VVT1_ON(void);
void VVT1_OFF(void);
void VVT2_ON(void);
void VVT2_OFF(void);

#if defined(PWM_FAN_AVAILABLE)//PWM fan not available on Arduino MEGA
void fanInterrupt(void);
#endif

#endif