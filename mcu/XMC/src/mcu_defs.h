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
 *  file:     mcu_defs.h (XMC)
 *  brief:    XMC MCU Family definitions
 *  version:  1.00
 *  date:     2018-02-10
 *  authors:  nvitya
*/

#ifndef __MCU_DEFS_H
#define __MCU_DEFS_H


#if	defined(MCUSF_1000)

  #define MCU_INTRC_SPEED     4000000

  #if !defined(MAX_CLOCK_SPEED)
    #define MAX_CLOCK_SPEED  32000000
  #endif

#endif


// the official XMC definition in the header is not so usable (different definitions for every port)

typedef struct XMC_GPIO_PORT
{
  __IO uint32_t  OUT;				// The port output register determines the value of a GPIO pin when it is selected by Pn_IOCRx as output
  __O  uint32_t  OMR;				// The port output modification register contains control bits that make it
                      // possible to individually set, reset, or toggle the logic state of a single port line
  __I  uint32_t  RESERVED0[2];
  __IO uint32_t  IOCR[4];			// The port input/output control registers select the digital output and input
                                           // driver functionality and characteristics of a GPIO port pin
  __I  uint32_t  RESERVED1;
  __I  uint32_t  IN;				   // The logic level of a GPIO pin can be read via the read-only port input register Pn_IN
  __I  uint32_t  RESERVED2[6];
  __IO uint32_t  PHCR[2];			// Pad hysteresis control register
  __I  uint32_t  RESERVED3[6];
  __IO uint32_t  PDISC;				// Pin Function Decision Control Register is to disable/enable the digital pad
                                           // structure in shared analog and digital ports
  __I  uint32_t  RESERVED4[3];
  __IO uint32_t  PPS;				// Pin Power Save Register
  __IO uint32_t  HWSEL;				// Pin Hardware Select Register
//
} XMC_GPIO_PORT_t;

#define HW_GPIO_REGS  XMC_GPIO_PORT_t
#define HW_UART_REGS  USIC_CH_TypeDef

#if __CORTEX_M < 3
  #define CLOCKCNT       (CCU40_CC43->TIMER)
  #define CLOCKCNT_BITS  16
#endif

inline void __attribute__((always_inline)) mcu_preinit_code()
{
}

#endif // __MCU_DEFS_H
