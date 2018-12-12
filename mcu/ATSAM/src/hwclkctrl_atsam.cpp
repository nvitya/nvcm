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
 *  file:     hwclkctrl_atsam.cpp
 *  brief:    ATSAM MCU Clock / speed setup
 *  version:  1.00
 *  date:     2018-02-10
 *  authors:  nvitya
*/

#include "platform.h"

#include "hwclkctrl.h"

#define SYS_BOARD_OSCOUNT   (CKGR_MOR_MOSCXTST(0x8))

/* Clock Definitions */
#define SYS_UTMIPLL     		(480000000UL)	/* UTMI PLL frequency */

//#define SYS_CKGR_MOR_KEY_VALUE	CKGR_MOR_KEY(0x37) /* Key to unlock MOR register */

#ifndef CKGR_MOR_KEY_PASSWD
  #define CKGR_MOR_KEY_PASSWD CKGR_MOR_KEY(0x37)
#endif

bool THwClkCtrl_atsam::ExtOscReady()
{
  return ((PMC->PMC_SR & PMC_SR_MOSCXTS) != 0);
}

void THwClkCtrl_atsam::StartExtOsc()
{
	PMC->CKGR_MOR = CKGR_MOR_KEY_PASSWD | SYS_BOARD_OSCOUNT | CKGR_MOR_MOSCRCEN | CKGR_MOR_MOSCXTEN;
}

void THwClkCtrl_atsam::PrepareHiSpeed(unsigned acpuspeed)
{
	unsigned fws;

#if defined(MCUSF_3X)
	EFC0->EEFC_FMR = EEFC_FMR_FWS(5);
	EFC1->EEFC_FMR = EEFC_FMR_FWS(5);
#elif defined(MCUSF_4S)

	if      (acpuspeed <=  21000000)
	{
		fws = 0;
	}
	else if (acpuspeed <=  42000000)
	{
		fws = 1;
	}
	else if (acpuspeed <=  63000000)
	{
		fws = 2;
	}
	else if (acpuspeed <=  84000000)
	{
		fws = 3;
	}
	else if (acpuspeed <= 105000000)
	{
		fws = 4;
	}
	else
	{
		fws = 5;
	}

	EFC0->EEFC_FMR = EEFC_FMR_FWS(fws) | EEFC_FMR_CLOE;
  #if defined(EFC1)
	  EFC1->EEFC_FMR = EEFC_FMR_FWS(fws);
  #endif

	#ifdef REG_CMCC_CTRL
	  REG_CMCC_CTRL = 1;
  #endif

#elif defined(MCUSF_E70)

	unsigned base = EEFC_FMR_CLOE;
	//unsigned base = EEFC_FMR_SCOD;

	if      (acpuspeed <=  25000000)
	{
		fws = 0;
	}
	else if (acpuspeed <=  51000000)
	{
		fws = 1;
	}
	else if (acpuspeed <=  76000000)
	{
		fws = 2;
	}
	else if (acpuspeed <= 102000000)
	{
		fws = 3;
	}
	else if (acpuspeed <= 128000000)
	{
		fws = 4;
	}
	else
	{
		fws = 5;
	}

	EFC->EEFC_FMR = EEFC_FMR_CLOE | EEFC_FMR_FWS(fws);

#else
	#error "Unhandled MCU"
#endif
}

bool THwClkCtrl_atsam::SetupPlls(bool aextosc, unsigned abasespeed, unsigned acpuspeed)
{
	/* Switch to 3-20MHz Xtal oscillator */
	PMC->CKGR_MOR = CKGR_MOR_KEY_PASSWD | SYS_BOARD_OSCOUNT |
	                           CKGR_MOR_MOSCRCEN | CKGR_MOR_MOSCXTEN | CKGR_MOR_MOSCSEL;

	while (!(PMC->PMC_SR & PMC_SR_MOSCSELS))
	{
	}

	// select main clock as master source
 	PMC->PMC_MCKR = (PMC->PMC_MCKR & ~(uint32_t)PMC_MCKR_CSS_Msk) | PMC_MCKR_CSS_MAIN_CLK;
 	while (!(PMC->PMC_SR & PMC_SR_MCKRDY))
	{
	}

 	// Set up the PLLA to use as CPU clock (HCLK)

	unsigned freqmul = (acpuspeed / abasespeed) - 1;

	/* Initialize PLLA */
	PMC->CKGR_PLLAR =	CKGR_PLLAR_ONE | CKGR_PLLAR_PLLACOUNT(0x3f)	| CKGR_PLLAR_MULA(freqmul) | CKGR_PLLAR_DIVA(1);
	while (!(PMC->PMC_SR & PMC_SR_LOCKA))
	{
	}

	unsigned ctrlbase = PMC_MCKR_PRES_CLK_1 | PMC_MCKR_CSS_PLLA_CLK;
#if MAX_CLOCK_SPEED > 150000000
	if (acpuspeed > 150000000)
	{
		ctrlbase |= PMC_MCKR_MDIV_PCK_DIV2;

		// DO NOT modify MDIV and CSS feild at the same access
		PMC->PMC_MCKR = (PMC->PMC_MCKR & ~PMC_MCKR_MDIV_Msk) | PMC_MCKR_MDIV_PCK_DIV2;
	}
#endif

	// Activate prescaler / divider with main clock
	PMC->PMC_MCKR = (ctrlbase & ~PMC_MCKR_CSS_Msk) | PMC_MCKR_CSS_MAIN_CLK;
	while (!(PMC->PMC_SR & PMC_SR_MCKRDY))
	{
	}

	// Switch to PLLA
	PMC->PMC_MCKR = ctrlbase | PMC_MCKR_CSS_PLLA_CLK;
	while (!(PMC->PMC_SR & PMC_SR_MCKRDY))
	{
	}

#if defined(REG_PMC_USB)
	// Prepare the PLLB for USB, 48 MHz

	unsigned usbdiv = 1;
	freqmul = usbdiv * 48000000 / abasespeed;
	// the minimal multiplier is 7, so we use a /2 divider here
	while (freqmul < 7)
	{
		++usbdiv;
		freqmul = usbdiv * 48000000 / abasespeed;
	}

	// setup PLLB for the USB clock
	PMC->CKGR_PLLBR =	CKGR_PLLBR_PLLBCOUNT(0x3f)	| CKGR_PLLBR_MULB(freqmul) | CKGR_PLLBR_DIVB(usbdiv);
	while (!(PMC->PMC_SR & PMC_SR_LOCKB))
	{
	}

	PMC->PMC_USB = 0
		| (1 << 0)   // select PLLB for the USB
		| (0 << 8)   // no more dividing
	;
#endif

  return true;
}

