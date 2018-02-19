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
 *  file:     hwuart_atsam.cpp
 *  brief:    ATSAM UART (with USART support)
 *  version:  1.00
 *  date:     2018-02-10
 *  authors:  nvitya
*/

#include <stdio.h>
#include <stdarg.h>

#include "hwuart.h"

bool THwUart_atsam::Init(int adevnum)  // devnum: 0 - 4 = UART0..4, 0x100..0x103 = USART0..3
{
	unsigned code;
	unsigned perid;
	unsigned periphclock = SystemCoreClock;

	devnum = adevnum;

	initialized = false;

	regs = nullptr;
	usartregs = nullptr;

	if (false)
	{
		// only for the ifdef logic...
	}
#ifdef UART
	else if (0 == devnum)
	{
		regs = (HW_UART_REGS *)UART;
		perid = ID_UART;
	}
#endif
#ifdef UART0
	else if (0 == devnum)
	{
		regs = (HW_UART_REGS *)UART0;
		perid = ID_UART0;
	}
#endif
#ifdef UART1
	else if (1 == devnum)
	{
		regs = (HW_UART_REGS *)UART1;
		perid = ID_UART1;
	}
#endif
#ifdef UART2
	else if (2 == devnum)
	{
		regs = (HW_UART_REGS *)UART2;
		perid = ID_UART2;
	}
#endif
#ifdef UART3
	else if (3 == devnum)
	{
		regs = (HW_UART_REGS *)UART3;
		perid = ID_UART3;
	}
#endif
#ifdef UART4
	else if (4 == devnum)
	{
		regs = (HW_UART_REGS *)UART4;
		perid = ID_UART4;
	}
#endif
#ifdef USART0
	else if (0x100 == devnum)
	{
		usartregs = (HW_UART_ALT_REGS *)USART0;
		perid = ID_USART0;
	}
#endif
#ifdef USART1
	else if (0x101 == devnum)
	{
		usartregs = (HW_UART_ALT_REGS *)USART1;
		perid = ID_USART1;
	}
#endif
#ifdef USART2
	else if (0x102 == devnum)
	{
		usartregs = (HW_UART_ALT_REGS *)USART2;
		perid = ID_USART2;
	}
#endif
#ifdef USART3
	else if (0x102 == devnum)
	{
		usartregs = (HW_UART_ALT_REGS *)USART3;
		perid = ID_USART3;
	}
#endif

	if (periphclock > 150000000)  periphclock = periphclock / 2;

	if (!regs && !usartregs)
	{
		return false;
	}

	// Enable the peripheral
	if (perid < 32)
	{
		PMC->PMC_PCER0 = (1 << perid);
	}
	else
	{
		PMC->PMC_PCER1 = (1 << (perid-32));
	}

	if (regs)
	{
		// UART

		// disable UART
		regs->UART_CR = (UART_CR_RXDIS | UART_CR_TXDIS);

		regs->UART_CR = UART_CR_RSTSTA | UART_CR_RSTRX | UART_CR_RSTTX; //(UART_CR_REQCLR | UART_CR_RSTSTA);

		// set mode register
		code =
		(   0
			| (1 << 4)   // enable filter
			| (0 << 12)  // periph clock as a source
			| (0 << 14)  // normal mode
		)
		;

		if (parity)
		{
			if (oddparity)  code |= (1 << 9);
			else            code |= (0 << 9);
		}
		else
		{
			code |= (4 << 9); // no parity
		}
		regs->UART_MR = code;

		// setup baud rate
		unsigned baseclock = periphclock >> 4;
		unsigned divider = ((((baseclock << 1) / baudrate) + 1) >> 1);  // round up the result

		regs->UART_BRGR = divider;

		// Enable:
		regs->UART_CR = (UART_CR_RXEN | UART_CR_TXEN);
	}
	else
	{
		// USART

		// disable UART
		usartregs->US_CR = (US_CR_RXDIS | US_CR_TXDIS);

		usartregs->US_CR = US_CR_RSTSTA | US_CR_RSTRX | US_CR_RSTTX;

		// set mode register
		code =
		(   (0 << 0)   // UART, normal mode
			| (0 << 4)   // Peripheral clock selected

			| (3 << 6)   // CHRL: 8 bit characters
			| (0 << 8)   // SYNC: async mode
			| ((halfstopbits - 2) << 12)  // NBSTOP: stop bits
			| (0 << 14)  // CHMODE: normal mode

			| (0 << 16)  // MSBF: LSB first
			| (0 << 17)  // MODE9: 8 bit mode
			| (0 << 18)  // CLKO: no clock drive
			| (0 << 19)  // OVER: 16 x Oversampling
			| (1 << 28)  // FILTER: filter the line
		)
		;
		if (parity)
		{
			if (oddparity)  code |= (1 << 9);
			else            code |= (0 << 9);
		}
		else
		{
			code |= (4 << 9); // no parity
		}
		usartregs->US_MR = code;

		// setup baud rate
		unsigned baseclock = periphclock >> 4;
		unsigned divider = ((((baseclock << 1) / baudrate) + 1) >> 1);  // round up the result

		usartregs->US_BRGR = divider;

		// Enable:
		usartregs->US_CR = (US_CR_RXEN | US_CR_TXEN);
	}

	initialized = true;

	return true;
}

bool THwUart_atsam::TrySendChar(char ach)
{
	if (usartregs)
	{
		if (usartregs->US_CSR & US_CSR_TXEMPTY)
		{
			usartregs->US_THR = ach;
			return true;
		}
		else
		{
			return false;
		}
	}
	else
	{
		if (regs->UART_SR & UART_SR_TXEMPTY)
		{
			regs->UART_THR = ach;
			return true;
		}
		else
		{
			return false;
		}
	}
}

bool THwUart_atsam::TryRecvChar(char * ach)
{
	if (usartregs)
	{
		if (usartregs->US_CSR & US_CSR_RXRDY)
		{
			*ach = usartregs->US_RHR;
			return true;
		}
		else
		{
			return false;
		}
	}
	else
	{
		if (regs->UART_SR & UART_SR_RXRDY)
		{
			*ach = regs->UART_RHR;
			return true;
		}
		else
		{
			return false;
		}
	}
}

#ifdef HW_HAS_PDMA

void THwUart_atsam::PdmaInit(bool istx, THwDmaChannel * admach)
{
	admach->InitPeriphDma(istx, regs, usartregs);
	DmaAssign(istx, admach);
}

#endif

void THwUart_atsam::DmaAssign(bool istx, THwDmaChannel * admach)
{
	if (istx)
	{
		txdma = admach;
		if (usartregs)
		{
			admach->Prepare(istx, (void *)&usartregs->US_THR, 0);
		}
		else
		{
			admach->Prepare(istx, (void *)&regs->UART_THR, 0);
		}
	}
	else
	{
		rxdma = admach;
		if (usartregs)
		{
			admach->Prepare(istx, (void *)&usartregs->US_RHR, 0);
		}
		else
		{
			admach->Prepare(istx, (void *)&regs->UART_RHR, 0);
		}
	}
}

bool THwUart_atsam::DmaStartSend(THwDmaTransfer * axfer)
{
	if (!txdma)
	{
		return false;
	}

	// On Atmel no peripheral preparation is required.

	txdma->StartTransfer(axfer);

	return true;
}

bool THwUart_atsam::DmaStartRecv(THwDmaTransfer * axfer)
{
	if (!rxdma)
	{
		return false;
	}

	// On Atmel no peripheral preparation is required.

	rxdma->StartTransfer(axfer);

	return true;
}

