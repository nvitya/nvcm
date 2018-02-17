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
 *  file:     hwclkctrl_kinetis.cpp
 *  brief:    KIENTIS MCU Clock / speed setup
 *  version:  1.00
 *  date:     2018-02-10
 *  authors:  nvitya
*/

#include "platform.h"
#include "hwclkctrl.h"

#ifdef MCUSF_KL03

bool THwClkCtrl_kinetis::ExtOscReady()
{
  return true;
}

void THwClkCtrl_kinetis::StartExtOsc()
{

}

bool THwClkCtrl_kinetis::IntHSOscReady()
{
	return true;
}

void THwClkCtrl_kinetis::StartIntHSOsc()
{
	MCG->MC |= MCG_MC_HIRCEN_MASK;
}

void THwClkCtrl_kinetis::PrepareHiSpeed(unsigned acpuspeed)
{
	// ???
}

bool THwClkCtrl_kinetis::SetupPlls(bool aextosc, unsigned abasespeed, unsigned acpuspeed)
{
	if (aextosc)
	{
		// TODO: implement external oscillator
		return false;
	}

	if (acpuspeed == 48000000)
	{
		// select HIRC
		MCG->C1 &= ~(3 << 6); // 00 = select HIRC as main clock source
		while ((MCG->S & (3 << 2)) != 0)
		{
			// wait until HIRC is selected
		}

		return true;
	}
	else if (acpuspeed <= 8000000)
	{
		// Select LIRC
		// TODO: implement LIRC
		return false;
	}

  return false;
}

#elif defined(MCUSF_KV30)

bool THwClkCtrl_kinetis::ExtOscReady()
{
  return true;
}

void THwClkCtrl_kinetis::StartExtOsc()
{

}

bool THwClkCtrl_kinetis::IntHSOscReady()
{
	return true;
}

void THwClkCtrl_kinetis::StartIntHSOsc()
{

}

void THwClkCtrl_kinetis::PrepareHiSpeed(unsigned acpuspeed)
{
	SMC->PMPROT = 0x80;
  SMC->PMCTRL = (3 << 5); // set High Speed Run Mode
  while (SMC->PMSTAT != 0x80)
  {
  	// wait until High Speed Power mode set
  }
}

bool THwClkCtrl_kinetis::SetupPlls(bool aextosc, unsigned abasespeed, unsigned acpuspeed)
{
	unsigned n;

	if (aextosc)
	{
		// TODO: implement external oscillator
		return false;
	}

	uint8_t tmp;

	// select a safe clock first
	MCG->C1 |= (1 << 2); // select the internal slow clock (already selected)

	MCG->C7 = 2; // select the IRC48M oscillator
	for (n = 0; n < 5000; ++n)  __NOP();

	uint32_t tmp32;

	SIM->SOPT2 |= (3 << 16); // PLLFLLSEL: select IRC48M

	// set up safe divisors for 96 MHz
	SIM->CLKDIV1 = 0
		| (7 << 16)  // OUTDIV4: flash, 7 = /8
		| (1 << 24)  // OUTDIV2: bus, 1 = /2
		| (0 << 28)  // OUTDIV1: core, 0 = /1
	;

	// set CLKS until we change che FLL settings
	tmp = MCG->C1;
	tmp &= 0x3F;
	tmp |= 0x80;  // select IRC48, bypass the FLL
	MCG->C1 = tmp;

	if (acpuspeed == 48000000)
	{
		// correct divisors
		SIM->CLKDIV1 = 0
			| (2 << 16)  // OUTDIV4: flash, 1 = /3
			| (0 << 24)  // OUTDIV2: bus, 0 = /1
			| (0 << 28)  // OUTDIV1: core, 0 = /1
		;
		return true;
	}

	if (acpuspeed == 96000000)  // the highest clock speed using the internal RC clock
	{
		tmp = MCG->C2;
		tmp &= 0xCF;
		tmp |= 0x20; // RANGE, 2 = Select high freq range
		MCG->C2 = tmp;

		tmp = MCG->C1;
		tmp &= ~(7 << 3);
		tmp |=  (6 << 3);  // FRDIV = divide by 1280 when RANGE >= 2
		MCG->C1 = tmp;

		tmp = MCG->C4;
		tmp &= ~(7 << 5);
		tmp |=  (3 << 5); // select DCO multiplication for 96 MHz
		MCG->C4 = tmp;

		tmp = MCG->C1;
		tmp &= ~(1 << 2);  // clear IREFS
		MCG->C1 = tmp;

		tmp = MCG->C1;
		tmp &= ~(3 << 6);
		tmp |=  (0 << 6);  // select FLL as clock
		MCG->C1 = tmp;

		while ((MCG->S & (3 << 2)) != 0)
		{
			// wait until FLL is selected
		}

		// wait until it is stable
		for (n = 0; n < 5000; ++n)  __NOP();

#if 1
		// set the flash divisor to fastest

		// According to specification the maximal speed of the Flash unit is 25 MHz.
		// It should be run fine with 24 MHz (OUTDIV4 = 3), however the test piece was
		// not stabile with that. The lowest working divisor was 5 (OUTDIV4 = 4), resulting around 20 MHz.
		SIM->CLKDIV1 = 0
			| (4 << 16)  // OUTDIV4: flash, 4 = /5
			| (1 << 24)  // OUTDIV2: bus, 1 = /2
			| (0 << 28)  // OUTDIV1: core, 0 = /1
		;
#endif

		return true;
	}

  return false;
}

