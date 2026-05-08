#include "board_definition.h"
#include "src/pins/fastOutputPin.h"
#include "preprocessor.h"

// LCOV_EXCL_START
// Exclude from code coverage, since this is all board output control

static fastOutputPin_t pins[INJ_CHANNELS];

void initInjDirectIO(const uint8_t (&pinNumbers)[INJ_CHANNELS])
{
    for (uint8_t i = 0; i < _countof(pins); i++)
    {
        pins[i].setPin(pinNumbers[i], OUTPUT);
    }
}

void openInjector_DIRECT(uint8_t channel)
{
    if (channel<=_countof(pins))
    {
        pins[channel-1U].setPinHigh();
    }
}
void closeInjector_DIRECT(uint8_t channel)
{
    if (channel<=_countof(pins))
    {
        pins[channel-1U].setPinLow();
    }
}

// LCOV_EXCL_STOP