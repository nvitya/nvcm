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
 *  file:     hwuart_kinetis.cpp
 *  brief:    KINETIS UART
 *  version:  1.00
 *  date:     2018-02-10
 *  authors:  nvitya
*/

#if 0

#include <stdio.h>
#include <stdarg.h>

#include "hwuart.h"

bool THwUart_kinetis::Init(int adevnum)
{
	unsigned x;

	devnum = adevnum;
	initialized = false;

	regs = nullptr;
	if      (0 == devnum)
	{
		regs = (HW_UART_REGS *)LPUART0_BASE;
		// turn on the clock

		SIM->SCGC5 |= (1 << 20); // enable system clock for the module
		x = (SIM->SOPT2 & ~(3 << 26));
		x |= (1 << 26); // select HIRC48M as clock
		SIM->SOPT2 = x;
	}

	if (!regs)
	{
		return false;
	}


	// disable transmit and receive
	regs->CTRL &= (3 << 18);

	unsigned baseclock = 48000000 >> 4;
	unsigned sbr = baseclock / baudrate;

	// BAUD RATE register
	x = 0
	 | (0x0F << 24)  // OSR(5): oversampling, 0x0F = 16
	 | (0 << 13)     // SBNS: 0 = 1 stop bit
	 | (sbr << 0)
	;
	if (halfstopbits == 4)
	{
		x |= (1 << 13);
	}
	regs->BAUD = x;


	// STAT register
	x = 0
	 | (0 << 29)  // MSBF: 0 = LSB first
	;
	regs->STAT = x;

	// CTRL register
	x = 0
	 | (1 << 19)  // TE: 1 = transmitter enable
	 | (1 << 18)  // RE: 1 = receiver enable
	 | (0 <<  4)  // M: 8 bit characters
	;
	if (parity)
	{
		x |= (1 << 1);
		if (oddparity)	x |= (1 << 0);
	}
	regs->CTRL = x;  // enables the UART as well

	initialized = true;

	return true;
}

bool THwUart_kinetis::TrySendChar(char ach)
{
	if ((regs->STAT & LPUART_STAT_TDRE_MASK) == 0)
	{
		return false;
	}

	regs->DATA = ach;

	return true;
}

bool THwUart_kinetis::SendFinished()
{
	if (regs->STAT & LPUART_STAT_TC_MASK)
	{
		return true;
	}
	else
	{
		return false;
	}
}

#endif
