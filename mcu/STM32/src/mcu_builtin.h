#ifndef __MCU_BUILTIN_H
#define __MCU_BUILTIN_H

#if 0

//----------------------------------------------------------------------
// STM32
//----------------------------------------------------------------------

// STM32F0: Cortex-M0

#elif defined(MCU_STM32F030F4)

  #define MCUF_STM32
  #define MCUSF_F0

  #include "stm32f030x6.h"

#elif defined(MCU_STM32F072RB)

  #define MCUF_STM32
  #define MCUSF_F0

  #include "stm32f072xb.h"

// STM32F1: Cortex-M3

#elif defined(MCU_STM32F103C8)

  #define MCUF_STM32
  #define MCUSF_F1
  #include "stm32f103xb.h"

// STM32F3: Cortex-M4F

#elif defined(MCU_STM32F301C8) || defined(MCU_STM32F301K6)

  #define MCUF_STM32
  #define MCUSF_F3

  #include "stm32f301x8.h"

// STM32F4: Cortex-M4F

#elif defined(MCU_STM32F446ZE)

	#define MCUF_STM32
  #define MCUSF_F4
  #define MAX_CLOCK_SPEED  180000000

  #include "stm32f446xx.h"

#elif defined(MCU_STM32F429ZI)

	#define MCUF_STM32
  #define MCUSF_F4
  #define MAX_CLOCK_SPEED  180000000

  #include "stm32f429xx.h"

#elif defined(MCU_STM32F407VE) || defined(MCU_STM32F407VG) || defined(MCU_STM32F407ZE) || defined(MCU_STM32F407ZG)

	#define MCUF_STM32
  #define MCUSF_F4
  #define MAX_CLOCK_SPEED  168000000

  #include "stm32f407xx.h"

// STM32F7: Cortex-M7

#elif defined(MCU_STM32F746ZG)

	#define MCUF_STM32
  #define MCUSF_F7

  #include "stm32f746xx.h"

#else

  #error "Unknown MCU"

#endif

#endif
