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
	// automatic Flash wait state is enabled by default
#ifndef OSCCTRL
	NVMCTRL->CTRLB.bit.RWS = 1;
	NVMCTRL->CTRLB.bit.CACHEDIS = 0;
#endif

#if defined(CMCC)
	// enable cache
	CMCC->CTRL.bit.CEN = 1;
#endif
}

#ifdef OSCCTRL

// ATSAME5X / ATSAMC2x

bool THwClkCtrl_atsam_v2::SetupPlls(bool aextosc, unsigned abasespeed, unsigned acpuspeed)
{
	if (!aextosc)
	{
		return false;  // internal osc not implemented
	}

	// Setup DPLL0

	uint32_t tmpldr;
	uint8_t  tmpldrfrac;

	unsigned refclk;
	unsigned refdiv = 0;
	refclk = abasespeed / (2 * (refdiv + 1));

	// Calculate LDRFRAC and LDR
	tmpldr = (acpuspeed << 4) / refclk;
	tmpldrfrac = tmpldr & 0x0f;
	tmpldr = (tmpldr >> 4) - 1;

#if defined(MCUSF_E5X)

	OSCCTRL->Dpll[0].DPLLRATIO.reg = (tmpldr << 0) | (tmpldrfrac << 16);

	OSCCTRL->Dpll[0].DPLLCTRLB.reg = 0
		| (refdiv << 16)
		| (1 << 11)  // Lock bypass
		| (0 <<  8)  // lock time
		| (2 <<  5)  // reference clock
	;

	OSCCTRL->Dpll[0].DPLLCTRLA.reg = (1 << 6) | (1 << 1);  // run in standby, enable

	while (!(OSCCTRL->Dpll[0].DPLLSTATUS.bit.LOCK))
	{
		// wait for lock
	}

	// set up DPLL0 as main clock
	//GCLK->GENCTRL[0].reg = 0x00000100; // select XOSC0
	//GCLK->GENCTRL[0].reg = 0x00000106; // select DFLL48
	GCLK->GENCTRL[0].reg = 0x00010107; // select DPLL0
	while (GCLK->SYNCBUSY.bit.GENCTRL0)
	{
		// wait until synced
	}

	// use GCLK3 as slow clock from the internal 32k source
	GCLK->GENCTRL[3].reg = 0x00010104;
	while (GCLK->SYNCBUSY.bit.GENCTRL3)
	{
		// wait until synced
	}

	// turn on the slow clock, some peripherals require it
	GCLK->PCHCTRL[3].reg = 0x00000043;

	MCLK->HSDIV.reg = 1;
	MCLK->CPUDIV.reg = 1;

	//MCLK->APBBMASK.bit.RAMECC_ = 0;

#else  // MCUSF_C2X

	OSCCTRL->DPLLRATIO.reg = (tmpldr << 0) | (tmpldrfrac << 16);

	OSCCTRL->DPLLCTRLB.reg = 0
		| (refdiv << 16)
		| (0 << 12)  // Lock bypass
		| (0 <<  8)  // lock time
		| (1 <<  4)  // reference clock, 1 = XOSC
	;

	OSCCTRL->DPLLCTRLA.reg = (1 << 6) | (1 << 1);  // run in standby, enable

	while (!(OSCCTRL->DPLLSTATUS.bit.LOCK))
	{
		// wait for lock
	}

	// set up DPLL0 as main clock
	GCLK->GENCTRL[0].reg = 0x00010107; // select DPLL96
	while (GCLK->SYNCBUSY.bit.GENCTRL & (1 << (2 + 0))) // wait until GENCLK SYNC 0 busy
	{
		// wait until synced
	}

	// use GCLK3 as slow clock from the internal 32k source
	GCLK->GENCTRL[3].reg = 0x00010104;
	while (GCLK->SYNCBUSY.bit.GENCTRL & (1 << (2 + 3)))  // wait until GENCLK SYNC 3 busy
	{
		// wait until synced
	}

	// turn on the slow clock, some peripherals require it
	//GCLK->PCHCTRL[3].reg = 0x00000043;

	//MCLK->CPUDIV.reg = 1;

#endif

	return true;
}

#else

bool THwClkCtrl_atsam_v2::SetupPlls(bool aextosc, unsigned abasespeed, unsigned acpuspeed)
{
	if (aextosc)
	{
		// TODO: implement using FDPLL / FDPLL96M
		return false;
	}
	else
	{
		// internal oscillator
		if (acpuspeed != 48000000)
		{
			// TODO: implement other speed
			return false;
		}

		// GCLK0: main clock
		GCLK->GENCTRL.reg = 0x00010700; // Select DFLL48M
		GCLK->GENDIV.reg  = 0x00000000; // no division for GCLK0
	}

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

	return true;
}

#endif