#elif defined(MCUSF_K20)

bool THwClkCtrl_kinetis::ExtOscReady()
{
	return ((MCG->S & MCG_S_OSCINIT0_MASK) == 0);
}

void THwClkCtrl_kinetis::StartExtOsc()
{
	//MCG->C2 = MCG_C2_EREFS0_MASK | MCG_C2_HGO_MASK | MCG_C2_RANGE0(2);  // prepare for high speed crystal (16MHz)

	OSC->CR = (OSC_CR_ERCLKEN_MASK | OSC_CR_EREFSTEN_MASK); // | OSC_CR_SC8P_MASK);
}

bool THwClkCtrl_kinetis::IntHSOscReady()
{
	return true;
}

void THwClkCtrl_kinetis::StartIntHSOsc()
{
	MCG->MC |= MCG_MC_HIRCEN_MASK;
}

void THwClkCtrl_kinetis::PrepareHiSpeed(unsigned acpuspeed)
{
	// automatic flash wait state control
}

bool THwClkCtrl_kinetis::SetupPlls(bool aextosc, unsigned abasespeed, unsigned acpuspeed)
{
	unsigned tmp;

	if (!aextosc)
	{
		// TODO: implement internal oscillator
		return false;
	}

	for (tmp = 0; tmp < 1000; ++tmp) { __NOP(); }

	MCG->C7 = 0;

	MCG->C1 = 0
		| MCG_C1_CLKS(2)  // select external clock
		| MCG_C1_IRCLKEN_MASK  // keep enable internal RC
		| MCG_C1_IREFS(0)  // select internal RC as reference
	;

//  while (MCG->S & MCG_S_IREFST_MASK)
//  {
//  }

  while ((MCG->S & MCG_S_CLKST_MASK) != (2 << 2))
  {
  }
	//for (tmp = 0; tmp < 5000; ++tmp) { __NOP(); }

	// enable pll
	MCG->C5 &= ~MCG_C5_PLLCLKEN0_MASK;  // disable pll
	while ((MCG->S & MCG_S_LOCK0_MASK) == 1)
	{
	}

#if 0
	unsigned prdiv = abasespeed / 2000000;  // to have 2 MHz base clock
	unsigned mul = acpuspeed / 2000000;  // = VDIV

	tmp = MCG->C6;
	tmp &= ~MCG_C6_VDIV0_MASK;
	tmp |= ((mul - 1) & MCG_C6_VDIV0_MASK);
	MCG->C6 = tmp;

	MCG->C5 = MCG_C5_PLLCLKEN0_MASK | MCG_C5_PLLSTEN0_MASK | MCG_C5_PRDIV0(prdiv-1);

/*
	// wait for PLL lock
	while ((MCG->S & MCG_S_LOCK0_MASK) == 0)
	{
	}
*/

	// select the PLL as the clock source
	tmp = MCG->C6;
	tmp &= ~MCG_C6_PLLS_MASK;
	tmp |= MCG_C6_PLLS(1);
	MCG->C6 = tmp;
	// wait until it is selected
	while ((MCG->S & MCG_S_PLLST_MASK) == 0)
	{
	}

/*
	// select PLL for MCGPLL/MCGFLL
	SIM->SOPT2 |= SIM_SOPT2_PLLFLLSEL_MASK;
*/

#endif

	//MCG->C1 = 0x80;  // select ext clock for base clock
	//MCG->C1 = 0x40;  // internal clock

  return true;
}

#else
  //#error "clock control is not implemented."
#endif
