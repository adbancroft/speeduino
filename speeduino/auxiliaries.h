#ifndef AUX_H
#define AUX_H

#include <SimplyAtomic.h>
#include "board_definition.h"
#include "port_pin.h"

void initialiseAuxPWM(void);
void boostControl(void);
void boostDisable(void);
void boostByGear(void);
void vvtControl(void);
void initialiseFan(void);
void initialiseAirCon(void);
void nitrousControl(void);
void fanControl(void);
void airConControl(void);
bool READ_AIRCON_REQUEST(void);
void wmiControl(void);

#define SIMPLE_BOOST_P  1
#define SIMPLE_BOOST_I  1
#define SIMPLE_BOOST_D  1

#if(defined(CORE_TEENSY) || defined(CORE_STM32))
#define BOOST_PIN_LOW()         (setPinState(pinBoost, LOW))
#define BOOST_PIN_HIGH()        (setPinState(pinBoost, HIGH))
#define VVT1_PIN_LOW()          (setPinState(pinVVT_1, LOW))
#define VVT1_PIN_HIGH()         (setPinState(pinVVT_1, HIGH))
#define VVT2_PIN_LOW()          (setPinState(pinVVT_2, LOW))
#define VVT2_PIN_HIGH()         (setPinState(pinVVT_2, HIGH))
#define N2O_STAGE1_PIN_LOW()    (setPinState(configPage10.n2o_stage1_pin, LOW))
#define N2O_STAGE1_PIN_HIGH()   (setPinState(configPage10.n2o_stage1_pin, HIGH))
#define N2O_STAGE2_PIN_LOW()    (setPinState(configPage10.n2o_stage2_pin, LOW))
#define N2O_STAGE2_PIN_HIGH()   (setPinState(configPage10.n2o_stage2_pin, HIGH))
#define FUEL_PUMP_ON()          (setPinState(pinFuelPump, HIGH))
#define FUEL_PUMP_OFF()         (setPinState(pinFuelPump, LOW))

#define AIRCON_ON()             { setPinState(pinAirConComp, !(configPage15.airConCompPol)); BIT_SET(currentStatus.airConStatus, BIT_AIRCON_COMPRESSOR); }
#define AIRCON_OFF()            { setPinState(pinAirConComp,  (configPage15.airConCompPol)); BIT_CLEAR(currentStatus.airConStatus, BIT_AIRCON_COMPRESSOR); }
#define AIRCON_FAN_ON()         { setPinState(pinAirConFan, !(configPage15.airConFanPol)); BIT_SET(currentStatus.airConStatus, BIT_AIRCON_FAN); }
#define AIRCON_FAN_OFF()        { setPinState(pinAirConFan,  (configPage15.airConFanPol)); BIT_CLEAR(currentStatus.airConStatus, BIT_AIRCON_FAN); }

#define FAN_ON()                { setPinState(pinFan, !(configPage6.fanInv)); }
#define FAN_OFF()               { setPinState(pinFan,  (configPage6.fanInv)); }
#else
#define BOOST_PIN_LOW()         ATOMIC() { setPin_Low(boost_pin_port); }
#define BOOST_PIN_HIGH()        ATOMIC() { setPin_High(boost_pin_port);  }
#define VVT1_PIN_LOW()          ATOMIC() { extern ioPort vvt1_pin_port; setPin_Low(vvt1_pin_port);   }
#define VVT1_PIN_HIGH()         ATOMIC() { extern ioPort vvt1_pin_port; setPin_High(vvt1_pin_port);    }
#define VVT2_PIN_LOW()          ATOMIC() { extern ioPort vvt2_pin_port; setPin_Low(vvt2_pin_port);   }
#define VVT2_PIN_HIGH()         ATOMIC() { extern ioPort vvt2_pin_port; setPin_High(vvt2_pin_port);    }
#define N2O_STAGE1_PIN_LOW()    ATOMIC() { setPin_Low(n2o_stage1_pin_port);  }
#define N2O_STAGE1_PIN_HIGH()   ATOMIC() { setPin_High(n2o_stage1_pin_port);   }
#define N2O_STAGE2_PIN_LOW()    ATOMIC() { setPin_Low(n2o_stage2_pin_port);  }
#define N2O_STAGE2_PIN_HIGH()   ATOMIC() { setPin_High(n2o_stage2_pin_port);   }
#define FUEL_PUMP_ON()          ATOMIC() { setPin_High(pump_pin_port); }
#define FUEL_PUMP_OFF()         ATOMIC() { setPin_Low(pump_pin_port); }

#define AIRCON_ON()             ATOMIC() { setPinState(aircon_comp_pin_port, !(configPage15.airConCompPol)); BIT_SET(currentStatus.airConStatus, BIT_AIRCON_COMPRESSOR); }
#define AIRCON_OFF()            ATOMIC() { setPinState(aircon_comp_pin_port,  (configPage15.airConCompPol)); BIT_CLEAR(currentStatus.airConStatus, BIT_AIRCON_COMPRESSOR); }
#define AIRCON_FAN_ON()         ATOMIC() { setPinState(aircon_fan_pin_port, !(configPage15.airConFanPol)); BIT_SET(currentStatus.airConStatus, BIT_AIRCON_FAN); }
#define AIRCON_FAN_OFF()        ATOMIC() { setPinState(aircon_fan_pin_port,  (configPage15.airConFanPol)); BIT_CLEAR(currentStatus.airConStatus, BIT_AIRCON_FAN); }

#define FAN_ON()                ATOMIC() { extern ioPort fan_pin_port; setPinState(fan_pin_port, !(configPage6.fanInv)); }
#define FAN_OFF()               ATOMIC() { extern ioPort fan_pin_port; setPinState(fan_pin_port,  (configPage6.fanInv)); }
#endif

#define READ_N2O_ARM_PIN()    (readPin(n2o_arming_pin_port)==HIGH)

#define VVT1_ON()     VVT1_PIN_HIGH();
#define VVT1_OFF()    VVT1_PIN_LOW();
#define VVT2_ON()     VVT2_PIN_HIGH();
#define VVT2_OFF()    VVT2_PIN_LOW();
#define VVT_TIME_DELAY_MULTIPLIER  50

#define WMI_TANK_IS_EMPTY() ((configPage10.wmiEmptyEnabled) ? ((configPage10.wmiEmptyPolarity) ? digitalRead(pinWMIEmpty) : !digitalRead(pinWMIEmpty)) : 1)


#if defined(PWM_FAN_AVAILABLE)//PWM fan not available on Arduino MEGA
extern uint16_t fan_pwm_max_count; //Used for variable PWM frequency
void fanInterrupt(void);
#endif

extern uint16_t vvt_pwm_max_count; //Used for variable PWM frequency
extern uint16_t boost_pwm_max_count; //Used for variable PWM frequency

void boostInterrupt(void);
void vvtInterrupt(void);

#endif