#include "acc_mc33810.h"

#if defined(OUTPUT_CONTROL_SUPPORTED)

mc33810_IC_t mc33810_1;
mc33810_IC_t mc33810_2;

#define MC33810_1_ACTIVE() setPin_Low(mc33810_1.pin);
#define MC33810_1_INACTIVE() setPin_High(mc33810_1.pin);
#define MC33810_2_ACTIVE() setPin_Low(mc33810_2.pin);
#define MC33810_2_INACTIVE() setPin_High(mc33810_2.pin);

void initMC33810(const mc33810_config_t &ic1, const mc33810_config_t &ic2)
{
    //Set the output states of both ICs to be off to fuel and ignition
    mc33810_1.stateBits = 0U;
    mc33810_1.pin = pinToOutputPort(ic1.pin);
    mc33810_1.config = ic1;
    mc33810_2.stateBits = 0U;
    mc33810_2.pin = pinToOutputPort(ic2.pin);
    mc33810_2.config = ic2;

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
    MC33810_1_ACTIVE();
    SPI.transfer16(cmd);
    MC33810_1_INACTIVE();
    //IC2
    MC33810_2_ACTIVE();
    SPI.transfer16(cmd);
    MC33810_2_INACTIVE();

    //Disable the Open Load pull-down current sync (See page 31 of MC33810 DS)
    /*
    0010 = LSD Fault Command
    1000 = LSD Fault operation is Shutdown (Default)
    1111 = Open load detection fault when active (Default)
    0000 = Disable open load detection when off (Changed from 1111 to 0000)
    */
    cmd = 0b0010100011110000;
    //IC1
    MC33810_1_ACTIVE();
    SPI.transfer16(cmd);
    MC33810_1_INACTIVE();
    //IC2
    MC33810_2_ACTIVE();
    SPI.transfer16(cmd);
    MC33810_2_INACTIVE();
    
    if( (LED_BUILTIN != SCK) && (LED_BUILTIN != MOSI) && (LED_BUILTIN != MISO) ) { 
        pinMode(LED_BUILTIN, OUTPUT); //This is required on as the LED pin can otherwise be reset to an input
    }
}

#endif