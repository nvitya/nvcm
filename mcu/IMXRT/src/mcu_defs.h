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
 *  file:     mcu_defs.h (IMXRT)
 *  brief:    IMXRT MCU Family definitions
 *  version:  1.00
 *  date:     2018-11-23
 *  authors:  nvitya
*/

#ifndef __MCU_DEFS_H
#define __MCU_DEFS_H

#if defined(MCUSF_1020)

  #define MAX_CLOCK_SPEED  500000000

#endif

#define HW_GPIO_REGS      GPIO_Type
#define HW_UART_REGS      LPUART_Type
#define HW_SPI_REGS       LPSPI_Type
#define HW_QSPI_REGS		  FLEXSPI_Type

inline void __attribute__((always_inline)) mcu_preinit_code()
{
  // Disable Watchdog
  if (WDOG1->WCR & WDOG_WCR_WDE_MASK)
  {
      WDOG1->WCR &= ~WDOG_WCR_WDE_MASK;
  }
  if (WDOG2->WCR & WDOG_WCR_WDE_MASK)
  {
      WDOG2->WCR &= ~WDOG_WCR_WDE_MASK;
  }
  RTWDOG->CNT = 0xD928C520U; // 0xD928C520U is the update key
  RTWDOG->TOVAL = 0xFFFF;
  RTWDOG->CS = (uint32_t) ((RTWDOG->CS) & ~RTWDOG_CS_EN_MASK) | RTWDOG_CS_UPDATE_MASK;
}

#endif // __MCU_DEFS_H
