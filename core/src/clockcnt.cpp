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
 *  file:     clockcnt.h
 *  brief:    Clock Counter vendor-independent implementations
 *  version:  1.00
 *  date:     2018-02-10
 *  authors:  nvitya
 *
 *  notes:
 *    for Cortex-M0 systems MCU specific clockcnt_init() required (using a HW timer)
*/

#include "platform.h"
#include "clockcnt.h"

#if __CORTEX_M >= 3

// from Cortex-M3 use the DWT_CYCCNT

void clockcnt_init()
{
	CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;
#if __CORTEX_M >= 7
	DWT->LAR = 0xC5ACCE55;	// this is requied for Cortex M7
#endif
	DWT->CTRL = DWT->CTRL | 1;
}

#endif

#if defined(CLOCKCNT16)

  uint32_t clockcnt32_high = 0;
  uint16_t last_clockcnt16 = 0;

#endif

void delay_clocks(unsigned aclocks)
{
	unsigned remaining = aclocks;

	unsigned t0 = CLOCKCNT;
	while (CLOCKCNT - t0 < remaining)
	{
		// wait
	}
}

