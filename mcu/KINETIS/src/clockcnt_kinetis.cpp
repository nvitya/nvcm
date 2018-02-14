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
 *  file:     clockcnt_kinetis.cpp
 *  brief:    KINETIS Clock Counter for M0 MCUs
 *  version:  1.00
 *  date:     2018-02-10
 *  authors:  nvitya
*/

#include "platform.h"

#if __CORTEX_M < 3

// clock timer initialization for Cortex-M0 processors

void clockcnt_init()
{
	unsigned x;

  // configure clock source
	SIM->SCGC6 |= (1 << 25); // enable system clock for the module

	x = (SIM->SOPT2 & ~(3 << 24));
	x |= (1 << 24); // select HIRC48M as clock, TODO: support other speeds
	SIM->SOPT2 = x;

	// configure the timer
	TPM1->CONF = 0;
	TPM1->MOD = 0xFFFF;
	TPM1->SC = 0
		| (0 <<  5)  // CPWMS: 0 = up counter mode
		| (1 <<  3)  // CMOD(2): increment by every clock
		| (0 <<  0)  // PS(3): prescale factor, 0 = divide by 1
	;

}

#endif

