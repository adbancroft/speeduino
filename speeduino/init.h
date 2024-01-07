#ifndef INIT_H
#define INIT_H
#include "pin_mapping.h"
#include "auxiliaries.h"
#include "sensors.h"

void initialiseAll(void);

#define VSS_USES_RPM2() (isVssModeInterrupt() && (pinMapping.inputs.pinVSS == pinMapping.inputs.pinTrigger2) && !BIT_CHECK(decoderState, BIT_DECODER_HAS_SECONDARY)) // VSS is on the same pin as RPM2 and RPM2 is not used as part of the decoder
#define FLEX_USES_RPM2() (isFlexEnabled() && (pinMapping.inputs.pinFlex == pinMapping.inputs.pinTrigger2) && !BIT_CHECK(decoderState, BIT_DECODER_HAS_SECONDARY)) // Same as above, but for Flex sensor

#endif