#ifndef __MCU_BUILTIN_H
#define __MCU_BUILTIN_H

#if 0

//----------------------------------------------------------------------
// Atmel
//----------------------------------------------------------------------

#elif defined(MCU_ATSAM3X8E)

  #define MCUF_ATSAM
  #define MCUSF_3X

  #define __SAM3X8E__
  #include "sam3xa.h"

#elif defined(MCU_ATSAME70N20)

  #define MCUF_ATSAM
  #define MCUSF_E70

  #define __SAME70N20__
  #include "same70.h"

#elif defined(MCU_ATSAME70Q21)

  #define MCUF_ATSAM
  #define MCUSF_E70

  #define __SAME70Q21__
  #include "same70.h"

#elif defined(MCU_ATSAM4S2B) || defined(MCU_ATSAM4S8B)

  #define MCUF_ATSAM
  #define MCUSF_4S

  #define __SAM4S8B__
  #include "sam4s.h"

#else

  #error "Unknown MCU"

#endif

#endif
