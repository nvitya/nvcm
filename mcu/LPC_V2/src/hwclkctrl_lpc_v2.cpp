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
 *  file:     hwclkctrl_lpc_v2.cpp
 *  brief:    LPC_V2 MCU Clock / speed setup.
 *  version:  1.00
 *  date:     2018-02-10
 *  authors:  nvitya
 *  notes:
 *    Only 30 MHz and 24 MHz are supported !
*/

#include "platform.h"
#include "hwclkctrl.h"

void THwClkCtrl_lpc_v2::StartIntHSOsc()
{
  // the 12MHz internal RC oscillator is running by default.
}

bool THwClkCtrl_lpc_v2::IntHSOscReady()
{
	return true;
}

bool THwClkCtrl_lpc_v2::ExtOscReady()
{
  return true;
}

void THwClkCtrl_lpc_v2::StartExtOsc()
{
  LPC_SYSCON->PDRUNCFG &= ~(1 << 5);
}

void THwClkCtrl_lpc_v2::PrepareHiSpeed(unsigned acpuspeed)
{
  uint32_t ws = 0;
  if (acpuspeed > 20000000)  ws = 1;

  uint32_t tmp = LPC_FMC->FLASHCFG;
  tmp &= ~(0x3);
  tmp |= ws;
  LPC_FMC->FLASHCFG = tmp;
}

bool THwClkCtrl_lpc_v2::SetupPlls(bool aextosc, unsigned abasespeed, unsigned acpuspeed)
{
  LPC_SYSCON->PDRUNCFG |= (1 << 7);  // // Disable PLL power

  if (aextosc)
  {
  	LPC_SYSCON->SYSPLLCLKSEL = 1;
  }
  else
  {
  	LPC_SYSCON->SYSPLLCLKSEL = 0;
  }

  // update clock source sequence
	LPC_SYSCON->SYSPLLCLKUEN = 0;
	LPC_SYSCON->SYSPLLCLKUEN = 1;

	unsigned msel = 0;
	unsigned psel = 0;
	unsigned ahbdiv = 1;

	// fcco = fin * 2 * P * M, must be between 156 ... 320 MHz !!!!

	if ((abasespeed == 12000000) && (acpuspeed == 30000000))
	{
		msel = 4;
		psel = 1;
		ahbdiv = 2;
	}
	else if ((abasespeed == 12000000) && (acpuspeed == 24000000))
	{
		msel = 1;
		psel = 2;
		ahbdiv = 1;
	}
	else
	{
		return false;
	}

/*
	unsigned q = acpuspeed / abasespeed;
	unsigned rem = acpuspeed - (q * abasespeed);

	if (rem == 0)
	{
		msel = q - 1;
	}
	else if (rem * 2 == abasespeed)
	{
		psel = 1;
		msel = 2 * acpuspeed / abasespeed;
	}
	else if (rem * 4 == abasespeed)
	{
		psel = 2;
		msel = 4 * acpuspeed / abasespeed;
	}
	else if (rem * 8 == abasespeed)
	{
		psel = 3;
		msel = 8 * acpuspeed / abasespeed;
	}
	else
	{
		return false;
	}

*/

	LPC_SYSCON->SYSPLLCTRL = ((psel << 5) | msel);

  LPC_SYSCON->PDRUNCFG &= ~(1 << 7);  // Enable power to the PLL

	while ((LPC_SYSCON->SYSPLLSTAT & 1) == 0) { } // wait for lock

	// change the main clock to PLL
	LPC_SYSCON->MAINCLKSEL = 3;

	LPC_SYSCON->MAINCLKUEN = 0;
	LPC_SYSCON->MAINCLKUEN = 1;

	LPC_SYSCON->SYSAHBCLKDIV = ahbdiv;

  return true;
}
