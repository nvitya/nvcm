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
 *  brief:    XMC4000 MCU Clock / speed setup
 *  version:  1.00
 *  date:     2018-02-10
 *  authors:  nvitya
*/

#include "platform.h"

#ifdef MCUSF_4000

#include "hwclkctrl.h"
#include "xmc_utils.h"

bool THwClkCtrl_xmc::ExtOscReady()
{
  return true;
}

void THwClkCtrl_xmc::StartExtOsc()
{
  SCU_OSC->OSCHPCTRL = 0
  	| (0 <<  0)  // X1DEN
  	| (0 <<  1)  // SHBY
  	| (3 <<  2)  // GAINSEL(2): 3 = 4-25 MHz input crystal
  	| (0 <<  4)  // MODE(2): 0 = external crystal mode
  	| (3 << 16)  // OSCVAL(4): = Crystal freq / 2.5 MHz - 1 ?
  ;
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
  SCB->CCR &= ~(SCB_CCR_UNALIGN_TRP_Msk);

  uint32_t temp;

  temp = FLASH0->FCON;
  temp &= ~FLASH_FCON_WSPFLASH_Msk;
  temp |= 4;
  FLASH0->FCON = temp;
}

#define SCU_PLL_PLLSTAT_OSC_USABLE  (SCU_PLL_PLLSTAT_PLLHV_Msk | \
                                     SCU_PLL_PLLSTAT_PLLLV_Msk | \
                                     SCU_PLL_PLLSTAT_PLLSP_Msk)

bool THwClkCtrl_xmc::SetupPlls(bool aextosc, unsigned abasespeed, unsigned acpuspeed)
{
	// setup a safe clock first
	SCU_CLK->SYSCLKCR = 0;  // select the internal fast oscillator first

	if (!aextosc)
	{
		// not implemented
		return false;
	}

  /* enable PLL */
  SCU_PLL->PLLCON0 &= ~(SCU_PLL_PLLCON0_VCOPWD_Msk | SCU_PLL_PLLCON0_PLLPWD_Msk);

  /* select OSC_HP clock as PLL input */
  SCU_PLL->PLLCON2 &= ~SCU_PLL_PLLCON2_PINSEL_Msk;

  /* restart OSC Watchdog */
  SCU_PLL->PLLCON0 &= ~SCU_PLL_PLLCON0_OSCRES_Msk;

  while ((SCU_PLL->PLLSTAT & SCU_PLL_PLLSTAT_OSC_USABLE) != SCU_PLL_PLLSTAT_OSC_USABLE)
  {
    /* wait till OSC_HP output frequency is usable */
  }

  /* Go to bypass the Main PLL */
  SCU_PLL->PLLCON0 |= SCU_PLL_PLLCON0_VCOBYP_Msk;

  /* disconnect Oscillator from PLL */
  SCU_PLL->PLLCON0 |= SCU_PLL_PLLCON0_FINDIS_Msk;

  unsigned k1div = 1;
  unsigned pdiv = 1; // use the crystal input freq
  unsigned freqmul = acpuspeed / abasespeed;

  unsigned min_vco_freq = 260000000;
  unsigned max_vco_freq = 520000000;

  unsigned k2div = 1;
  while (acpuspeed * k2div < min_vco_freq)
  {
  	++k2div;
  }

  unsigned ndiv = freqmul * k2div;  // VCO multiplier

  /* Setup divider settings for main PLL */
  SCU_PLL->PLLCON1 = 0
  	| ((pdiv - 1)      << 24)
		| ((k2div - 1)     << 16)
		| ((ndiv - 1)      <<  8)
		| ((k1div - 1)     <<  0)
	;

  /* Set OSCDISCDIS */
  SCU_PLL->PLLCON0 |= SCU_PLL_PLLCON0_OSCDISCDIS_Msk;

  /* connect Oscillator to PLL */
  SCU_PLL->PLLCON0 &= ~SCU_PLL_PLLCON0_FINDIS_Msk;

  /* restart PLL Lock detection */
  SCU_PLL->PLLCON0 |= SCU_PLL_PLLCON0_RESLD_Msk;

  while ((SCU_PLL->PLLSTAT & SCU_PLL_PLLSTAT_VCOLOCK_Msk) == 0U)
  {
    /* wait for PLL Lock */
  }

  /* Disable bypass- put PLL clock back */
  SCU_PLL->PLLCON0 &= ~SCU_PLL_PLLCON0_VCOBYP_Msk;
  while ((SCU_PLL->PLLSTAT & SCU_PLL_PLLSTAT_VCOBYST_Msk) != 0U)
  {
    /* wait for normal mode */
  }

  /* Before scaling to final frequency we need to setup the clock dividers */
  SCU_CLK->SYSCLKCR = 0
  	| (0 <<  0)  // SYSDIV(8): 0 = do not divide
		| (1 << 16)  // SYSSEL: 1 = select PLL, 0 = internal OSC (OFI)
	;

  SCU_CLK->CPUCLKCR = (0 << 0);  // CPUDIV: 0 = (f_CPU == f_SYS)
  SCU_CLK->PBCLKCR  = (0 << 0);  // PBDIV: 0 = (f_PERIPH == f_CPU)

  SCU_CLK->CCUCLKCR = 0; // no div2 for CCU
  SCU_CLK->WDTCLKCR = 0; // use the OFI for the WDT
  SCU_CLK->USBCLKCR = ((3 - 1) << 0) | (1 << 16); // use PLL with /3
  //SCU_CLK->ECATCLKCR = ???;
  SCU_CLK->EXTCLKCR = 0; // use SYS clock for the external clock

  /* Enable selected clocks */
  SCU_CLK->CLKSET = 0; // no effect ...

  // todo: enable USB PLL

	SystemCoreClock = acpuspeed;  // store it temorarily

  return true;
}

void THwClkCtrl_xmc::SyncToExtOsc(unsigned aextspeed)
{
}

#endif // def MCUSF_4000
