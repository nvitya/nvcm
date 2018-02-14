#ifndef __MCU_BUILTIN_H
#define __MCU_BUILTIN_H

#if 0

//----------------------------------------------------------------------
// Freescale / Kinetis
//----------------------------------------------------------------------

#elif defined(MCU_MK20DN)

  #define MCUF_KINETIS
  #define MCUSF_K20

  #include "MK20D10.h"

#elif defined(MCU_MKL03)

  #define MCUF_KINETIS
  #define MCUSF_KL03

  #include "MKL03Z4.h"

#elif defined(MCU_MKV30F)

  #define MCUF_KINETIS
  #define MCUSF_KV30

  #include "MKV30F12810.h"

#else

  #error "Unknown MCU"

#endif

#endif
