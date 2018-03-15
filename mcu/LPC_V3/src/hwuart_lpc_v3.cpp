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
 *  file:     hwuart_lpc_v3.cpp
 *  brief:    LPC_V3 UART
 *  version:  1.00
 *  date:     2018-02-10
 *  authors:  nvitya
*/


#include <stdio.h>
#include <stdarg.h>

#include "hwuart.h"

bool THwUart_lpc_v3::Init(int adevnum)
{
	unsigned code;

	devnum = adevnum;
	initialized = false;
	regs = nullptr;

	if ((devnum < 0) or (devnum > 9))
	{
		return false;
	}

	unsigned fcnum = devnum;
	unsigned fcbaseaddr[] = FLEXCOMM_BASE_ADDRS;

	regs = (HW_UART_REGS *)(fcbaseaddr[fcnum]);

	// turn on the flexcomm hw:
	if (fcnum <= 7)
	{
		SYSCON->AHBCLKCTRLSET[1] = (1 << (11 + fcnum));
	}
	else
	{
		SYSCON->AHBCLKCTRLSET[2] = (1 << (14 + (fcnum - 8)));
	}

	SYSCON->FCLKSEL[fcnum] = 1; // select the 48 MHz free running oscillator as clock source
	unsigned basefreq = 48000000;

	// select uart mode
	FLEXCOMM_Type * flexcomm = (FLEXCOMM_Type *)regs;
	flexcomm->PSELID = 1; // select USART function

	// setup UART

	// disable UART
	regs->CFG = 0;

	// search the best settings for the desirede baud rate

  uint32_t best_diff = (uint32_t)-1, best_osrval = 0xf, best_brgval = (uint32_t)-1;
  uint32_t osrval, brgval, diff;
  uint32_t rbaudrate;

  for (osrval = best_osrval; osrval >= 8; osrval--)
  {
      brgval = (basefreq / ((osrval + 1) * baudrate)) - 1;
      if (brgval > 0xFFFF)
      {
          continue;
      }
      rbaudrate = basefreq / ((osrval + 1) * (brgval + 1));
      diff = baudrate < rbaudrate ? rbaudrate - baudrate : baudrate - rbaudrate;
      if (diff < best_diff)
      {
          best_diff = diff;
          best_osrval = osrval;
          best_brgval = brgval;
      }
  }

  /* value over range */
  if (best_brgval > 0xFFFF)
  {
      return false;
  }

  regs->OSR = best_osrval;
  regs->BRG = best_brgval;

  regs->CTL = 0;

  regs->INTENCLR = 0x1F868; // clear all interrupts

  code = ((1 << 0) | (1 << 2));  // enable + 8 bit data length

  if (parity)
  {
  	if (oddparity)  code |=  (3 << 4);
  	else            code |=  (2 << 4); // even parity
  }

  if (halfstopbits > 2)  code |= (1 << 6);   // 2 stop bit

  regs->CFG = code;

  regs->FIFOCFG = (3 | (3 << 16)); // enable TX + RX FIFO, empty FIFOs

	initialized = true;

	return true;
}

bool THwUart_lpc_v3::TrySendChar(char ach)
{
	if ((regs->FIFOSTAT & USART_FIFOSTAT_TXNOTFULL_MASK) == 0)
	{
		return false;
	}

	regs->FIFOWR = ach;

	return true;
}

bool THwUart_lpc_v3::TryRecvChar(char * ach)
{
	if (regs->FIFOSTAT & USART_FIFOSTAT_RXNOTEMPTY_MASK)
	{
		unsigned d = regs->FIFORD;
		*ach = (d & 0xFF);
		return true;
	}

	return false;
}
