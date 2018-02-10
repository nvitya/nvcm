#ifndef __MCU_BUILTIN_H
#define __MCU_BUILTIN_H

//----------------------------------------------------------------------
// IMXRT
//----------------------------------------------------------------------

#if defined(MCU_IMXRT1050) || defined(MCU_IMXRT1051) || defined(MCU_IMXRT1052)

  #define MCUF_IMXRT
  #define MCUSF_1050

  #include "MIMXRT1052.h"

#else

  #error "Unknown MCU"

#endif

#endif
