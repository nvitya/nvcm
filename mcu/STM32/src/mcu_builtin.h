/* -----------------------------------------------------------------------------
 * This file is a part of the NVCM project: https://github.com/nvitya/nvcm
 * Copyright (c) 2018 Viktor Nagy, nvitya
 *
 * This software is provided 'as-is', without any express or implied warranty.
 * In no event will the authors be held liable for any damages arising from
 * the use of this software. Permission is granted to anyone to use this
 * software for any purpose, including commercial applications, and to alter
 * it and redistribute it freely, subject to the following restrictions:
 *
 * 1. The origin of this software must not be misrepresented; you must not
 *    claim that you wrote the original software. If you use this software in
 *    a product, an acknowledgment in the product documentation would be
 *    appreciated but is not required.
 *
 * 2. Altered source versions must be plainly marked as such, and must not be
 *    misrepresented as being the original software.
 *
 * 3. This notice may not be removed or altered from any source distribution.
 * --------------------------------------------------------------------------- */
/*
 *  file:     mcu_builtin.h (STM32)
 *  brief:    Built-in STM32 MCU definitions
 *  version:  1.00
 *  date:     2018-02-10
 *  authors:  nvitya
*/

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

#elif defined(MCU_STM32F070F6)

  #define MCUF_STM32
  #define MCUSF_F0

  #include "stm32f070x6.h"

#elif defined(MCU_STM32F070RB)

  #define MCUF_STM32
  #define MCUSF_F0

  #include "stm32f070xb.h"

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

#elif defined(MCU_STM32F303CB) || defined(MCU_STM32F303CC)

  #define MCUF_STM32
  #define MCUSF_F3

  #include "stm32f303xc.h"

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

#elif defined(MCU_STM32F405RG) || defined(MCU_STM32F405VE) || defined(MCU_STM32F405VG) || defined(MCU_STM32F405ZE) || defined(MCU_STM32F405ZG)

	#define MCUF_STM32
  #define MCUSF_F4
  #define MAX_CLOCK_SPEED  168000000

  #include "stm32f405xx.h"

#elif defined(MCU_STM32F407VE) || defined(MCU_STM32F407VE) || defined(MCU_STM32F407VG) || defined(MCU_STM32F407ZE) || defined(MCU_STM32F407ZG)

  #define MCUF_STM32
  #define MCUSF_F4
  #define MAX_CLOCK_SPEED  168000000

  #include "stm32f407xx.h"

#elif defined(MCU_STM32G474RE) || defined(MCU_STM32G473RE) || defined(MCU_STM32G473CB) || defined(MCU_STM32G473CE)

  #define MCUF_STM32
  #define MCUSF_G4

  #include "stm32g474xx.h"

#elif defined(MCU_STM32G431CB) || defined(MCU_STM32G431KB)

  #define MCUF_STM32
  #define MCUSF_G4

  #include "stm32g431xx.h"

// STM32F7: Cortex-M7

#elif defined(MCU_STM32F746ZG) || defined(MCU_STM32F746NG)

  #define MCUF_STM32
  #define MCUSF_F7

  #include "stm32f746xx.h"

#else

  #error "Unknown MCU"

#endif

#endif
