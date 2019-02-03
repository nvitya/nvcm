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
 *  file:     atsam_v2_utils.cpp
 *  brief:    ATSAM V2 Utilities
 *  version:  1.00
 *  date:     2019-01-18
 *  authors:  nvitya
*/

#include "atsam_v2_utils.h"

void atsam2_enable_mclk(bool isahb, uint8_t regid, uint8_t bitid)
{
	volatile uint32_t * pu32;

#if defined(MCUSF_DXX)
	if (isahb)
	{
		pu32 = (volatile uint32_t *)&(PM->AHBMASK);
	}
	else
	{
		pu32 = (volatile uint32_t *)&(PM->APBAMASK);
		pu32 += regid;
	}
#else	// E5X, C2X
	if (isahb)
	{
		pu32 = (volatile uint32_t *)&(MCLK->AHBMASK);
	}
	else
	{
		pu32 = (volatile uint32_t *)&(MCLK->APBAMASK);
		pu32 += regid;
	}
#endif
	*pu32 |= (1 << bitid);
}

void atsam2_set_periph_gclk(uint32_t perid, uint8_t gclk)
{
#if defined(MCUSF_DXX)
	GCLK->CLKCTRL.reg = 0
		| (1     << 14)  // enable
		| (gclk  <<  8)  // select GCLK (generator)
		| (perid <<  0)  // select peripheral
	;
#else	// E5X, C2X
	GCLK->PCHCTRL[perid].reg = 0
		| (1 << 6)     // enable
		| (gclk << 0)  // select generator
	;
#endif
}

void atsam2_gclk_setup(uint8_t genid, uint8_t reference, uint32_t division)
{
#if defined(MCUSF_DXX)

	GCLK->GENCTRL.reg = 0
	  | (genid     <<  0)  // ID(4)
		| (reference <<  8)  // SRC(5)
		| (1         << 16)  // GENEN
		| (1         << 17)  // IDC
		| (1         << 21)  // RUNSTDBY
	;

	GCLK->GENDIV.reg  = 0
    | (genid     <<  0)  // ID(4)
		| ((division - 1)  <<  8)  // DIV(16)
	;

	while (GCLK->STATUS.bit.SYNCBUSY)
	{
		// wait until synced
	}

#else
	GCLK->GENCTRL[genid].reg = 0
		| (reference << 0) // SRC(5)
		| (1        <<  8) // GENEN: 1 = ENABLE
		| (1        <<  9) // IDC
		| (0        << 10) // OOV
		| (0        << 11) // OE: output enable
		| (0        << 12) // DIVSEL: 0 = normal division, 1 = 2^DIVSEL division
		| (1        << 13) // RUNSTDBY: 1 = run in standby
		| ((division - 1) << 16) // DIV(16)
	;

	while (GCLK->SYNCBUSY.reg & (1 << (GCLK_SYNCBUSY_GENCTRL0_Pos + genid)))
	{
		// wait until synced
	}

#endif

}

