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

#define BOOST_PIN_LOW()         ATOMIC_BLOCK(ATOMIC_RESTORESTATE) { setPin_Low(pinMapping.outputs.pinBoost); }
#define BOOST_PIN_HIGH()        ATOMIC_BLOCK(ATOMIC_RESTORESTATE) { setPin_High(pinMapping.outputs.pinBoost);  }
#define N2O_STAGE1_PIN_LOW()    ATOMIC_BLOCK(ATOMIC_RESTORESTATE) { setPin_Low(configPage10.n2o_stage1_pin);  }
#define N2O_STAGE1_PIN_HIGH()   ATOMIC_BLOCK(ATOMIC_RESTORESTATE) { setPin_High(configPage10.n2o_stage1_pin);   }
#define N2O_STAGE2_PIN_LOW()    ATOMIC_BLOCK(ATOMIC_RESTORESTATE) { setPin_Low(configPage10.n2o_stage2_pin);  }
#define N2O_STAGE2_PIN_HIGH()   ATOMIC_BLOCK(ATOMIC_RESTORESTATE) { setPin_High(configPage10.n2o_stage2_pin);   }
#define FUEL_PUMP_ON()          ATOMIC_BLOCK(ATOMIC_RESTORESTATE) { setPin_High(pinMapping.outputs.pinFuelPump); }
#define FUEL_PUMP_OFF()         ATOMIC_BLOCK(ATOMIC_RESTORESTATE) { setPin_Low(pinMapping.outputs.pinFuelPump); }

#define AIRCON_ON()             ATOMIC_BLOCK(ATOMIC_RESTORESTATE) { setPinState(pinMapping.outputs.pinAirConComp, !(configPage15.airConCompPol)); BIT_SET(currentStatus.airConStatus, BIT_AIRCON_COMPRESSOR); }
#define AIRCON_OFF()            ATOMIC_BLOCK(ATOMIC_RESTORESTATE) { setPinState(pinMapping.outputs.pinAirConComp,  (configPage15.airConCompPol)); BIT_CLEAR(currentStatus.airConStatus, BIT_AIRCON_COMPRESSOR); }
#define AIRCON_FAN_ON()         ATOMIC_BLOCK(ATOMIC_RESTORESTATE) { setPinState(pinMapping.outputs.pinAirConFan, !(configPage15.airConFanPol)); BIT_SET(currentStatus.airConStatus, BIT_AIRCON_FAN); }
#define AIRCON_FAN_OFF()        ATOMIC_BLOCK(ATOMIC_RESTORESTATE) { setPinState(pinMapping.outputs.pinAirConFan,  (configPage15.airConFanPol)); BIT_CLEAR(currentStatus.airConStatus, BIT_AIRCON_FAN); }

#define FAN_ON()                ATOMIC_BLOCK(ATOMIC_RESTORESTATE) { setPinState(pinMapping.outputs.pinFan, !(configPage6.fanInv)); }
#define FAN_OFF()               ATOMIC_BLOCK(ATOMIC_RESTORESTATE) { setPinState(pinMapping.outputs.pinFan,  (configPage6.fanInv)); }

#define READ_N2O_ARM_PIN()    (readPin(configPage10.n2o_arming_pin)==HIGH)

#define VVT1_ON()               ATOMIC_BLOCK(ATOMIC_RESTORESTATE) { setPin_High(pinMapping.outputs.pinVVT_1); }
#define VVT1_OFF()              ATOMIC_BLOCK(ATOMIC_RESTORESTATE) { setPin_Low(pinMapping.outputs.pinVVT_1);  }
#define VVT2_ON()               ATOMIC_BLOCK(ATOMIC_RESTORESTATE) { setPin_High(pinMapping.outputs.pinVVT_2); }
#define VVT2_OFF()              ATOMIC_BLOCK(ATOMIC_RESTORESTATE) { setPin_Low(pinMapping.outputs.pinVVT_2);  }
#define VVT_TIME_DELAY_MULTIPLIER  50

#define WMI_TANK_IS_EMPTY() (isWMIEmptyEnabled() ? readPin(pinMapping.inputs.pinWMIEmpty)==configPage10.wmiEmptyPolarity : true)

#if defined(PWM_FAN_AVAILABLE)//PWM fan not available on Arduino MEGA
void fanInterrupt(void);
#endif

#endif