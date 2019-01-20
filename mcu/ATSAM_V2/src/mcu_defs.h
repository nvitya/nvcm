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
 *  file:     mcu_defs.h (ATSAM_V2)
 *  brief:    ATSAM_V2 MCU Family definitions
 *  version:  1.00
 *  date:     2018-02-10
 *  authors:  nvitya
*/

#ifndef __MCU_DEFS_H
#define __MCU_DEFS_H

// SUB-FAMILIES
#if defined(MCUSF_DXX) || defined(MCUSF_C2X)

  #define MAX_CLOCK_SPEED   48000000

#elif defined(MCUSF_E5X)

  #define MAX_CLOCK_SPEED  120000000

#endif


#define HW_GPIO_REGS      PortGroup
#define HW_UART_REGS      SercomUsart

#if __CORTEX_M < 3
  #define CLOCKCNT_BITS  32

	#if defined(MCUSF_C2X)
	  uint32_t atsam_v2_clockcnt_read();
	  #define CLOCKCNT       (atsam_v2_clockcnt_read())
	#else
		#define CLOCKCNT       (TC1->COUNT32.COUNT.reg)
	#endif
#endif



inline void __attribute__((always_inline)) mcu_preinit_code()
{
  #ifdef WDT_MR_WDDIS
    WDT->WDT_MR |= WDT_MR_WDDIS;  // Turn off the WDT, can not be turned back on until reset
  #endif
}

inline void __attribute__((always_inline)) mcu_postinit_code()
{
}

#endif // __MCU_DEFS_H
