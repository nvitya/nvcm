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
 *  file:     mcu_defs.h (KIENTIS)
 *  brief:    KINETIS MCU Family definitions
 *  version:  1.00
 *  date:     2018-02-10
 *  authors:  nvitya
*/

#ifndef __MCU_DEFS_H
#define __MCU_DEFS_H

#if defined(MCUSF_K20)

  #define MAX_CLOCK_SPEED  48000000

#elif defined(MCUSF_KV30)

  #define MAX_CLOCK_SPEED  100000000

#elif defined(MCUSF_KV31)

  #define MAX_CLOCK_SPEED  120000000

#elif defined(MCUSF_KL03)

  #define MAX_CLOCK_SPEED  48000000

#endif

#define HW_GPIO_REGS  GPIO_Type

#ifdef LPUART_Type
  #define HW_UART_REGS  LPUART_Type
#else
  #define HW_UART_REGS  UART_Type
#endif

#if __CORTEX_M < 3
  #define CLOCKCNT16       (TPM1->CNT)
#endif

inline void __attribute__((always_inline)) mcu_preinit_code()
{
  // The Kinetis processors start with Watchdog enabled
  WDOG->UNLOCK = 0xC520;
  WDOG->UNLOCK = 0xD928;
  WDOG->STCTRLH = 0xD2;
}


#endif // __MCU_DEFS_H
