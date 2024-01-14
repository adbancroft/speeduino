#ifndef DECODERS_H
#define DECODERS_H

#include "decoder_trigger_types.h"

#define DECODER_MISSING_TOOTH     0
#define DECODER_BASIC_DISTRIBUTOR 1
#define DECODER_DUAL_WHEEL        2
#define DECODER_GM7X              3
#define DECODER_4G63              4
#define DECODER_24X               5
#define DECODER_JEEP2000          6
#define DECODER_AUDI135           7
#define DECODER_HONDA_D17         8
#define DECODER_MIATA_9905        9
#define DECODER_MAZDA_AU          10
#define DECODER_NON360            11
#define DECODER_NISSAN_360        12
#define DECODER_SUBARU_67         13
#define DECODER_DAIHATSU_PLUS1    14
#define DECODER_HARLEY            15
#define DECODER_36_2_2_2          16
#define DECODER_36_2_1            17
#define DECODER_420A              18
#define DECODER_WEBER             19
#define DECODER_ST170             20
#define DECODER_DRZ400            21
#define DECODER_NGC               22
#define DECODER_VMAX              23
#define DECODER_RENIX             24
#define DECODER_ROVERMEMS		      25
#define DECODER_SUZUKI_K6A        26
#define DECODER_HONDA_J32         27

#define BIT_DECODER_2ND_DERIV           0 //The use of the 2nd derivative calculation is limited to certain decoders. This is set to either true or false in each decoders setup routine
#define BIT_DECODER_IS_SEQUENTIAL       1 //Whether or not the decoder supports sequential operation
#define BIT_DECODER_UNUSED1             2 
#define BIT_DECODER_HAS_SECONDARY       3 //Whether or not the decoder supports fixed cranking timing
#define BIT_DECODER_HAS_FIXED_CRANKING  4
#define BIT_DECODER_VALID_TRIGGER       5 //Is set true when the last trigger (Primary or secondary) was valid (ie passed filters)
#define BIT_DECODER_TOOTH_ANG_CORRECT   6 //Whether or not the triggerToothAngle variable is currently accurate. Some patterns have times when the triggerToothAngle variable cannot be accurately set.

extern volatile uint8_t decoderState;

//This isn't to to filter out wrong pulses on triggers, but just to smooth out the cam angle reading for better closed loop VVT control.
#define ANGLE_FILTER(input, alpha, prior) (((long)(input) * (256 - (alpha)) + ((long)(prior) * (alpha)))) >> 8

//All of the below are the 6 required functions for each decoder / pattern
decoder_t triggerSetup_missingTooth(void);

decoder_t triggerSetup_DualWheel(void);

decoder_t triggerSetup_BasicDistributor(void);

decoder_t triggerSetup_GM7X(void);

decoder_t triggerSetup_4G63(void);

decoder_t triggerSetup_24X(void);

decoder_t triggerSetup_Jeep2000(void);

decoder_t triggerSetup_Audi135(void);

decoder_t triggerSetup_HondaD17(void);

decoder_t triggerSetup_HondaJ32(void);

decoder_t triggerSetup_Miata9905(void);

decoder_t triggerSetup_MazdaAU(void);

decoder_t triggerSetup_non360(void);

decoder_t triggerSetup_Nissan360(void);

decoder_t triggerSetup_Subaru67(void);

decoder_t triggerSetup_Daihatsu(void);

decoder_t triggerSetup_Harley(void);

decoder_t triggerSetup_ThirtySixMinus222(void);

decoder_t triggerSetup_ThirtySixMinus21(void);

decoder_t triggerSetup_420a(void);

decoder_t triggerSetup_Webber(void);

decoder_t triggerSetup_FordST170(void);

decoder_t triggerSetup_DRZ400(void);

decoder_t triggerSetup_NGC(void);

decoder_t triggerSetup_Renix(void);

decoder_t triggerSetup_RoverMEMS(void);

decoder_t triggerSetup_Vmax(void);

decoder_t triggerSetup_SuzukiK6A(void);

int getCamAngle_Miata9905(void);

bool isFixedCrankLock(void);

bool hasEngineStopped(uint32_t curTime);

void engineStoppedDecoder(void);

bool isRevolutionOne(void);

uint32_t getToothLogValue(void);

extern volatile unsigned long toothLastToothTime; //The time (micros()) that the last tooth was registered

#define CRANK_SPEED 0U
#define CAM_SPEED   1U

#endif
