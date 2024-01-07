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

#define SIMPLE_BOOST_P  1
#define SIMPLE_BOOST_I  1
#define SIMPLE_BOOST_D  1

extern ioPort vvt1_pin_port;
extern ioPort vvt2_pin_port;
extern ioPort fan_pin_port;

extern ioPort pump_pin_port;

#if(defined(CORE_TEENSY) || defined(CORE_STM32))
#define BOOST_PIN_LOW()         (setPinState(pinMapping.outputs.pinBoost, LOW))
#define BOOST_PIN_HIGH()        (setPinState(pinMapping.outputs.pinBoost, HIGH))
#define VVT1_PIN_LOW()          (setPinState(pinMapping.outputs.pinVVT_1, LOW))
#define VVT1_PIN_HIGH()         (setPinState(pinMapping.outputs.pinVVT_1, HIGH))
#define VVT2_PIN_LOW()          (setPinState(pinMapping.outputs.pinVVT_2, LOW))
#define VVT2_PIN_HIGH()         (setPinState(pinMapping.outputs.pinVVT_2, HIGH))
#define N2O_STAGE1_PIN_LOW()    (setPinState(configPage10.n2o_stage1_pin, LOW))
#define N2O_STAGE1_PIN_HIGH()   (setPinState(configPage10.n2o_stage1_pin, HIGH))
#define N2O_STAGE2_PIN_LOW()    (setPinState(configPage10.n2o_stage2_pin, LOW))
#define N2O_STAGE2_PIN_HIGH()   (setPinState(configPage10.n2o_stage2_pin, HIGH))
#define FUEL_PUMP_ON()          (setPinState(pinMapping.outputs.pinFuelPump, HIGH))
#define FUEL_PUMP_OFF()         (setPinState(pinMapping.outputs.pinFuelPump, LOW))

#define AIRCON_ON()             { setPinState(pinMapping.outputs.pinAirConComp, !(configPage15.airConCompPol)); BIT_SET(currentStatus.airConStatus, BIT_AIRCON_COMPRESSOR); }
#define AIRCON_OFF()            { setPinState(pinMapping.outputs.pinAirConComp,  (configPage15.airConCompPol)); BIT_CLEAR(currentStatus.airConStatus, BIT_AIRCON_COMPRESSOR); }
#define AIRCON_FAN_ON()         { setPinState(pinMapping.outputs.pinAirConFan, !(configPage15.airConFanPol)); BIT_SET(currentStatus.airConStatus, BIT_AIRCON_FAN); }
#define AIRCON_FAN_OFF()        { setPinState(pinMapping.outputs.pinAirConFan,  (configPage15.airConFanPol)); BIT_CLEAR(currentStatus.airConStatus, BIT_AIRCON_FAN); }

#define FAN_ON()                { setPinState(pinMapping.outputs.pinFan, !(configPage6.fanInv)); }
#define FAN_OFF()               { setPinState(pinMapping.outputs.pinFan,  (configPage6.fanInv)); }
#else
#define BOOST_PIN_LOW()         ATOMIC_BLOCK(ATOMIC_RESTORESTATE) { setPin_Low(boost_pin_port); }
#define BOOST_PIN_HIGH()        ATOMIC_BLOCK(ATOMIC_RESTORESTATE) { setPin_High(boost_pin_port);  }
#define VVT1_PIN_LOW()          ATOMIC_BLOCK(ATOMIC_RESTORESTATE) { setPin_Low(vvt1_pin_port);   }
#define VVT1_PIN_HIGH()         ATOMIC_BLOCK(ATOMIC_RESTORESTATE) { setPin_High(vvt1_pin_port);    }
#define VVT2_PIN_LOW()          ATOMIC_BLOCK(ATOMIC_RESTORESTATE) { setPin_Low(vvt2_pin_port);   }
#define VVT2_PIN_HIGH()         ATOMIC_BLOCK(ATOMIC_RESTORESTATE) { setPin_High(vvt2_pin_port);    }
#define N2O_STAGE1_PIN_LOW()    ATOMIC_BLOCK(ATOMIC_RESTORESTATE) { setPin_Low(n2o_stage1_pin_port);  }
#define N2O_STAGE1_PIN_HIGH()   ATOMIC_BLOCK(ATOMIC_RESTORESTATE) { setPin_High(n2o_stage1_pin_port);   }
#define N2O_STAGE2_PIN_LOW()    ATOMIC_BLOCK(ATOMIC_RESTORESTATE) { setPin_Low(n2o_stage2_pin_port);  }
#define N2O_STAGE2_PIN_HIGH()   ATOMIC_BLOCK(ATOMIC_RESTORESTATE) { setPin_High(n2o_stage2_pin_port);   }
#define FUEL_PUMP_ON()          ATOMIC_BLOCK(ATOMIC_RESTORESTATE) { setPin_High(pump_pin_port); }
#define FUEL_PUMP_OFF()         ATOMIC_BLOCK(ATOMIC_RESTORESTATE) { setPin_Low(pump_pin_port); }

#define AIRCON_ON()             ATOMIC_BLOCK(ATOMIC_RESTORESTATE) { setPinState(aircon_comp_pin_port, !(configPage15.airConCompPol)); BIT_SET(currentStatus.airConStatus, BIT_AIRCON_COMPRESSOR); }
#define AIRCON_OFF()            ATOMIC_BLOCK(ATOMIC_RESTORESTATE) { setPinState(aircon_comp_pin_port,  (configPage15.airConCompPol)); BIT_CLEAR(currentStatus.airConStatus, BIT_AIRCON_COMPRESSOR); }
#define AIRCON_FAN_ON()         ATOMIC_BLOCK(ATOMIC_RESTORESTATE) { setPinState(aircon_fan_pin_port, !(configPage15.airConFanPol)); BIT_SET(currentStatus.airConStatus, BIT_AIRCON_FAN); }
#define AIRCON_FAN_OFF()        ATOMIC_BLOCK(ATOMIC_RESTORESTATE) { setPinState(aircon_fan_pin_port,  (configPage15.airConFanPol)); BIT_CLEAR(currentStatus.airConStatus, BIT_AIRCON_FAN); }

#define FAN_ON()                ATOMIC_BLOCK(ATOMIC_RESTORESTATE) { setPinState(fan_pin_port, !(configPage6.fanInv)); }
#define FAN_OFF()               ATOMIC_BLOCK(ATOMIC_RESTORESTATE) { setPinState(fan_pin_port,  (configPage6.fanInv)); }
#endif

#define READ_N2O_ARM_PIN()    (readPin(n2o_arming_pin_port)==HIGH)

#define VVT1_ON()     VVT1_PIN_HIGH();
#define VVT1_OFF()    VVT1_PIN_LOW();
#define VVT2_ON()     VVT2_PIN_HIGH();
#define VVT2_OFF()    VVT2_PIN_LOW();
#define VVT_TIME_DELAY_MULTIPLIER  50

#define WMI_TANK_IS_EMPTY() (isWMIEmptyEnabled() ? digitalRead(pinMapping.inputs.pinWMIEmpty)==configPage10.wmiEmptyPolarity : true)

#if defined(PWM_FAN_AVAILABLE)//PWM fan not available on Arduino MEGA
extern uint16_t fan_pwm_max_count; //Used for variable PWM frequency
void fanInterrupt(void);
#endif

extern uint16_t vvt_pwm_max_count; //Used for variable PWM frequency
extern uint16_t boost_pwm_max_count; //Used for variable PWM frequency

#endif