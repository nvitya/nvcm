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
 *  file:     xmc_utils.cpp
 *  brief:    some XMC helper functions
 *  version:  1.00
 *  date:     2018-02-10
 *  authors:  nvitya
*/

#include "platform.h"

#define SCU_GCU_PASSWD_PROT_ENABLE  (195UL) /**< Password for enabling protection */
#define SCU_GCU_PASSWD_PROT_DISABLE (192UL) /**< Password for disabling protection */

/* API to make protected bitfields available for modification */
void XMC_SCU_UnlockProtectedBits(void)
{
#if defined(SCU_GENERAL_PASSWD_PROTS_Msk)
  SCU_GENERAL->PASSWD = SCU_GCU_PASSWD_PROT_DISABLE;

  while(((SCU_GENERAL->PASSWD)&SCU_GENERAL_PASSWD_PROTS_Msk))
  {
    /* Loop until the lock is removed */
  }
#endif
}

/* API to lock protected bitfields from being modified */
void XMC_SCU_LockProtectedBits(void)
{
#if defined(SCU_GENERAL_PASSWD_PROTS_Msk)
  SCU_GENERAL->PASSWD = SCU_GCU_PASSWD_PROT_ENABLE;
#endif
}

void xmc_enable_periph_clock(uint8_t cgnum, unsigned permask)
{
	XMC_SCU_UnlockProtectedBits();
	if (cgnum == 0)
	{
		SCU_CLK->CGATCLR0 |= permask;
#if defined(MCUSF_4000)
		SCU_RESET->PRCLR0 = permask;
#endif
	}
#if defined(MCUSF_4000)
	else if (cgnum == 1)
	{
		SCU_CLK->CGATCLR1 |= permask;
#if defined(MCUSF_4000)
		SCU_RESET->PRCLR1 = permask;
#endif
	}
#endif
	XMC_SCU_LockProtectedBits();
}

USIC_CH_TypeDef * xmc_usic_ch_init(uint8_t usicnum, uint8_t chnum)
{
	USIC_CH_TypeDef * regs = nullptr;

	regs = nullptr;
	if (0 == usicnum)
	{
		if (chnum)
		{
			regs = USIC0_CH1;
		}
		else
		{
			regs = USIC0_CH0;
		}

		xmc_enable_periph_clock(0, SCU_CLK_CGATSTAT0_USIC0_Msk);
	}
#if defined(USIC1)
	else if (1 == usicnum)
	{
		if (chnum)
		{
			regs = USIC1_CH1;
		}
		else
		{
			regs = USIC1_CH0;
		}
		#if defined(SCU_CLK_CGATSTAT1_USIC1_Msk)
			xmc_enable_periph_clock(1, SCU_CLK_CGATSTAT1_USIC1_Msk);
		#else
			xmc_enable_periph_clock(0, SCU_CLK_CGATSTAT0_USIC1_Msk);
    #endif
	}
#endif
#if defined(USIC2)
	else if (2 == usicnum)
	{
		if (chnum)
		{
			regs = USIC2_CH1;
		}
		else
		{
			regs = USIC2_CH0
		}
		xmc_enable_periph_clock(0, SCU_CLK_CGATSTAT0_USIC2_Msk);
	}
#endif
#if defined(USIC3)
	else if (2 == usicnum)
	{
		if (chnum)
		{
			regs = USIC3_CH1;
		}
		else
		{
			regs = USIC3_CH0
		}
		xmc_enable_periph_clock(0, SCU_CLK_CGATSTAT0_USIC3_Msk);
	}
#endif

	if (regs)
	{
		regs->KSCFG = ((1 << 0) | (1 << 1)); // MODEN + BPMODEN
		while ((regs->KSCFG & 1) == 0U)
		{
			// Wait till the channel is enabled
		}
	}

	return regs;
}

void xmc_usic_set_baudrate(USIC_CH_TypeDef * regs, uint32_t speed, uint32_t oversampling)
{
  uint32_t clock_divider;
  uint32_t clock_divider_min;

  uint32_t pdiv;
  uint32_t pdiv_int;
  uint32_t pdiv_int_min;

  uint32_t pdiv_frac;
  uint32_t pdiv_frac_min;

  uint32_t peripheral_clock = SystemCoreClock / 100;
  uint32_t rate = speed / 100;

	clock_divider_min = 1;
	pdiv_int_min = 1;
	pdiv_frac_min = 0x3ff;

	for (clock_divider = 1023; clock_divider > 0; --clock_divider)
	{
		pdiv = ((peripheral_clock * clock_divider) / (rate * oversampling));
		pdiv_int = pdiv >> 10;
		pdiv_frac = pdiv & 0x3ff;

		if ((pdiv_int < 1024) && (pdiv_frac < pdiv_frac_min))
		{
			pdiv_frac_min = pdiv_frac;
			pdiv_int_min = pdiv_int;
			clock_divider_min = clock_divider;
		}
	}

  regs->FDR = ((1 << 15) | (clock_divider_min << 0));

  regs->BRG = 0
  	| (0 << 0)                    // CLKSEL(2): 0 = fractional divider
    | ((oversampling - 1) << 10)  // DCTQ(5)
    | ((pdiv_int_min - 1) << 16); // PDIV(10)
  ;
}
