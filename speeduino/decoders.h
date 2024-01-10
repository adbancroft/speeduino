#ifndef DECODERS_H
#define DECODERS_H

#include <stdint.h>
#include "board_selector.h"
// #include "globals.h"

using trigger_handler_t = void (*)(void);
#if defined(CORE_SAMD21)
using trigger_edge_t = PinStatus;
#else
using trigger_edge_t = uint8_t;
#endif

struct trigger_t {
  trigger_handler_t handler;
  trigger_edge_t edge; // CHANGE, RISING, FALLING
};

static constexpr trigger_t NULL_TRIGGER = { nullptr, 0U };

static inline bool isValid(const trigger_t &trigger) {
  return trigger.handler!=nullptr;
}

struct decoder_t {
    uint16_t (*getRPM)(void); //Pointer to the getRPM function (Gets pointed to the relevant decoder)
    int (*getCrankAngle)(void); //Pointer to the getCrank Angle function (Gets pointed to the relevant decoder)
    void (*triggerSetEndTeeth)(void); //Pointer to the triggerSetEndTeeth function of each decoder

    trigger_t primaryTrigger;
    trigger_t secondaryTrigger;
    trigger_t tertiaryTrigger;
};


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

#define BIT_DECODER_2ND_DERIV           0 //The use of the 2nd derivative calculation is limited to certain decoders. This is set to either true or false in each decoders setup routine
#define BIT_DECODER_IS_SEQUENTIAL       1 //Whether or not the decoder supports sequential operation
#define BIT_DECODER_UNUSED1             2 
#define BIT_DECODER_HAS_SECONDARY       3 //Whether or not the decoder supports fixed cranking timing
#define BIT_DECODER_HAS_FIXED_CRANKING  4
#define BIT_DECODER_VALID_TRIGGER       5 //Is set true when the last trigger (Primary or secondary) was valid (ie passed filters)
#define BIT_DECODER_TOOTH_ANG_CORRECT   6 //Whether or not the triggerToothAngle variable is currently accurate. Some patterns have times when the triggerToothAngle variable cannot be accurately set.

//220 bytes free
extern volatile uint8_t decoderState;

/*
extern volatile bool validTrigger; //Is set true when the last trigger (Primary or secondary) was valid (ie passed filters)
extern volatile bool triggerToothAngleIsCorrect; //Whether or not the triggerToothAngle variable is currently accurate. Some patterns have times when the triggerToothAngle variable cannot be accurately set.
extern bool secondDerivEnabled; //The use of the 2nd derivative calculation is limited to certain decoders. This is set to either true or false in each decoders setup routine
extern bool decoderIsSequential; //Whether or not the decoder supports sequential operation
extern bool decoderHasSecondary; //Whether or not the pattern uses a secondary input
extern bool decoderHasFixedCrankingTiming; 
*/

//This isn't to to filter out wrong pulses on triggers, but just to smooth out the cam angle reading for better closed loop VVT control.
#define ANGLE_FILTER(input, alpha, prior) (((long)(input) * (256 - (alpha)) + ((long)(prior) * (alpha)))) >> 8

bool isFixedCrankLock(void);

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

decoder_t triggerSetup_Miata9905(void);
int getCamAngle_Miata9905(void);

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

extern volatile unsigned long curTime;
extern volatile unsigned long curGap;
extern volatile unsigned long curTime2;
extern volatile unsigned long curGap2;
extern volatile unsigned long curGap3;
extern volatile unsigned long lastGap;
extern volatile unsigned long targetGap;

extern unsigned long MAX_STALL_TIME; //The maximum time (in uS) that the system will continue to function before the engine is considered stalled/stopped. This is unique to each decoder, depending on the number of teeth etc. 500000 (half a second) is used as the default value, most decoders will be much less.
extern volatile uint16_t toothCurrentCount; //The current number of teeth (Once sync has been achieved, this can never actually be 0
extern volatile byte toothSystemCount; //Used for decoders such as Audi 135 where not every tooth is used for calculating crank angle. This variable stores the actual number of teeth, not the number being used to calculate crank angle
extern volatile unsigned long toothSystemLastToothTime; //As below, but used for decoders where not every tooth count is used for calculation
extern volatile unsigned long toothLastToothTime; //The time (micros()) that the last tooth was registered
extern volatile unsigned long toothLastSecToothTime; //The time (micros()) that the last tooth was registered on the secondary input
extern volatile unsigned long toothLastThirdToothTime; //The time (micros()) that the last tooth was registered on the second cam input
extern volatile unsigned long toothLastMinusOneToothTime; //The time (micros()) that the tooth before the last tooth was registered
extern volatile unsigned long toothLastMinusOneSecToothTime; //The time (micros()) that the tooth before the last tooth was registered on secondary input
extern volatile unsigned long targetGap2;

extern volatile unsigned long toothOneTime; //The time (micros()) that tooth 1 last triggered
extern volatile unsigned long toothOneMinusOneTime; //The 2nd to last time (micros()) that tooth 1 last triggered
extern volatile bool revolutionOne; // For sequential operation, this tracks whether the current revolution is 1 or 2 (not 1)

extern volatile unsigned int secondaryToothCount; //Used for identifying the current secondary (Usually cam) tooth for patterns with multiple secondary teeth
extern volatile unsigned long secondaryLastToothTime; //The time (micros()) that the last tooth was registered (Cam input)
extern volatile unsigned long secondaryLastToothTime1; //The time (micros()) that the last tooth was registered (Cam input)

extern uint16_t triggerActualTeeth;
extern volatile unsigned long triggerFilterTime; // The shortest time (in uS) that pulses will be accepted (Used for debounce filtering)
extern volatile unsigned long triggerSecFilterTime; // The shortest time (in uS) that pulses will be accepted (Used for debounce filtering) for the secondary input
extern unsigned int triggerSecFilterTime_duration; // The shortest valid time (in uS) pulse DURATION
extern volatile uint16_t triggerToothAngle; //The number of crank degrees that elapse per tooth
extern byte checkSyncToothCount; //How many teeth must've been seen on this revolution before we try to confirm sync (Useful for missing tooth type decoders)
extern unsigned long lastCrankAngleCalc;
extern unsigned long lastVVTtime; //The time between the vvt reference pulse and the last crank pulse

typedef uint32_t UQ24X8_t;
constexpr uint8_t UQ24X8_Shift = 8U;

/** @brief uS per degree at current RPM in UQ24.8 fixed point */
extern UQ24X8_t microsPerDegree;
constexpr uint8_t microsPerDegree_Shift = UQ24X8_Shift;

typedef uint16_t UQ1X15_t;
constexpr uint8_t UQ1X15_Shift = 15U;

/** @brief Degrees per uS in UQ1.15 fixed point.
 * 
 * Ranges from 8 (0.000246) at MIN_RPM to 3542 (0.108) at MAX_RPM
 */
extern UQ1X15_t degreesPerMicro;
constexpr uint8_t degreesPerMicro_Shift = UQ1X15_Shift;

extern int16_t toothAngles[24]; //An array for storing fixed tooth angles. Currently sized at 24 for the GM 24X decoder, but may grow later if there are other decoders that use this style

#define CRANK_SPEED 0U
#define CAM_SPEED   1U

// used by the ROVER MEMS pattern
#define ID_TOOTH_PATTERN 0 // have we identified teeth to skip for calculating RPM?
#define SKIP_TOOTH1 1
#define SKIP_TOOTH2 2
#define SKIP_TOOTH3 3
#define SKIP_TOOTH4 4

#endif
