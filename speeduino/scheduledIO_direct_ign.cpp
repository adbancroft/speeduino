#include "globals.h"
#include "board_definition.h"
#include "src/pins/fastOutputPin.h"
#include "preprocessor.h"

// LCOV_EXCL_START
// Exclude from code coverage, since this is all board output control
 
static fastOutputPin_t pins[IGN_CHANNELS];

void initIgnDirectIO(const uint8_t (&pinNumbers)[IGN_CHANNELS])
{
    for (uint8_t i = 0; i < _countof(pins); i++)
    {
        pins[i].setPin(pinNumbers[i], OUTPUT);
    }
}

static inline void coilLow(uint8_t channel)
{
    if (channel<=_countof(pins))
    {
        pins[channel-1U].setPinLow();
    }
}

static inline void coilHigh(uint8_t channel)
{
    if (channel<=_countof(pins))
    {
        pins[channel-1U].setPinHigh();
    }
}

void coilCharging_DIRECT(uint8_t channel) 
{ 
    if (configPage4.IgInv == GOING_HIGH)
    {
        coilLow(channel);
    }
    else
    {
        coilHigh(channel);
    }
}

void coilStopCharging_DIRECT(uint8_t channel) 
{
    if (configPage4.IgInv == GOING_HIGH)
    {
        coilHigh(channel);
     }
     else 
     {
        coilLow(channel);
     }
}

// LCOV_EXCL_STOP