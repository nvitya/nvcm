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
 *  file:     hwclkctrl_lpc_v3.cpp
 *  brief:    LPC_V3 MCU Clock / speed setup
 *  version:  1.00
 *  date:     2018-02-10
 *  authors:  nvitya
*/

#include "platform.h"
#include "hwclkctrl.h"

bool THwClkCtrl_lpc_v3::ExtOscReady()
{
  return true;
}

void THwClkCtrl_lpc_v3::StartExtOsc()
{
  SYSCON->PDRUNCFGCLR[0] |= SYSCON_PDRUNCFG_PDEN_VD2_ANA_MASK;
  SYSCON->PDRUNCFGCLR[1] |= SYSCON_PDRUNCFG_PDEN_SYSOSC_MASK;
}

// this function is presented in an external library only !
//extern "C" void POWER_SetVoltageForFreq(uint32_t freq);

void THwClkCtrl_lpc_v3::PrepareHiSpeed(unsigned acpuspeed)
{
  uint32_t tmp;

  //  Reverse enginered POWER_SetVoltageForFreq(180000000); /*!< Set voltage for the one of the fastest clock outputs: System clock output */
  unsigned * vcregs = (unsigned * )0x40020000;  // undocumented voltage control registers
  vcregs[0] = 13; // this was originally 11
  vcregs[1] = 12;
  vcregs[2] = 11;
  vcregs[3] = 13; // this was originally 11
  vcregs[4] = 11;
  vcregs[5] = 11;

  tmp = SYSCON->FLASHCFG & ~(SYSCON_FLASHCFG_FLASHTIM_MASK);
  SYSCON->FLASHCFG = tmp | (8 << SYSCON_FLASHCFG_FLASHTIM_SHIFT);  // 9 clock wait state

  SYSCON->FLASHCFG |= ((1 << 5) | (1 << 6)); // prefetch enable + prefetch override enable
}

unsigned lpc54_pll_calc_ndiv(unsigned val)
{
	unsigned max=0x00000100, x=0x00000080;
	switch (val)
	{
		case 0: x = 0xFFFFFFFF; break;
		case 1: x = 0x00000302; break;
		case 2: x = 0x00000202; break;
		default:
			for (int i = val; i <= max; i++)
			{
				x = (((x ^ (x>>2) ^ (x>>3) ^ (x>>4)) & 1) << 7) |	((x>>1) & 0x7F);
			}
	}
	return x;
}

unsigned lpc54_pll_calc_pdiv(unsigned val)
{
	unsigned max=0x20, x=0x10;
	switch (val)
	{
		case 0: x = 0xFFFFFFFF; break;
		case 1: x = 0x00000062; break;
		case 2: x = 0x00000042; break;
		default:
			for (int i = val; i <= max; i++)
			{
				x = (((x ^ (x>>2)) & 1) << 4) | ((x>>1) & 0xF);
			}
	}
	return x;
}

unsigned lpc54_pll_calc_mdiv(unsigned val)
{
	unsigned max=0x00008000, x=0x00004000;
	switch (val)
	{
		case 0: x = 0xFFFFFFFF; break;
		case 1: x = 0x00018003; break;
		case 2: x = 0x00010003; break;
		default:
			for (int i = val; i <= max; i++)
			{
				x = (((x ^ (x>>1)) & 1) << 14) | ((x>>1) & 0x3FFF);
			}
	}

	return x;
}

static const unsigned lpc_pll_m_table[] =
{
		0x00000,  // 0
		0x18003,  // 1
		0x10003,  // 2
		0x0001,   // 3
		0x0003,   // 4
		0x0007,   // 5
		0x000F,   // 6
		0x001F,   // 7
		0x003F,   // 8
		0x007F,   // 9
		0x00FF,   // 10
		0x01FF,   // 11
		0x03FF,   // 12
		0x07FF,   // 13
		0x0FFF,   // 14
		0x1FFF,   // 15
		0x3FFF,   // 16
		0x7FFF,   // 17
		0x7FFE,   // 18
		0x7FFD,   // 19
		0x7FFA,   // 20
		0x7FF5,   // 21
		0x7FEA,   // 22
		0x7FD5,   // 23
		0x7FAA,   // 24
		0x7F55,   // 25
		0x7EAA,   // 26
		0x7D55,   // 27
		0x7AAA,   // 28
		0x7555,   // 29
		0x6AAA,   // 30
		0x5555,   // 31
		0x2AAA,   // 32
		0x5554,   // 33
		0x2AA9,   // 34
		0x5553,   // 35
		0x2AA6,   // 36
		0x554C,   // 37
		0x2A99,   // 38
		0x5533,   // 39
		0x2A66    // 40
};

