#include "acc_mc33810.h"

#if defined(OUTPUT_CONTROL_SUPPORTED)

uint8_t MC33810_BIT_INJ1 = 1;
uint8_t MC33810_BIT_INJ2 = 2;
uint8_t MC33810_BIT_INJ3 = 3;
uint8_t MC33810_BIT_INJ4 = 4;
uint8_t MC33810_BIT_INJ5 = 5;
uint8_t MC33810_BIT_INJ6 = 6;
uint8_t MC33810_BIT_INJ7 = 7;
uint8_t MC33810_BIT_INJ8 = 8;

uint8_t MC33810_BIT_IGN1 = 1;
uint8_t MC33810_BIT_IGN2 = 2;
uint8_t MC33810_BIT_IGN3 = 3;
uint8_t MC33810_BIT_IGN4 = 4;
uint8_t MC33810_BIT_IGN5 = 5;
uint8_t MC33810_BIT_IGN6 = 6;
uint8_t MC33810_BIT_IGN7 = 7;
uint8_t MC33810_BIT_IGN8 = 8;

byte pinMC33810_1_CS;
byte pinMC33810_2_CS;
ioPort portMC33810_1_CS;
ioPort portMC33810_2_CS;
volatile uint8_t mc33810_1_requestedState; //Current binary state of the 1st ICs IGN and INJ values
volatile uint8_t mc33810_2_requestedState; //Current binary state of the 2nd ICs IGN and INJ values

#define MC33810_1_ACTIVE() setPin_Low(portMC33810_1_CS);
#define MC33810_1_INACTIVE() setPin_High(portMC33810_1_CS);
#define MC33810_2_ACTIVE() setPin_Low(portMC33810_2_CS);
#define MC33810_2_INACTIVE() setPin_High(portMC33810_2_CS);

void initMC33810(void)
{
    //Set the output states of both ICs to be off to fuel and ignition
    mc33810_1_requestedState = 0;
    mc33810_2_requestedState = 0;

    portMC33810_1_CS = pinToOutputPort(pinMC33810_1_CS);
    portMC33810_2_CS = pinToOutputPort(pinMC33810_2_CS);

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