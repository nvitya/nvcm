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
 *  file:     clockcnt_xmc.cpp
 *  brief:    XMC1000 Clock Counter for M0 MCUs
 *  version:  1.00
 *  date:     2018-02-10
 *  authors:  nvitya
*/

#include "platform.h"
#include "xmc_utils.h"

#if __CORTEX_M < 3

// clock timer initialization for Cortex-M0 processors

void clockcnt_init()
{
	xmc_enable_periph_clock(0, 1 << 2); // enable CCU40

	CCU40->GIDLC = (1 << 8); // enable prescaler
	CCU40->GCTRL = 0; // global control

	CCU40_CC43->TC = 0;   // control register, use the defaults (0)
	CCU40_CC43->PSC = 1;  // prescaler configuration: divide by two to count CPU clocks (PCLK = 2 x MCLK)
	CCU40_CC43->PRS = 0xFFFF;  // period shadow register

	CCU40->GCSS = ((1 << 19) | (7 << 12)); // actualize shadow values

	CCU40->GIDLC = (1 << 3); // idle mode clear for 03
	CCU40_CC43->TCSET = 1;  // run the timer
}

#endif

