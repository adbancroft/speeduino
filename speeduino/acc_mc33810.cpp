#include <SPI.h>
#include "acc_mc33810.h"
#include "src/pins/fastOutputPin.h"
#include "globals.h"

static uint8_t MC33810_BIT_INJ[8] = {};
static uint8_t MC33810_BIT_IGN[8] = {};

static constexpr uint8_t MC33810_ONOFF_CMD = 0x30; //48 in decimal

struct mc33810_t
{
    fastOutputPin_t pin;
    volatile uint8_t requestedState; //Current binary state of the 2nd ICs IGN and INJ values
    volatile uint8_t returnState; //Current binary state of the 1st ICs IGN and INJ values
};

static mc33810_t mc33810_1;
static mc33810_t mc33810_2;

void setMC33810_1_ACTIVE(void) { mc33810_1.pin.setPinHigh(); }
void setMC33810_1_INACTIVE(void) { mc33810_1.pin.setPinLow(); }
void setMC33810_2_ACTIVE(void) { mc33810_2.pin.setPinHigh(); }
void setMC33810_2_INACTIVE(void) { mc33810_2.pin.setPinLow(); }

void __attribute__((optimize("Os"))) initMC33810(uint8_t pinMC33810_1, uint8_t pinMC33810_2,
                                                 const uint8_t (&injBits)[8], const uint8_t (&ignBits)[8])
{
    static_assert(sizeof(MC33810_BIT_INJ)==sizeof(injBits), "Mismatch!");
    memcpy(MC33810_BIT_INJ, injBits, sizeof(MC33810_BIT_INJ));
    static_assert(sizeof(MC33810_BIT_IGN)==sizeof(ignBits), "Mismatch!");
    memcpy(MC33810_BIT_IGN, ignBits, sizeof(MC33810_BIT_IGN));

    //Set pin port/masks
    mc33810_1.pin.setPin(pinMC33810_1, OUTPUT);
    mc33810_2.pin.setPin(pinMC33810_2, OUTPUT);

    //Set the output states of both ICs to be off to fuel and ignition
    mc33810_1.requestedState = 0;
    mc33810_2.requestedState = 0;
    mc33810_1.returnState = 0;
    mc33810_2.returnState = 0;

    SPI.begin();
    //These are the SPI settings per the datasheet
	SPI.beginTransaction(SPISettings(6000000, MSBFIRST, SPI_MODE0)); 

    //Set the ignition outputs to GPGD mode
    /*
    0001 = Mode select command
    1111 = Set all 1 GD[0...3] outputs to use GPGD mode
    00000000 = All remaining values are unused (For us)
    */
    //uint16_t cmd = 0b000111110000;
    uint16_t cmd = 0b0001111100000000;
    //IC1
    setMC33810_1_ACTIVE();
    SPI.transfer16(cmd);
    setMC33810_1_INACTIVE();
    //IC2
    setMC33810_2_ACTIVE();
    SPI.transfer16(cmd);
    setMC33810_2_INACTIVE();

    //Disable the Open Load pull-down current sync (See page 31 of MC33810 DS)
    /*
    0010 = LSD Fault Command
    1000 = LSD Fault operation is Shutdown (Default)
    1111 = Open load detection fault when active (Default)
    0000 = Disable open load detection when off (Changed from 1111 to 0000)
    */
    cmd = 0b0010100011110000;
    //IC1
    setMC33810_1_ACTIVE();
    SPI.transfer16(cmd);
    setMC33810_1_INACTIVE();
    //IC2
    setMC33810_2_ACTIVE();
    SPI.transfer16(cmd);
    setMC33810_2_INACTIVE();
}

