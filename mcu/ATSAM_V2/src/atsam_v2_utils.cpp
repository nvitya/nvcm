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

const Sercom * sercom_inst_list[] = SERCOM_INSTS;

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
		| (0         << 20)  // DIVSEL
		| (1         << 21)  // RUNSTDBY
	;

	GCLK->GENDIV.reg  = 0
    | (genid     <<  0)  // ID(4)
		| (division  <<  8)  // DIV(16)
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
		| (division << 16) // DIV(16)
	;

	#ifdef GCLK_SYNCBUSY_GENCTRL0_Pos
		while (GCLK->SYNCBUSY.reg & (1 << (GCLK_SYNCBUSY_GENCTRL0_Pos + genid)))
		{
			// wait until synced
		}

	#else
		while (GCLK->SYNCBUSY.bit.GENCTRL)
		{
			// wait until synced
		}

	#endif

#endif

}

bool atsam2_sercom_enable(int devnum, uint8_t clksrc)
{
	unsigned perid;

	if (devnum < 0)
	{
		return false;
	}
	else if (devnum >= SERCOM_INST_NUM)
	{
		return false;
	}
#if defined(MCUSF_E5X)
	else if (devnum < 2)
	{
		MCLK->APBAMASK.reg |= (1 << (12 + devnum));  // Enable/unmask CPU interface (register access)
		perid = 7 + devnum;
	}
	else if (devnum < 4)
	{
		MCLK->APBBMASK.reg |= (1 << (9 + devnum - 2)); // Enable/unmask CPU interface (register access)
		perid = 23 + (devnum - 2);
	}
	else if (devnum < 8)
	{
		MCLK->APBDMASK.reg |= (1 << (devnum - 4)); // Enable/unmask CPU interface (register access)
		perid = 34 + (devnum - 4);
	}
	else
	{
		return false;
	}

	// setup peripheral clock
	GCLK->PCHCTRL[perid].reg = ((clksrc << 0) | (1 << 6));   // select main clock frequency (120 MHz) + enable

#elif defined(MCUSF_C2X)
	else if (devnum <= 5)
	{
		MCLK->APBCMASK.reg |= (1 << (1 + devnum));  // enable register interface
		perid = 19 + devnum;
		if (devnum == 5)
		{
			GCLK->PCHCTRL[25].reg = (1 << 6) | (clksrc << 0);  // Enable the peripheral and select clock source
			GCLK->PCHCTRL[24].reg = (1 << 6) | (3 << 0);  // Select the SERCOM5 slow clock
		}
		else
		{
			GCLK->PCHCTRL[19 + devnum].reg = (1 << 6) | (clksrc << 0);  // Enable the peripheral and select the clock source
			GCLK->PCHCTRL[18].reg = (1 << 6) | (3 << 0);  // Select the SERCOM slow clock
		}
	}
	else
	{
		return false;
	}
#elif defined(MCUSF_D10)
	else
	{
		PM->APBCMASK.reg |= (1 << (2 + devnum));  // enable register interface
		GCLK->CLKCTRL.reg = 0x430E + devnum;      // Select GCLK3 for SERCOM
	}

#else
  #error "UART Unimplemented."
#endif

	return true;
}
