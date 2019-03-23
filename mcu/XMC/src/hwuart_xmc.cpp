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
 *  file:     hwuart_xmc.cpp
 *  brief:    XMC UART
 *  version:  1.00
 *  date:     2018-02-10
 *  authors:  nvitya
*/

#include <stdio.h>
#include <stdarg.h>

#include "hwuart.h"
#include "xmc_utils.h"

bool THwUart_xmc::Init(int ausicnum, int achnum, int ainputpin)
{
	unsigned code;
	unsigned rv;
	unsigned clockdiv = 1;

	usicnum = ausicnum;
	chnum = achnum;
	inputpin = ainputpin;
	devnum = achnum;

	initialized = false;

	regs = xmc_usic_ch_init(usicnum, chnum);
	if (!regs)
	{
		return false;
	}

  // Configure baud rate

	unsigned oversampling = 16;

  uint32_t clock_divider;
  uint32_t clock_divider_min;

  uint32_t pdiv;
  uint32_t pdiv_int;
  uint32_t pdiv_int_min;

  uint32_t pdiv_frac;
  uint32_t pdiv_frac_min;

  uint32_t peripheral_clock = SystemCoreClock / 100;
  uint32_t rate = baudrate / 100;

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

  /* Configure frame format
   * Configure the number of stop bits
   * Pulse length is set to 0 to have standard UART signaling,
   * i.e. the 0 level is signaled during the complete bit time
   * Sampling point set equal to the half of the oversampling period
   * Enable Sample Majority Decision */
  regs->PCR_ASCMode = 0
  	| (((halfstopbits-1) >> 1) << 1)    // STPB: stop bits
  	| (((oversampling >> 1) + 1) << 8)  // SP: sample point = middle
  	| (1 << 0)  // SMD: 1 = three samples are taken
  ;

  /* Set passive data level, high
     Set word length. Data bits - 1
     If frame length is > 0, frame_lemgth-1; else, FLE = WLE (Data bits - 1)
     Transmission Mode: The shift control signal is considered active if it
     is at 1-level. This is the setting to be programmed to allow data transfers */
  regs->SCTR = 0
  	| (0 << 0)  // SDIR: 0 = LSB first
  	| (1 << 1)  // PDL: passive data level
  	| (0 << 2)  // DSM(2): data shift mode
  	| (1 << 8)  // TRM(2): transmission mode
  	| ((databits - 1) << 16) // FLE(6): frame length
  	| ((databits - 1) << 24) // WLE(4): word length
  ;

  /* Clear protocol status */
  regs->PSCR = 0xFFFFFFFF;

  /*Set input source path*/
  regs->DX0CR = 0
  	| (inputpin << 0)  // DSEL: DXnB selected
  ;

  // Configure FIFO-s

  unsigned fifooffs = chnum * 32;

  // Configure transmit FIFO
  //XMC_USIC_CH_TXFIFO_Configure(regs, 32, XMC_USIC_CH_FIFO_SIZE_32WORDS, 1);

  /* Disable FIFO */
  regs->TBCTR = 0;

  /* LOF = 0, A standard transmit buffer event occurs when the filling level equals the limit value and gets
   * lower due to transmission of a data word
   * STBTEN = 0, the trigger of the standard transmit buffer event is based on the transition of the fill level
   *  from equal to below the limit, not the fact being below
   */
  regs->TBCTR = 0
    | ((fifooffs + 16) << 0)  // DPTR(6): data pointer
    | (1  << 8)  // LIMIT(6) for interrupt generation
    | (4  << 24) // SIZE(3): size code, 4 = 16 entries
  ;

  // Configure receive FIFO
  //XMC_USIC_CH_RXFIFO_Configure(regs,  0, XMC_USIC_CH_FIFO_SIZE_32WORDS, 0);

  /* Disable FIFO */
  regs->RBCTR = 0;

  /* LOF = 1, A standard receive buffer event occurs when the filling level equals the limit value and gets bigger
   *  due to the reception of a new data word
   */
  regs->RBCTR = 0
      | (fifooffs << 0)  // DPTR(6): data pointer
      | (0 << 8)  // LIMIT(6) for interrupt generation
      | (4 << 24) // SIZE(3): size code, 4 = 16 entries
      | (1 << 28) // LOF: event on limit overflow
  ;

  /* Enable transfer buffer */
  regs->TCSR = 0
  	| (1 << 10) // TDEN
  	| (1 << 8)  // TDSSM
  ;

  // Channel Control Register

  rv = 0
  	| (2 << 0)  // set UART mode
  ;

  if (parity)
  {
  	if (oddparity)
  	{
  		rv |= (3 < 8);
  	}
  	else
  	{
  		rv |= (2 < 8);
  	}
  }
  regs->CCR = rv;  // this enables the UART

	initialized = true;

	return true;
}

bool THwUart_xmc::TrySendChar(char ach)
{
	if (regs->TRBSR & (1 << 12))  // is the Transmit FIFO full?
	{
		return false;
	}

	regs->IN[0] = ach; // put the character into the transmit fifo

	return true;
}

bool THwUart_xmc::TryRecvChar(char * ach)
{
	if (regs->TRBSR & (1 << 3))  // is the Receive Buffer Empty?
	{
		return false;
	}

	*ach = (uint16_t)regs->OUTR;

	return true;
}

bool THwUart_xmc::SendFinished()
{
	if (regs->TRBSR & (1 << 11))  // Transmit buffer empty?
	{
		return true;
	}
	else
	{
		return false;
	}
}

