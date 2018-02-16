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
 *  file:     hwspi_lpc_v3.h
 *  brief:    LPC_V3 SPI (master only)
 *  version:  1.00
 *  date:     2018-02-10
 *  authors:  nvitya
*/

#include "platform.h"
#include "hwspi.h"

static const unsigned fcbaseaddr[] = FLEXCOMM_BASE_ADDRS;

bool THwSpi_lpc_v3::Init(int adevnum)
{
	if ((adevnum < 0) || (adevnum > 9))
	{
		devnum = -1;
		return false;
	}

	devnum = adevnum;

	// turn on the flexcomm hw:
	if (devnum <= 7)
	{
		SYSCON->AHBCLKCTRLSET[1] = (1 << (11 + devnum));
	}
	else
	{
		SYSCON->AHBCLKCTRLSET[2] = (1 << (14 + (devnum - 8)));
	}

	// set the comm type
	FLEXCOMM_Type * fc = (FLEXCOMM_Type *)(fcbaseaddr[devnum]);
	fc->PSELID = 2; // select SPI

	regs = (HW_SPI_REGS *)fc;

	SYSCON->FCLKSEL[devnum] = 1; // select the 48 MHz free running oscillator as clock source
	basespeed = 48000000;

	unsigned n;

	n = 0
		| (0 << 0)  // do not enable yet
		| (1 << 2)  // 1 = Master mode
		| (0 << 3)  // 0 = MSB first
		| (0 << 4)  // CPHA: clock phase select
		| (0 << 5)  // CPOL: clock polarity, 0 = low lon indle
		| (0 << 8)  // SPOLx: SSEL is active low
	;
	if (lsb_first)        n |= (1 << 3);
	if (datasample_late)  n |= (1 << 4);
	if (idleclk_high)  		n |= (1 << 5);
	regs->CFG = n;

	regs->DIV = ((basespeed / speed) - 1);

	regs->DLY = 0
		| (0 <<  0)  // PRE_DELAY(4)
		| (0 <<  4)  // POST_DELAY(4)
		| (0 <<  8)  // FRAME_DELAY(4)
		| (0 << 12)  // TRANSFER_DELAY(4)
	;

	regs->INTENSET = 0; // disable interrupts

	// set the control word:
	unsigned short * ctrlbits = (unsigned short *)&(regs->FIFOWR);
	++ctrlbits;
	*ctrlbits = 0
		| (15 <<  0) // assert all chip selects
		| (0  <<  4)  // EOT
		| (0  <<  5)  // EOF
		| (0  <<  6)  // RXIGNORE
	  | ((databits - 1) << 8)
	;

	regs->FIFOCFG = 0
		| (1 <<  0)  // 1 = Enable TX
		| (1 <<  1)  // 1 = Enable RX
		| (1 <<  4)  // 8 x 16 bit (this is the only possibility)
		| (0 << 12)  // 1 = Enable TX DMA
		| (0 << 13)  // 1 = Enable RX DMA
		| (3 << 16)  // clear FIFOs
	;

	// Enable
	regs->CFG |= 1;

	return true;
}

bool THwSpi_lpc_v3::TrySendData(uint16_t adata)
{
	if (regs->FIFOSTAT & (1 << 5)) // TX FIFO is not full
	{
		regs->FIFOWR = adata;
		return true;
	}
	else
	{
		return false;
	}
}

bool THwSpi_lpc_v3::TryRecvData(uint16_t * dstptr)
{
	if (regs->FIFOSTAT & (1 << 6))  // RX FIFO not empty
	{
		*dstptr = regs->FIFORD;
		return true;
	}
	else
	{
		return false;
	}
}

void THwSpi_lpc_v3::DmaAssign(bool istx, THwDmaChannel * admach)
{
	if (istx)
	{
		txdma = admach;
		txdma->Prepare(true, (void *)&(regs->FIFOWR), 0);
		regs->FIFOCFG |= (1 << 12); // enable the TX DMA
	}
	else
	{
		rxdma = admach;
		rxdma->Prepare(false, (void *)&regs->FIFORD, 0);
		regs->FIFOCFG |= (1 << 13); // enable the RX DMA
	}
}

bool THwSpi_lpc_v3::DmaStartSend(THwDmaTransfer * axfer)
{
	txdma->StartTransfer(axfer);
	return true;
}

bool THwSpi_lpc_v3::DmaStartRecv(THwDmaTransfer * axfer)
{
	rxdma->StartTransfer(axfer);
	return true;
}


bool THwSpi_lpc_v3::SendFinished()
{
	if (regs->STAT & (1 << 8))  // SPI Master Idle
	{
		return true;
	}
	else
	{
		return false;
	}
}

bool THwSpi_lpc_v3::DmaSendCompleted()
{
	if (txdma && txdma->Active())
	{
		// Send DMA is still active
		return false;
	}

	return SendFinished();
}

bool THwSpi_lpc_v3::DmaRecvCompleted()
{
	if (rxdma && rxdma->Active())
	{
		// Send DMA is still active
		return false;
	}

	return true;
}
