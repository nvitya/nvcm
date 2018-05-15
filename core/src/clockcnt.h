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
 *  brief:    Clock Counter vendor-independent definitions
 *  version:  1.00
 *  date:     2018-02-10
 *  authors:  nvitya
*/

#ifndef CLOCKCNT_H_
#define CLOCKCNT_H_

#include "platform.h"

#define clockcnt_t unsigned

#ifndef CLOCKCNT

#if __CORTEX_M >= 3

  // from Cortex-M3 use the DWT_CYCCNT:
  #define CLOCKCNT (*((volatile unsigned *)0xE0001004))
  #define CLOCKCNT_BITS  32

#elif defined(CLOCKCNT16)

	// the 32 bit clock counter works properly only when it is called repeatedly within 0.683 ms !!!!

	extern uint32_t clockcnt32_high;
	extern uint16_t last_clockcnt16;

	inline unsigned clockcnt16_to_32()
	{
		uint16_t t0 = CLOCKCNT16;
		if (t0 < last_clockcnt16)
		{
			clockcnt32_high += 0x10000;
		}
		last_clockcnt16 = t0;
		return clockcnt32_high + t0;
	}

  #define CLOCKCNT clockcnt16_to_32()

#else
  // On Cortex-M0 a 16 or 32 bit timer hw required
  #error "Define CLOCKCNT_BITS for Cortex-M0 processors (hw timer required)"
#endif

#endif

#define ELAPSEDCLOCKS(t1, t0) ((clockcnt_t)(t1 - t0))

void clockcnt_init();  // for Cortex-M0 it is platform dependent

void delay_clocks(unsigned aclocks);

inline void delay_us(unsigned aus)
{
	delay_clocks(aus * (SystemCoreClock / 1000000));
}

inline void delay_ms(unsigned ams)
{
	delay_clocks(ams * (SystemCoreClock / 1000));
}

#endif /* CLOCKCNT_H_ */
