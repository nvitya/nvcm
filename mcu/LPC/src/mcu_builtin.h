#ifndef __MCU_BUILTIN_H
#define __MCU_BUILTIN_H

//----------------------------------------------------------------------
// NXP
//----------------------------------------------------------------------

#if defined(MCU_LPC4330) || defined(MCU_LPC4333) || defined(MCU_LPC4337) || \
	  defined(MCU_LPC4350) || defined(MCU_LPC4353) || defined(MCU_LPC4357)

  #define MCUF_LPC
  #define MCUSF_43XX

  #define CHIP_LPC43XX
  #define CORE_M4

  #include "chip_lpc43xx.h"

#else

  #error "Unknown MCU"

#endif

#endif
