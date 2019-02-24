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
 *  file:     hwclkctrl_atsam_v2.cpp
 *  brief:    ATSAM_V2 MCU Clock / speed setup
 *  version:  1.00
 *  date:     2018-02-10
 *  authors:  nvitya
*/

#include "platform.h"
#include "hwclkctrl.h"
#include "atsam_v2_utils.h"

bool THwClkCtrl_atsam_v2::ExtOscReady()
{
#if defined(OSCCTRL_STATUS_XOSCRDY0)
	return (OSCCTRL->STATUS.bit.XOSCRDY0 != 0);
#elif defined(OSCCTRL_STATUS_XOSCRDY)
	return (OSCCTRL->STATUS.bit.XOSCRDY != 0);
#else
	return (SYSCTRL->PCLKSR.bit.XOSCRDY != 0);
#endif
}

void THwClkCtrl_atsam_v2::StartExtOsc()
{
#ifdef OSCCTRL
	// ATSAME5X
	MCLK->APBAMASK.bit.OSCCTRL_ = 1;
	MCLK->AHBMASK.bit.NVMCTRL_ = 1;
	MCLK->APBAMASK.bit.GCLK_ = 1;

	#if defined(OSCCTRL_STATUS_XOSCRDY0)
		OSCCTRL->XOSCCTRL[0].bit.STARTUP = 8; // ~ 8 ms
		OSCCTRL->XOSCCTRL[0].bit.ENALC = 1; // automatic amplitude
		OSCCTRL->XOSCCTRL[0].bit.ONDEMAND = 0; // always run
		// settings for 8-16 MHz Crystal:
		OSCCTRL->XOSCCTRL[0].bit.XTALEN = 1;
		OSCCTRL->XOSCCTRL[0].bit.IMULT = 4;
		OSCCTRL->XOSCCTRL[0].bit.IPTAT = 3;

		OSCCTRL->XOSCCTRL[0].bit.ENABLE = 1;
	#elif defined(OSCCTRL_STATUS_XOSCRDY)
		OSCCTRL->XOSCCTRL.bit.STARTUP = 8; // ~ 8 ms
		OSCCTRL->XOSCCTRL.bit.GAIN = 3; // for 12 MHz Crystal
		OSCCTRL->XOSCCTRL.bit.AMPGC = 1; // automatic amplitude
		OSCCTRL->XOSCCTRL.bit.ONDEMAND = 0; // always run
		// settings for 8-16 MHz Crystal:
		OSCCTRL->XOSCCTRL.bit.XTALEN = 1;

		OSCCTRL->XOSCCTRL.bit.ENABLE = 1;
	#endif

#else
	SYSCTRL->XOSC.bit.STARTUP = 8;
	SYSCTRL->XOSC.bit.AMPGC = 1;
	SYSCTRL->XOSC.bit.ONDEMAND = 0;
	SYSCTRL->XOSC.bit.XTALEN = 1;

	SYSCTRL->XOSC.bit.ENABLE = 1;
#endif
}

bool THwClkCtrl_atsam_v2::IntHSOscReady()
{
	return true;
}

void THwClkCtrl_atsam_v2::StartIntHSOsc()
{
#ifdef OSCCTRL
#else
	// turn on the DFLL48M

	GCLK->GENCTRL.reg = 0x00010600; // Select OSC8M as main clock source

	unsigned calibdata = *(unsigned *)(0x00806024);
	unsigned dfllcoarse = (calibdata >> 26);

	// stop DFLL
	SYSCTRL->DFLLCTRL.reg = 0
		| (0 << 2)   // 0 = open loop mode
		| (0 << 1)   // enable
	;

	// set the coarse calibration value
	SYSCTRL->DFLLVAL.bit.COARSE = dfllcoarse;
	SYSCTRL->DFLLVAL.bit.FINE = 512;

	// start it
	SYSCTRL->DFLLCTRL.reg = 0
		| (0 << 2)   // 0 = open loop mode
		| (1 << 1)   // enable
	;

	while (!SYSCTRL->PCLKSR.bit.DFLLRDY)
	{
		// wait
	}
#endif
}

void THwClkCtrl_atsam_v2::PrepareHiSpeed(unsigned acpuspeed)
{
#if defined(MCUSF_E5X)

	// automatic Flash wait state is enabled by default

#if defined(CMCC)

	// this family has a slow flash system, which can degrade the execution speed below 50%

	// enable cache, with data cacheing disabled

	// data cacheing can provide quite problematic effects specially with DMA, so its better to keep it off

	CMCC->CTRL.reg = 0;
	while (CMCC->SR.bit.CSTS) { } // wait until the cache is disabled
	CMCC->CFG.bit.DCDIS = 1; // disable Data cacheing
	CMCC->CFG.bit.ICDIS = 0; // enable instruction cacheing
	CMCC->CTRL.reg = 1; // enable the cache module

#endif

#elif defined(MCUSF_C2X) || defined(MCUSF_DXX)

	if (acpuspeed > 38000000)
	{
		NVMCTRL->CTRLB.bit.RWS = 2;
	}
	else if (acpuspeed > 19000000)
	{
		NVMCTRL->CTRLB.bit.RWS = 1;
	}
	else
	{
		NVMCTRL->CTRLB.bit.RWS = 0;
	}

	NVMCTRL->CTRLB.bit.CACHEDIS = 0;
#else
  #warning "unimplemented FLASH wait states !"
#endif

}

