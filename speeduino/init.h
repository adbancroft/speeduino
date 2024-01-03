#ifndef INIT_H
#define INIT_H
#include "pin_mapping.h"

void initialiseAll(void);

#define VSS_USES_RPM2() ((configPage2.vssMode > 1U) && (pinMapping.inputs.pinVSS == pinMapping.inputs.pinTrigger2) && !BIT_CHECK(decoderState, BIT_DECODER_HAS_SECONDARY)) // VSS is on the same pin as RPM2 and RPM2 is not used as part of the decoder
#define FLEX_USES_RPM2() ((configPage2.flexEnabled > 0U) && (pinMapping.inputs.pinFlex == pinMapping.inputs.pinTrigger2) && !BIT_CHECK(decoderState, BIT_DECODER_HAS_SECONDARY)) // Same as above, but for Flex sensor

#endif