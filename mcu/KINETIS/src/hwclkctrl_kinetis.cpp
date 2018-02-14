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

#elif defined(MCUSF_K20) // || defined(MCUSF_KV30)

bool THwClkCtrl_kinetis::ExtOscReady()
{
	return ((MCG->S & MCG_S_OSCINIT0_MASK) == 0);
}

void THwClkCtrl_kinetis::StartExtOsc()
{
	MCG->C2 = MCG_C2_EREFS0_MASK | MCG_C2_HGO_MASK | MCG_C2_RANGE0(2);  // prepare for high speed crystal (16MHz)

	OSC->CR = (OSC_CR_ERCLKEN_MASK | OSC_CR_EREFSTEN_MASK | OSC_CR_SC8P_MASK);
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
