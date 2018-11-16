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
 *  file:     boards_builtin.h (STM32)
 *  brief:    Built-in STM32 board definitions
 *  version:  1.00
 *  date:     2018-02-10
 *  authors:  nvitya
*/

#ifndef BOARDS_BUILTIN_H_
#define BOARDS_BUILTIN_H_

#if 0 // to use elif everywhere

//-------------------------------------------------------------------------------------------------
// STM32
//-------------------------------------------------------------------------------------------------

#elif defined(BOARD_MIN_F103)

  #define BOARD_NAME "STM32F103C8 Minimum Develompent Board"
  #define MCU_STM32F103C8
  #define MCU_INPUT_FREQ   8000000

#elif defined(BOARD_MIBO48_STM32F303)

  #define BOARD_NAME "STM32F303Cx 48-pin Develompent Board by nvitya"
  #define MCU_STM32F303CB
  #define MCU_INPUT_FREQ   8000000

#elif defined(BOARD_MIBO20_STM32F070)

  #define BOARD_NAME "STM32F070F6 20-pin Develompent Board by WF"
  #define MCU_STM32F070F6
  #define MCU_INPUT_FREQ   8000000

#elif defined(BOARD_MIBO20_STM32F030)

  #define BOARD_NAME "STM32F030F4 20-pin Develompent Board by WF"
  #define MCU_STM32F030F4

#elif defined(BOARD_MIBO64_STM32F405)

  #define BOARD_NAME "STM32F405RG 64-pin Develompent Board by nvitya"
  #define MCU_STM32F405RG
  #define MCU_INPUT_FREQ   8000000

#elif defined(BOARD_DISCOVERY_F072)

  #define BOARD_NAME "STM32F072 Discovery"
  #define MCU_STM32F072RB

#elif defined(BOARD_DEV_STM32F407VG)

  #define BOARD_NAME "STM32F407VG Minimal Board"
  #define MCU_STM32F407VG
  #define MCU_INPUT_FREQ   8000000

#elif defined(BOARD_DEV_STM32F407ZE)

  #define BOARD_NAME "STM32F407ZE Development Board"
  #define MCU_STM32F407ZE
  #define MCU_INPUT_FREQ   8000000

#elif defined(BOARD_NUCLEO_F446)

  #define BOARD_NAME "STM32F446 Nucleo-144"
  #define MCU_STM32F446ZE
  #define MCU_INPUT_FREQ   8000000

#elif defined(BOARD_NUCLEO_F746)

  #define BOARD_NAME "STM32F746 Nucleo-144"
  #define MCU_STM32F746ZG
  #define MCU_INPUT_FREQ   8000000

#elif defined(BOARD_DISCOVERY_F746)

  #define BOARD_NAME "STM32F746 Discovery"
  #define MCU_STM32F746NG
  #define MCU_INPUT_FREQ   25000000

  #define MAX_CLOCK_SPEED  200000000


#else

  #error "Unknown board."

#endif


#endif /* BOARDS_BUILTIN_H_ */