void openInjector1_MC33810(void) { setMC33810_1_ACTIVE(); BIT_SET(mc33810_1.requestedState, MC33810_BIT_INJ[0]); mc33810_1.returnState = SPI.transfer16(word(MC33810_ONOFF_CMD, mc33810_1.requestedState)); setMC33810_1_INACTIVE(); }
void openInjector2_MC33810(void) { setMC33810_1_ACTIVE(); BIT_SET(mc33810_1.requestedState, MC33810_BIT_INJ[1]); mc33810_1.returnState = SPI.transfer16(word(MC33810_ONOFF_CMD, mc33810_1.requestedState)); setMC33810_1_INACTIVE(); }
void openInjector3_MC33810(void) { setMC33810_1_ACTIVE(); BIT_SET(mc33810_1.requestedState, MC33810_BIT_INJ[2]); mc33810_1.returnState = SPI.transfer16(word(MC33810_ONOFF_CMD, mc33810_1.requestedState)); setMC33810_1_INACTIVE(); }
void openInjector4_MC33810(void) { setMC33810_1_ACTIVE(); BIT_SET(mc33810_1.requestedState, MC33810_BIT_INJ[3]); mc33810_1.returnState = SPI.transfer16(word(MC33810_ONOFF_CMD, mc33810_1.requestedState)); setMC33810_1_INACTIVE(); }
void openInjector5_MC33810(void) { setMC33810_2_ACTIVE(); BIT_SET(mc33810_2.requestedState, MC33810_BIT_INJ[4]); mc33810_2.returnState = SPI.transfer16(word(MC33810_ONOFF_CMD, mc33810_2.requestedState)); setMC33810_2_INACTIVE(); }
void openInjector6_MC33810(void) { setMC33810_2_ACTIVE(); BIT_SET(mc33810_2.requestedState, MC33810_BIT_INJ[5]); mc33810_2.returnState = SPI.transfer16(word(MC33810_ONOFF_CMD, mc33810_2.requestedState)); setMC33810_2_INACTIVE(); }
void openInjector7_MC33810(void) { setMC33810_2_ACTIVE(); BIT_SET(mc33810_2.requestedState, MC33810_BIT_INJ[6]); mc33810_2.returnState = SPI.transfer16(word(MC33810_ONOFF_CMD, mc33810_2.requestedState)); setMC33810_2_INACTIVE(); }
void openInjector8_MC33810(void) { setMC33810_2_ACTIVE(); BIT_SET(mc33810_2.requestedState, MC33810_BIT_INJ[7]); mc33810_2.returnState = SPI.transfer16(word(MC33810_ONOFF_CMD, mc33810_2.requestedState)); setMC33810_2_INACTIVE(); }

void closeInjector1_MC33810(void) { setMC33810_1_ACTIVE(); BIT_CLEAR(mc33810_1.requestedState, MC33810_BIT_INJ[0]); mc33810_1.returnState = SPI.transfer16(word(MC33810_ONOFF_CMD, mc33810_1.requestedState)); setMC33810_1_INACTIVE(); }
void closeInjector2_MC33810(void) { setMC33810_1_ACTIVE(); BIT_CLEAR(mc33810_1.requestedState, MC33810_BIT_INJ[1]); mc33810_1.returnState = SPI.transfer16(word(MC33810_ONOFF_CMD, mc33810_1.requestedState)); setMC33810_1_INACTIVE(); }
void closeInjector3_MC33810(void) { setMC33810_1_ACTIVE(); BIT_CLEAR(mc33810_1.requestedState, MC33810_BIT_INJ[2]); mc33810_1.returnState = SPI.transfer16(word(MC33810_ONOFF_CMD, mc33810_1.requestedState)); setMC33810_1_INACTIVE(); }
void closeInjector4_MC33810(void) { setMC33810_1_ACTIVE(); BIT_CLEAR(mc33810_1.requestedState, MC33810_BIT_INJ[3]); mc33810_1.returnState = SPI.transfer16(word(MC33810_ONOFF_CMD, mc33810_1.requestedState)); setMC33810_1_INACTIVE(); }
void closeInjector5_MC33810(void) { setMC33810_2_ACTIVE(); BIT_CLEAR(mc33810_2.requestedState, MC33810_BIT_INJ[4]); mc33810_2.returnState = SPI.transfer16(word(MC33810_ONOFF_CMD, mc33810_2.requestedState)); setMC33810_2_INACTIVE(); }
void closeInjector6_MC33810(void) { setMC33810_2_ACTIVE(); BIT_CLEAR(mc33810_2.requestedState, MC33810_BIT_INJ[5]); mc33810_2.returnState = SPI.transfer16(word(MC33810_ONOFF_CMD, mc33810_2.requestedState)); setMC33810_2_INACTIVE(); }
void closeInjector7_MC33810(void) { setMC33810_2_ACTIVE(); BIT_CLEAR(mc33810_2.requestedState, MC33810_BIT_INJ[6]); mc33810_2.returnState = SPI.transfer16(word(MC33810_ONOFF_CMD, mc33810_2.requestedState)); setMC33810_2_INACTIVE(); }
void closeInjector8_MC33810(void) { setMC33810_2_ACTIVE(); BIT_CLEAR(mc33810_2.requestedState, MC33810_BIT_INJ[7]); mc33810_2.returnState = SPI.transfer16(word(MC33810_ONOFF_CMD, mc33810_2.requestedState)); setMC33810_2_INACTIVE(); }

