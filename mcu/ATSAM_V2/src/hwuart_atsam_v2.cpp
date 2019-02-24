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
 *  file:     hwuart_atsam_v2.cpp
 *  brief:    ATSAM_V2 UART
 *  version:  1.00
 *  date:     2018-02-10
 *  authors:  nvitya
*/

#include <stdio.h>
#include <stdarg.h>

#include "hwuart.h"
#include "atsam_v2_utils.h"


bool THwUart_atsam_v2::Init(int adevnum)  // devnum: 0 - 7 = SERCOM ID
{
	unsigned code;
	unsigned perid;

	devnum = adevnum;
	initialized = false;
	regs = nullptr;

	if (!atsam2_sercom_enable(devnum, 0))
	{
		return false;
	}

	regs = (HW_UART_REGS *)sercom_inst_list[devnum];

	regs->CTRLA.bit.ENABLE = 0; // disable
	//regs->CTRLA.bit.SWRST = 1; // reset
	//while (regs->SYNCBUSY.bit.SWRST) { } // wait for reset

	//regs->CTRLA.bit.SWRST = 0; // reset

	// baud rate calculation
	// fbaud = fref / oversampling * (1 - baudvalue / 65536)
	// baudvalue = 65536 * (1 - oversampling * fbaud / fref)

	unsigned oversampling = 16;
	unsigned brdiv = SystemCoreClock / ((oversampling / 8) * baudrate);  // the lower 3 bits are the fractional part
	unsigned baudvalue = (((brdiv >> 3) & 0x1FFF) | ((brdiv & 7) << 13));

	regs->BAUD.reg = baudvalue;

	// CTRLB
	code = 0
		| (1 << 17)  // RXEN
		| (1 << 16)  // TXEN
		| ((((halfstopbits-2) / 2) & 1) << 6) // SBMODE
		| (0 <<  0)  // CHSIZE(3): 0 = 8 bit characters
	;

	if (parity and oddparity)
	{
		code |= (1 << 13);
	}

	// 0x30000

	while (regs->SYNCBUSY.bit.CTRLB) { } // wait for sync
	regs->CTRLB.reg = code;
	while (regs->SYNCBUSY.bit.CTRLB) { } // wait for sync

	// CTRLC
  #ifdef REG_SERCOM0_USART_CTRLC
	  regs->CTRLC.reg = 0x700002; //((7 << 20) | (2 << 0));
  #endif

	// CTRLA
	code = 0
		| (1 << 30)  // DORD: 1 = LSB first
		| (0 << 28)  // CMODE: async mode
		| (0 << 24)  // FORM(4): frame format, 0 = USART without parity
		| (0 << 22)  // SAMPA(2): sample adjustment
		| (1 << 20)  // RXPO(2): RX pad select, 1 = PAD[1] for RX
		| (0 << 16)  // TXPO(2): TX pad select, 2 = PAD[0] for TX, RTS=PAD[2], CTS=PAD[3]
		| (1 << 13)  // SAMPR(3): Sample rate, 16x oversampling, fractional b.r.g.
		| (1 <<  2)  // MODE(3): Mode, 1 = USART with internal clock
		| (0 <<  1)  // ENABLE
	;

	if (parity)
	{
		code |= (1 << 24);
	}

	regs->CTRLA.reg = code;

	regs->RXPL.reg = 0;
	regs->DBGCTRL.reg = 0;

	while (regs->SYNCBUSY.bit.ENABLE) { } // wait for enable
	regs->CTRLA.reg = (code | (1 << 1)); // enable it
	while (regs->SYNCBUSY.bit.ENABLE) { } // wait for enable

	initialized = true;

	return true;
}

bool THwUart_atsam_v2::TrySendChar(char ach)
{
	if (regs->INTFLAG.bit.DRE)
	{
		regs->DATA.reg = ach;
		return true;
	}
	else
	{
		return false;
	}
}

bool THwUart_atsam_v2::TryRecvChar(char * ach)
{
	if (regs->INTFLAG.bit.RXC)
	{
		*ach = regs->DATA.reg;
		return true;
	}
	else
	{
		return false;
	}
}

void THwUart_atsam_v2::DmaAssign(bool istx, THwDmaChannel * admach)
{
	if (istx)
	{
		txdma = admach;
	}
	else
	{
		rxdma = admach;
	}

	admach->Prepare(istx, (void *)&regs->DATA.reg, 0);
}


bool THwUart_atsam_v2::DmaStartSend(THwDmaTransfer * axfer)
{
	if (!txdma)
	{
		return false;
	}

	txdma->StartTransfer(axfer);

	return true;
}

bool THwUart_atsam_v2::DmaStartRecv(THwDmaTransfer * axfer)
{
	if (!rxdma)
	{
		return false;
	}

	rxdma->StartTransfer(axfer);

	return true;
}