bool THwClkCtrl_lpc_v3::SetupPlls(bool aextosc, unsigned abasespeed, unsigned acpuspeed)
{
	SYSCON->SYSPLLCLKSEL = 1; // select the external clock input

	// Reverse engineered POWER_SetPLL(void):
  SYSCON->PDRUNCFGCLR[0] = (1 << 26);   // Enable power to the PLLs
  // there was checking for the 0x40020053 bit 26 or 27 but seems not required

  SYSCON->PDRUNCFGSET[0] = (1 << 22);   // Power off PLL during setup changes

	/* Write PLL setup data */
	SYSCON->SYSPLLCTRL = SYSCON_SYSPLLCTRL_SELI(32U) | SYSCON_SYSPLLCTRL_SELP(16U) | SYSCON_SYSPLLCTRL_SELR(0U);

	unsigned pllxdec;

	// the NXP here uses a special encoding (without any explanation why)
	// we set up the dividers (N, P) fix to 1
	// and using a table for the multipier M

	pllxdec = SYSCON_SYSPLLNDEC_NDEC(0x302); // N = 1
	SYSCON->SYSPLLNDEC = pllxdec;
	SYSCON->SYSPLLNDEC = pllxdec | (1 << SYSCON_SYSPLLNDEC_NREQ_SHIFT); /* latch */

	pllxdec = SYSCON_SYSPLLPDEC_PDEC(98); // P = 1
	SYSCON->SYSPLLPDEC = pllxdec;
	SYSCON->SYSPLLPDEC = pllxdec | (1 << SYSCON_SYSPLLPDEC_PREQ_SHIFT); /* latch */

	unsigned pllmul = acpuspeed / abasespeed;
	if (pllmul > 40)  pllmul = 40;
	pllxdec = lpc_pll_m_table[pllmul];
	SYSCON->SYSPLLMDEC = pllxdec;
	SYSCON->SYSPLLMDEC = pllxdec | (1 << SYSCON_SYSPLLMDEC_MREQ_SHIFT); /* latch */

	volatile uint32_t delayX;
	uint32_t maxCCO = (1 << 18) | 0x5dd2; /* CCO = 1.6Ghz + MDEC enabled*/
	uint32_t curSSCTRL = SYSCON->SYSPLLMDEC & ~(1 << 17);

	/* Initialize  and power up PLL */
	SYSCON->SYSPLLMDEC = maxCCO;

  SYSCON->PDRUNCFGCLR[0] = (1 << 22);   // Enable power to the PLL

	/* Set mreq to activate */
	SYSCON->SYSPLLMDEC = maxCCO | (1 << 17);

	/* Delay for 72 uSec @ 12Mhz */
	for (delayX = 0; delayX < 172; ++delayX)
	{

	}

	/* clear mreq to prepare for restoring mreq */

	/* set original value back and activate */
	SYSCON->SYSPLLMDEC = curSSCTRL;
	SYSCON->SYSPLLMDEC = curSSCTRL | (1 << 17);

	while ((SYSCON->SYSPLLSTAT & SYSCON_SYSPLLSTAT_LOCK_MASK) == 0)
	{
		// Wait PLL lock
	}

	// switch main clock to the PLL
	SYSCON->MAINCLKSELB = 2;
	SYSCON->AHBCLKDIV = 0;

	// Setup FROHS to 48 MHz, it will be used as the source for FLEXCOMM units
	// by default the FROHS configured to 96 MHz by the boot rom with trimmed values
	// we just apply a by 2 divisior
	SYSCON->FROHFCLKDIV = 1;  // 1 = divide by two
	SYSCON->FROCTRL |= (1 << 30); // enable FROHS output

  return true;
}