void coil1High_MC33810(void) { setMC33810_1_ACTIVE(); BIT_SET(mc33810_1.requestedState, MC33810_BIT_IGN[0]); mc33810_1.returnState = SPI.transfer16(word(MC33810_ONOFF_CMD, mc33810_1.requestedState)); setMC33810_1_INACTIVE(); }
void coil2High_MC33810(void) { setMC33810_1_ACTIVE(); BIT_SET(mc33810_1.requestedState, MC33810_BIT_IGN[1]); mc33810_1.returnState = SPI.transfer16(word(MC33810_ONOFF_CMD, mc33810_1.requestedState)); setMC33810_1_INACTIVE(); }
void coil3High_MC33810(void) { setMC33810_1_ACTIVE(); BIT_SET(mc33810_1.requestedState, MC33810_BIT_IGN[2]); mc33810_1.returnState = SPI.transfer16(word(MC33810_ONOFF_CMD, mc33810_1.requestedState)); setMC33810_1_INACTIVE(); }
void coil4High_MC33810(void) { setMC33810_1_ACTIVE(); BIT_SET(mc33810_1.requestedState, MC33810_BIT_IGN[3]); mc33810_1.returnState = SPI.transfer16(word(MC33810_ONOFF_CMD, mc33810_1.requestedState)); setMC33810_1_INACTIVE(); }
void coil5High_MC33810(void) { setMC33810_2_ACTIVE(); BIT_SET(mc33810_2.requestedState, MC33810_BIT_IGN[4]); mc33810_2.returnState = SPI.transfer16(word(MC33810_ONOFF_CMD, mc33810_2.requestedState)); setMC33810_2_INACTIVE(); }
void coil6High_MC33810(void) { setMC33810_2_ACTIVE(); BIT_SET(mc33810_2.requestedState, MC33810_BIT_IGN[5]); mc33810_2.returnState = SPI.transfer16(word(MC33810_ONOFF_CMD, mc33810_2.requestedState)); setMC33810_2_INACTIVE(); }
void coil7High_MC33810(void) { setMC33810_2_ACTIVE(); BIT_SET(mc33810_2.requestedState, MC33810_BIT_IGN[6]); mc33810_2.returnState = SPI.transfer16(word(MC33810_ONOFF_CMD, mc33810_2.requestedState)); setMC33810_2_INACTIVE(); }
void coil8High_MC33810(void) { setMC33810_2_ACTIVE(); BIT_SET(mc33810_2.requestedState, MC33810_BIT_IGN[7]); mc33810_2.returnState = SPI.transfer16(word(MC33810_ONOFF_CMD, mc33810_2.requestedState)); setMC33810_2_INACTIVE(); }

