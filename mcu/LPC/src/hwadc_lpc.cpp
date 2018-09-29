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
 *  file:     hwadc_lpc.cpp
 *  brief:    LPC Simple Internal ADC
 *  version:  1.00
 *  date:     2018-09-29
 *  authors:  nvitya
*/

#include "hwadc_lpc.h"
#include "lpc_utils.h"
#include "clockcnt.h"

bool THwAdc_lpc::Init(int adevnum, uint32_t achannel_map)
{
	uint32_t tmp;

	initialized = false;

	devnum = adevnum;
	initialized = false;
	channel_map = (achannel_map & 0xFF);

	regs = nullptr;
	if      (0 == devnum)
	{
		regs = LPC_ADC0;
		lpc_enable_clock(CLK_APB3_ADC0, 1);
	}
#if defined(LPC_ADC1)
	else if (1 == devnum)
	{
		regs = LPC_ADC1;
		lpc_enable_clock(CLK_APB3_ADC1, 1);
	}
#endif
	else
	{
		return false;
	}

	// set ADC clock
	uint32_t adcmaxclock = 4500000;
	uint32_t baseclock = SystemCoreClock;
	uint32_t adcdiv = 1;
	while ((adcdiv < 255) && (baseclock / adcdiv > adcmaxclock))
	{
		adcdiv += 1;
	}
	adc_clock = baseclock / adcdiv;


	regs->CR = 0
		| (channel_map <<  0)  // SEL(8):
		| (adcdiv <<  8)  // CLKDIV(8):
		| (1 <<  16)  // BURST: 1 = repeated, continuous conversion
		| (0 <<  17)  // CLKS(3): 0 = 10 bit mode, 11 clocks
		| (1 <<  21)  // PDN: 1 = A/D converter is operational, 0 = power down
		| (0 <<  24)  // START(3): 0 = burst mode start, 1..7 = trigger select
		| (0 <<  27)  // EDGE: trigger edge
	;

	// calculate the actual conversion rate
	conv_adc_clocks = 11;
	act_conv_rate = adc_clock / conv_adc_clocks;

	initialized = true;
	return true;
}

