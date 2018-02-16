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
 *  file:     hwclkctrl_xmc.cpp
 *  brief:    XMC MCU Clock / speed setup
 *  version:  1.00
 *  date:     2018-02-10
 *  authors:  nvitya
*/

#include "platform.h"
#include "hwclkctrl.h"

#include "xmc_utils.h"

#ifdef MCUSF_1000

bool THwClkCtrl_xmc::ExtOscReady()
{
  return true;
}

void THwClkCtrl_xmc::StartExtOsc()
{
}

bool THwClkCtrl_xmc::IntHSOscReady()
{
  return true;  // always running
}

void THwClkCtrl_xmc::StartIntHSOsc()
{
}

void THwClkCtrl_xmc::PrepareHiSpeed(unsigned acpuspeed)
{
	// Flash wait states ???
}

bool THwClkCtrl_xmc::SetupPlls(bool aextosc, unsigned abasespeed, unsigned acpuspeed)
{
	if (aextosc)
	{
		// not implemented
		return false;
	}

	// unlock first
	XMC_SCU_UnlockProtectedBits();

  // the acpuspeed here means peripheral speed !!!
  unsigned basespeed = acpuspeed;
  unsigned pclk2x = 0;
  if (basespeed > 32000000)
  {
  	basespeed = (basespeed >> 1);
  	pclk2x = 1;
  }

	unsigned div = (256 * 32000) / (basespeed / 1000);
	unsigned idiv = (div >> 8);
	unsigned fdiv = (div & 0xFF);

	SCU_CLK->CLKCR = 0
		| (0 << 20)  // CNTADJ(10): 64 MHz base clock adjust
		| (0 << 17)  // RTCCLKSEL(3): RTC select
		| (pclk2x << 16)
		| (idiv << 8)
		| (fdiv << 0)
	;

  // Close the lock
	XMC_SCU_LockProtectedBits();

  return true;
}

#else

  #error "Clock not implemented."

#endif