void coil1Low_MC33810(void) { setMC33810_1_ACTIVE(); BIT_CLEAR(mc33810_1.requestedState, MC33810_BIT_IGN[0]); mc33810_1.returnState = SPI.transfer16(word(MC33810_ONOFF_CMD, mc33810_1.requestedState)); setMC33810_1_INACTIVE(); }
void coil2Low_MC33810(void) { setMC33810_1_ACTIVE(); BIT_CLEAR(mc33810_1.requestedState, MC33810_BIT_IGN[1]); mc33810_1.returnState = SPI.transfer16(word(MC33810_ONOFF_CMD, mc33810_1.requestedState)); setMC33810_1_INACTIVE(); }
void coil3Low_MC33810(void) { setMC33810_1_ACTIVE(); BIT_CLEAR(mc33810_1.requestedState, MC33810_BIT_IGN[2]); mc33810_1.returnState = SPI.transfer16(word(MC33810_ONOFF_CMD, mc33810_1.requestedState)); setMC33810_1_INACTIVE(); }
void coil4Low_MC33810(void) { setMC33810_1_ACTIVE(); BIT_CLEAR(mc33810_1.requestedState, MC33810_BIT_IGN[3]); mc33810_1.returnState = SPI.transfer16(word(MC33810_ONOFF_CMD, mc33810_1.requestedState)); setMC33810_1_INACTIVE(); }
void coil5Low_MC33810(void) { setMC33810_2_ACTIVE(); BIT_CLEAR(mc33810_2.requestedState, MC33810_BIT_IGN[4]); mc33810_2.returnState = SPI.transfer16(word(MC33810_ONOFF_CMD, mc33810_2.requestedState)); setMC33810_2_INACTIVE(); }
void coil6Low_MC33810(void) { setMC33810_2_ACTIVE(); BIT_CLEAR(mc33810_2.requestedState, MC33810_BIT_IGN[5]); mc33810_2.returnState = SPI.transfer16(word(MC33810_ONOFF_CMD, mc33810_2.requestedState)); setMC33810_2_INACTIVE(); }
void coil7Low_MC33810(void) { setMC33810_2_ACTIVE(); BIT_CLEAR(mc33810_2.requestedState, MC33810_BIT_IGN[6]); mc33810_2.returnState = SPI.transfer16(word(MC33810_ONOFF_CMD, mc33810_2.requestedState)); setMC33810_2_INACTIVE(); }
void coil8Low_MC33810(void) { setMC33810_2_ACTIVE(); BIT_CLEAR(mc33810_2.requestedState, MC33810_BIT_IGN[7]); mc33810_2.returnState = SPI.transfer16(word(MC33810_ONOFF_CMD, mc33810_2.requestedState)); setMC33810_2_INACTIVE(); }

void coil1Charging_MC33810(void)      { if(configPage4.IgInv == GOING_HIGH) { coil1Low_MC33810();  } else { coil1High_MC33810(); } }
void coil1StopCharging_MC33810(void)  { if(configPage4.IgInv == GOING_HIGH) { coil1High_MC33810(); } else { coil1Low_MC33810();  } }
void coil2Charging_MC33810(void)      { if(configPage4.IgInv == GOING_HIGH) { coil2Low_MC33810();  } else { coil2High_MC33810(); } }
void coil2StopCharging_MC33810(void)  { if(configPage4.IgInv == GOING_HIGH) { coil2High_MC33810(); } else { coil2Low_MC33810();  } }
void coil3Charging_MC33810(void)      { if(configPage4.IgInv == GOING_HIGH) { coil3Low_MC33810();  } else { coil3High_MC33810(); } }
void coil3StopCharging_MC33810(void)  { if(configPage4.IgInv == GOING_HIGH) { coil3High_MC33810(); } else { coil3Low_MC33810();  } }
void coil4Charging_MC33810(void)      { if(configPage4.IgInv == GOING_HIGH) { coil4Low_MC33810();  } else { coil4High_MC33810(); } }
void coil4StopCharging_MC33810(void)  { if(configPage4.IgInv == GOING_HIGH) { coil4High_MC33810(); } else { coil4Low_MC33810();  } }
void coil5Charging_MC33810(void)      { if(configPage4.IgInv == GOING_HIGH) { coil5Low_MC33810();  } else { coil5High_MC33810(); } }
void coil5StopCharging_MC33810(void)  { if(configPage4.IgInv == GOING_HIGH) { coil5High_MC33810(); } else { coil5Low_MC33810();  } }
void coil6Charging_MC33810(void)      { if(configPage4.IgInv == GOING_HIGH) { coil6Low_MC33810();  } else { coil6High_MC33810(); } }
void coil6StopCharging_MC33810(void)  { if(configPage4.IgInv == GOING_HIGH) { coil6High_MC33810(); } else { coil6Low_MC33810();  } }
void coil7Charging_MC33810(void)      { if(configPage4.IgInv == GOING_HIGH) { coil7Low_MC33810();  } else { coil7High_MC33810(); } }
void coil7StopCharging_MC33810(void)  { if(configPage4.IgInv == GOING_HIGH) { coil7High_MC33810(); } else { coil7Low_MC33810();  } }
void coil8Charging_MC33810(void)      { if(configPage4.IgInv == GOING_HIGH) { coil8Low_MC33810();  } else { coil8High_MC33810(); } }
void coil8StopCharging_MC33810(void)  { if(configPage4.IgInv == GOING_HIGH) { coil8High_MC33810(); } else { coil8Low_MC33810();  } }
