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
 *  file:     hwspi_atsam.cpp
 *  brief:    ATSAM SPI (master only)
 *  version:  1.00
 *  date:     2018-02-10
 *  authors:  nvitya
*/

#include "platform.h"
#include "hwspi.h"

bool THwSpi_atsam::Init(int adevnum)  // 0..1 = SPI0..1, 0x100 .. 0x103 = USART0..3
{
	devnum = adevnum;
	unsigned code;
	unsigned perid;
	unsigned periphclock = SystemCoreClock;

	if (periphclock > 150000000)  periphclock = periphclock / 2;

	regs = nullptr;
	usartregs = nullptr;

	// because of the peripheral IDs we need full multiple definitions
	// advanced SPIs
	if (false)
	{

	}

#ifdef SPI
	else if (0 == devnum)
	{
		regs = (HW_SPI_REGS *)SPI;
		perid = ID_SPI;
	}
#endif
#ifdef SPI0
	else if (0 == devnum)
	{
		regs = (HW_SPI_REGS *)SPI0;
		perid = ID_SPI0;
	}
#endif
#ifdef SPI1
	else if (1 == devnum)
	{
		regs = (HW_SPI_REGS *)SPI1;
		perid = ID_SPI1;
	}
#endif

	// USARTs:
	else if (0x100 == devnum)
	{
		usartregs = USART0;
		perid = ID_USART0;
	}
	else if (0x101 == devnum)
	{
		usartregs = USART1;
		perid = ID_USART1;
	}
#ifdef USART2
	else if (0x102 == devnum)
	{
		usartregs = USART2;
		perid = ID_USART2;
	}
#endif
#ifdef USART3
	else if (0x103 == devnum)
	{
		usartregs = USART3;
		perid = ID_USART3;
	}
#endif
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
		// Advanced SPI

		regs->SPI_CR = SPI_CR_SPIDIS;
		regs->SPI_CR = SPI_CR_SWRST;

		regs->SPI_MR = 0
			| (1 << 0)   // Master Mode
			| (0 << 1)   // Fixed Chip Select
			| (0 << 2)   // do not use CS decoder
			| (1 << 4)   // 1 = diasble mode fault detection
			| (0 << 5)   // do not wait for rx empty
			| (0 << 7)   // loopback disabled
			| (0 << 16)  // PCS0 is used
			| (100 << 24)  // Delay between chip selects
		;

		unsigned divider = ((((periphclock << 1) / speed) + 1) >> 1);  // round up the result
		if (divider < 1)  divider = 1;

		code = 0
			| (0 << 0)   // CPOL: Clock Polarity
			| (0 << 1)   // NCPHA: Data/Sampling edge
			| (0 << 2)   // CSNAAT: no chip selects between the transfers
			| (0 << 3)   // CSAAT: do not pull back CS when no data to transfer
			| ((databits - 8) << 4)   // BITS: 8 bits
			| (divider << 8)  // SCBR: baud rate
			| (100 << 16)  // DLYBS: CS to CLK delay
			| (0 << 24)  // DLYBCT: no delay between the transfers
 		;

		if (idleclk_high)      code |= (1 << 0);
		if (!datasample_late)  code |= (1 << 1);

		regs->SPI_CSR[0] = code;

		regs->SPI_CR = SPI_CR_SPIEN;  // SPI Enable
	}
	else
	{
		// USART

		// disable USART
		usartregs->US_CR = (US_CR_RXDIS | US_CR_TXDIS);
		// reset
		usartregs->US_CR = US_CR_RSTSTA | US_CR_RSTRX | US_CR_RSTTX;

		// set mode register
		code =
		(   (0xE << 0) // SPI master mode
			| (0 << 4)   // Peripheral clock MCK selected

			| (3 << 6)   // CHRL: 8 bit characters
			| (0 << 8)   // CPHA: late sample
			| (0 << 9)   // PAR: no parity
			| (0 << 14)  // CHMODE: normal mode

			| (0 << 16)  // CPOL: idle clock is low
			| (1 << 18)  // CLKO: drive the clock
			| (1 << 20)  // INACK:
			| (0 << 22)  // VARSYNC:
			| (0 << 28)  // FILTER: do not filter the line
		)
		;

		if (!datasample_late)  code |= (1 << 8);
		if (idleclk_high)      code |= (1 << 16);

		usartregs->US_MR = code;

		// setup baud rate
		unsigned divider = ((((periphclock << 1) / speed) + 1) >> 1);  // round up the result
		if (divider < 1)  divider = 1;

		usartregs->US_BRGR = divider;

		usartregs->US_TTGR = 0; // disable timeguard

		// Enable:
		usartregs->US_CR = (US_CR_RXEN | US_CR_TXEN);
	}

	return true;
}


bool THwSpi_atsam::TrySendData(unsigned short adata)
{
	if (regs)
	{
		if (regs->SPI_SR & SPI_SR_TDRE)
		{
			regs->SPI_TDR = adata;
			return true;
		}
		else
		{
			return false;
		}
	}
	else
	{
		if (usartregs->US_CSR & US_CSR_TXRDY)
		//if (usart->US_CSR & US_CSR_TXEMPTY)
		{
			usartregs->US_THR = adata;
			return true;
		}
		else
		{
			return false;
		}
	}
}

bool THwSpi_atsam::TryRecvData(unsigned short * dstptr)
{
	if (regs)
	{
		if (regs->SPI_SR & SPI_SR_RDRF)
		{
			*dstptr = (regs->SPI_RDR & 0xFFFF);
			return true;
		}
		else
		{
			return false;
		}
	}
	else
	{
		if (usartregs->US_CSR & US_CSR_RXRDY)
		{
			*dstptr = usartregs->US_RHR;
			return true;
		}
		else
		{
			return false;
		}
	}
}

void THwSpi_atsam::DmaAssign(bool istx, THwDmaChannel * admach)
{
	if (istx)
	{
		txdma = admach;
		if (regs)
		{
		  admach->Prepare(istx, (void *)&regs->SPI_TDR, 0);
		}
		else
		{
		  admach->Prepare(istx, (void *)&usartregs->US_THR, 0);
		}
	}
	else
	{
		rxdma = admach;
		if (regs)
		{
		  admach->Prepare(istx, (void *)&regs->SPI_RDR, 0);
		}
		else
		{
		  admach->Prepare(istx, (void *)&usartregs->US_RHR, 0);
		}
	}
}

bool THwSpi_atsam::DmaStartSend(THwDmaTransfer * axfer)
{
	if (!txdma)
	{
		return false;
	}

	txdma->StartTransfer(axfer);

	return true;
}

bool THwSpi_atsam::DmaStartRecv(THwDmaTransfer * axfer)
{
	if (!rxdma)
	{
		return false;
	}

	rxdma->StartTransfer(axfer);

	return true;
}

bool THwSpi_atsam::SendFinished()
{
	if (regs)
	{
		if (regs->SPI_SR & SPI_SR_TXEMPTY)
		{
			return true;
		}
		else
		{
			return false;
		}
	}
	else
	{
		if (usartregs->US_CSR & US_CSR_TXEMPTY)
		{
			return true;
		}
		else
		{
			return false;
		}
	}
}

bool THwSpi_atsam::DmaSendCompleted()
{
	if (txdma && txdma->Enabled())
	{
		// Send DMA is still active
		return false;
	}

	return SendFinished();
}

bool THwSpi_atsam::DmaRecvCompleted()
{
	if (rxdma && rxdma->Enabled())
	{
		// Recv DMA is still active
		return false;
	}

	return true;
}