// GCLK allocation:
//   GCLK0: CPU Clock and Main Peripheral clock (DFPLL / DPLL)
//   GCLK1: reserved for low frequencies (only this GCLK has 16 bit divider), and this can be used as source for other GCLK
//   GCLK2: 48 MHz from the internal DFLL48M (the I2C requires that)
//   GCLK3: 32 kHz Slow Clock

bool THwClkCtrl_atsam_v2::SetupPlls(bool aextosc, unsigned abasespeed, unsigned acpuspeed)
{
#if defined(MCUSF_DXX)
	uint32_t dfll48id = 7;
	uint32_t pllid = 8;
#else
	uint32_t dfll48id = 6;
	uint32_t pllid = 7;
#endif

	uint32_t xoscid = 0;

	if (!aextosc)
	{
		// use the internal 48 MHz oscillator

		abasespeed = 48000000;

		uint32_t division = abasespeed / acpuspeed;
		if (division < 1)  division = 1;

		atsam2_gclk_setup(0, dfll48id, division);  // use the DFLL48M directly as main clock (GCLK0)
	}
	else
	{
		// The DPLLs have the maximum reference input clock frequency of 2 MHz (3.2 MHz on E5X)
		// so we have to divide the XOSC clock to 2 MHz, a fix /2 is included

		unsigned refdiv = abasespeed / (2 * 2000000);  // divisor for 2 MHz
		if (refdiv < 1)  refdiv = 1;

		uint32_t cpumul = acpuspeed / 2000000;

		// the fractional divider won't be used because it makes jitter

#if defined(MCUSF_E5X)

		OSCCTRL->Dpll[0].DPLLRATIO.reg = 0
			| ((cpumul - 1) <<  0)  // LDR(13): loop divider
			| (0            << 16)  // LDRFRAC(5): fractional
		;

		OSCCTRL->Dpll[0].DPLLCTRLB.reg = 0
			| ((refdiv - 1) << 16)  // DIV(11): reference divisor = (2 * (DIV + 1))
			| (1            << 11)  // LBYPASS: Lock bypass
			| (0            <<  8)  // LTIME(3): lock time
			| (2            <<  5)  // REFCLK(3): 2 = XOSC0
		;

		OSCCTRL->Dpll[0].DPLLCTRLA.reg = (1 << 6) | (1 << 1);  // run in standby, enable

		while (!(OSCCTRL->Dpll[0].DPLLSTATUS.bit.LOCK))
		{
			// wait for lock
		}

#elif defined(MCUSF_C2X)

		OSCCTRL->DPLLRATIO.reg = 0
			| ((cpumul - 1) <<  0)  // LDR(13): loop divider
			| (0            << 16)  // LDRFRAC(5): fractional
		;

		OSCCTRL->DPLLCTRLB.reg = 0
			| ((refdiv - 1) << 16)  // DIV(11): reference divisor = (2 * (DIV + 1))
			| (1            << 11)  // LBYPASS: Lock bypass
			| (0            <<  8)  // LTIME(3): lock time
			| (2            <<  5)  // REFCLK(3): 2 = XOSC0
		;

		OSCCTRL->DPLLCTRLA.reg = (1 << 6) | (1 << 1);  // run in standby, enable

		while (!(OSCCTRL->DPLLSTATUS.bit.LOCK))
		{
			// wait for lock
		}

#else

	  return false; // PLL is not implemented for this family

#endif

		atsam2_gclk_setup(0, pllid, 1);  // use the PLL as main clock (GCLK0)
	}

	// Other setups

#if defined(MCUSF_E5X)

	MCLK->HSDIV.reg = 1;
	MCLK->CPUDIV.reg = 1;

	//MCLK->APBBMASK.bit.RAMECC_ = 0;

	atsam2_gclk_setup(2, 6, 1);  // GCLK2 = DFLL48M

	// use GCLK3 as slow clock from the internal 32k source
	atsam2_gclk_setup(3, 4, 1);

	// turn on the slow clock, some peripherals (e.g. SERCOM) require it
	GCLK->PCHCTRL[3].reg = 0x00000043;

#elif defined(MCUSF_C2X)

	MCLK->CPUDIV.reg = 1;

	atsam2_gclk_setup(2, 6, 1);  // GCLK2 = OSC48M

	// use GCLK3 as slow clock from the internal 32k source
	atsam2_gclk_setup(3, 4, 1);

#elif defined(MCUSF_DXX)

	// GCLK1: slow clock
	GCLK->GENDIV.reg  = 0x00000001; // no division for GCLK1
	GCLK->GENCTRL.reg = 0x00010401; // Select OSC32K

	// GCLK2: RTC clock
	GCLK->GENDIV.reg  = 0x00001F02; // /32  for GCLK2
	GCLK->GENCTRL.reg = 0x00010402; // Select OSC32K

	// select clock generators
	GCLK->CLKCTRL.reg = 0x410D; // GCLK1 for SERCOM slow

	// GCLK3: sercom base
	GCLK->GENCTRL.reg = 0x00010703; // Select DFLL48M
	GCLK->GENDIV.reg  = 0x00000003; // no division for GCLK3

#else
  #error "Implement clock settings for this subfamily"
#endif

	return true;
}

