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
 *  file:     mcu_defs.h (STM32)
 *  brief:    STM32 MCU Family definitions
 *  version:  1.00
 *  date:     2018-02-10
 *  authors:  nvitya
*/

#ifndef __MCU_DEFS_H
#define __MCU_DEFS_H

#if defined(MCUSF_F0)

  #define MAX_CLOCK_SPEED  48000000
  #define MCU_INTRC_SPEED   8000000

#elif	defined(MCUSF_L0)

  #define MAX_CLOCK_SPEED  32000000

#elif	defined(MCUSF_F1)

  #if !defined(MAX_CLOCK_SPEED)
    #define MAX_CLOCK_SPEED  72000000
  #endif

#elif	defined(MCUSF_F3)

  #if !defined(MAX_CLOCK_SPEED)
    #define MAX_CLOCK_SPEED  72000000
  #endif

#elif	defined(MCUSF_F4)

  #if !defined(MAX_CLOCK_SPEED)
    #define MAX_CLOCK_SPEED  180000000
  #endif

#elif	defined(MCUSF_F7)

  #if !defined(MAX_CLOCK_SPEED)
    #define MAX_CLOCK_SPEED  216000000
  #endif

#endif

#define HW_GPIO_REGS  GPIO_TypeDef
#define HW_UART_REGS  USART_TypeDef
#define HW_SPI_REGS   SPI_TypeDef
#ifdef QUADSPI
  #define HW_QSPI_REGS  QUADSPI_TypeDef
#endif

#if defined(MCUSF_F1) || defined(MCUSF_F0) || defined(MCUSF_L0) || defined(MCUSF_F3)
  #define HW_DMA_REGS 	DMA_Channel_TypeDef
#else
  #define HW_DMA_REGS   DMA_Stream_TypeDef
#endif

#if __CORTEX_M < 3
  #if defined(TIM14)
    #define CLOCKCNT16       (TIM14->CNT)      // use the worst timer for clock counting
  #else
    #define CLOCKCNT16       (TIM21->CNT)
  #endif
#endif

inline void __attribute__((always_inline)) mcu_preinit_code()
{
}

#endif // __MCU_DEFS_H
