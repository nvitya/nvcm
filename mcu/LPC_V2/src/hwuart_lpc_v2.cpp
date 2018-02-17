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
 *  file:     hwuart_lpc_v2.cpp
 *  brief:    LPC_V2 UART
 *  version:  1.00
 *  date:     2018-02-10
 *  authors:  nvitya
*/

#include <stdio.h>
#include <stdarg.h>

#include "hwuart.h"

bool THwUart_lpc_v2::Init(int adevnum)
{
	unsigned code;

	initialized = false;
	regs = nullptr;

	if (0 == adevnum)
	{
		regs = LPC_USART0;
	}
	else if (1 == adevnum)
	{
		regs = LPC_USART1;
	}
	else if (2 == adevnum)
	{
		regs = LPC_USART2;
	}
	else
	{
		return false;
	}

	devnum = adevnum;
	LPC_SYSCON->SYSAHBCLKCTRL |= (1 << (14 + devnum));
	LPC_SYSCON->PRESETCTRL |= (1 << (3 + devnum));

	LPC_SYSCON->UARTCLKDIV = 1; // enabled but do not divide the UART clock
	LPC_SYSCON->UARTFRGDIV = 0xFF; // the only allowed value, resuting / 256

	// usart_base_clock = pclock / uartdiv / ((256 + frgmul)/256)

	LPC_SYSCON->UARTFRGMULT = 4;  // frgq = 260 / 256

	// uart_clock = usart_base_clock / 16 / baudratereg

	unsigned basefreq = LPC_SYSCON->SYSAHBCLKDIV * ((SystemCoreClock * 16) / 260);

  unsigned brdiv = basefreq / baudrate;
  if (brdiv < 1)  brdiv = 1;
  regs->BRG = (brdiv - 1);

  regs->INTENCLR = 0x1F96D; // clear all interrupts

  regs->CTRL = 0;

  code = ((1 << 0) | (1 << 2));  // enable + 8 bit data length

  if (parity)
  {
  	if (oddparity)  code |=  (3 << 4);
  	else            code |=  (2 << 4); // even parity
  }

  if (halfstopbits > 2)  code |= (1 << 6);   // 2 stop bit

  regs->CFG = code;

	initialized = true;

	return true;
}

bool THwUart_lpc_v2::TrySendChar(char ach)
{
	if ((regs->STAT & (1 << 2)) == 0)
	{
		return false;
	}

	regs->TXDATA = ach;

	return true;
}

