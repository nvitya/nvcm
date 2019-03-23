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
 *  file:     hwclkctrl_xmc1.cpp
 *  brief:    XMC1000 MCU Clock / speed setup
 *  version:  1.00
 *  date:     2018-02-10
 *  authors:  nvitya
*/

#include "platform.h"

#ifdef MCUSF_1000

#include "hwclkctrl.h"

#include "xmc_utils.h"

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

#ifdef SCU_GENERAL_PFUCR_PFUBYP_Msk
  SCU_GENERAL->PFUCR &= ~SCU_GENERAL_PFUCR_PFUBYP_Msk;
#endif

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

#if UC_SERIES == XMC14  // 48 MHz variant

  unsigned dcospeed = 48000000;

	SCU_CLK->CLKCR1 = 0; // select the internal oscillator

#else

  unsigned dcospeed = 32000000;

#endif

	unsigned idiv = dcospeed / acpuspeed;

	SCU_CLK->CLKCR = 0
		| (0    << 20)  // CNTADJ(10):
		| (0    << 17)  // RTCCLKSEL(3): RTC select, 0 = 32 kHz standby clock
		| (1    << 16)  // PCLKSEL: 1 = PCLK = 2xMCLK, 0 = PCLK = MCLK
		| (idiv <<  8)  // IDIV(8): integer divider
		| (0    <<  0)  // FDIV(8): fractional divider
	;

  // Close the lock
	XMC_SCU_LockProtectedBits();

	SystemCoreClock = acpuspeed;  // store it temorarily

  return true;
}

void THwClkCtrl_xmc::SyncToExtOsc(unsigned aextspeed)
{
#if UC_SERIES == XMC14 // available only on the XMC1400

  XMC_SCU_UnlockProtectedBits();

	// start the external oscillator
	SCU_ANALOG->ANAOSCHPCTRL = 0
		| (0 <<  1)  // SHBY
		| (2 <<  2)  // GAINSEL(2): 2 = 4 - 20 MHz crystal
		| (0 <<  4)  // MODE(2): 0 = oscillator mode
		| (0 <<  6)  // HYSCTRL: 0 = hysteresis control off
	;

	// wait at least 10 ms until the OSC stabilizes
  uint32_t delaycnt = 24000;
  while (delaycnt > 0)  { --delaycnt; }

  //XMC_SCU_CLOCK_EnableDCO1ExtRefCalibration(XMC_SCU_CLOCK_SYNC_CLKSRC_OSCHP, 1000, 3000);
  //while (XMC_SCU_CLOCK_IsDCO1ExtRefCalibrationReady() == false);

  uint32_t ratio = SystemCoreClock / aextspeed; // extspeed must be lower !

  uint32_t prescaler = 1000;
  uint32_t syn_preload = ratio * prescaler;


  SCU_ANALOG->ANASYNC2 = 0
  	| (prescaler << 0) // PRESCALER(11)
  ;

  SCU_ANALOG->ANASYNC1 = 0
  	| (syn_preload <<  0)  // SYNC_PRELOAD(14):
  	| (1           << 14)  // SYNC_DCO_EN: 1 = enable sync
		| (1           << 15)  // XTAL_SEL: 1 = OSC_HP
	;

  XMC_SCU_LockProtectedBits();

  while ((SCU_ANALOG->ANASYNC2 & SCU_ANALOG_ANASYNC2_SYNC_READY_Msk) == 0)
	{
  	// wait until the synchronization is ready
	}

#endif
}

#endif // def MCUSF_1000
