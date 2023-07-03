#pragma once

#include "board_definition.h"

#if defined(OUTPUT_CONTROL_SUPPORTED)
#include "acc_mc33810.h"
#include "pre_processor.h"

#define MC33810_CALL_FUNC_SINGLE(prefix, index, postfix) CONCAT(prefix, CONCAT(index, postfix)) ();
// There is probably a better way!
#define MC33810_CALL_FUNC_BYINDEX(prefix, index, postfix) \
    switch (index) \
    { \
        default: \
        case 1: MC33810_CALL_FUNC_SINGLE(prefix, 1, postfix);  break; \
        case 2: MC33810_CALL_FUNC_SINGLE(prefix, 2, postfix);  break; \
        case 3: MC33810_CALL_FUNC_SINGLE(prefix, 3, postfix);  break; \
        case 4: MC33810_CALL_FUNC_SINGLE(prefix, 4, postfix);  break; \
        case 5: MC33810_CALL_FUNC_SINGLE(prefix, 5, postfix);  break; \
        case 6: MC33810_CALL_FUNC_SINGLE(prefix, 6, postfix);  break; \
        case 7: MC33810_CALL_FUNC_SINGLE(prefix, 7, postfix);  break; \
        case 8: MC33810_CALL_FUNC_SINGLE(prefix, 8, postfix);  break; \
    }

#define coilStartCharging_MC33810(coilIndex)  \
    { if(configPage4.IgInv == GOING_HIGH) { MC33810_CALL_FUNC_BYINDEX(coil, coilIndex, Low_MC33810);  } else { MC33810_CALL_FUNC_BYINDEX(coil, coilIndex, High_MC33810); } }
#define coilStopCharging_MC33810(coilIndex)  \
    { if(configPage4.IgInv == GOING_HIGH) { MC33810_CALL_FUNC_BYINDEX(coil, coilIndex, High_MC33810); } else { MC33810_CALL_FUNC_BYINDEX(coil, coilIndex, Low_MC33810); } }

#define openInjector_MC33810(injectorIndex)  MC33810_CALL_FUNC_BYINDEX(openInjector, injectorIndex, _MC33810)
#define closeInjector_MC33810(injectorIndex)  MC33810_CALL_FUNC_BYINDEX(closeInjector, injectorIndex, _MC33810)
#define toggleInjector_MC33810(injectorIndex)  MC33810_CALL_FUNC_BYINDEX(injector, injectorIndex, Toggle_MC33810)

#endif
