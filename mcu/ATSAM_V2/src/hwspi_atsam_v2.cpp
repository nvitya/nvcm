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
 *  file:     hwspi_atsam_v2.cpp
 *  brief:    ATSAM_V2 SPI
 *  version:  1.00
 *  date:     2019-01-17
 *  authors:  nvitya
 *  notes:
 *    the SERCOM pads are used as follows:
 *      pad 0 = MOSI
 *      pad 1 = SCK
 *      pad 2 = CS
 *      pad 3 = MISO
*/

#include <stdio.h>
#include <stdarg.h>

#include "hwspi_atsam_v2.h"
#include "hwspi.h"  // for eclipse indexer
#include "atsam_v2_utils.h"

bool THwSpi_atsam_v2::Init(int adevnum)  // devnum: 0 - 7 = SERCOM ID
{
	uint32_t tmp;
	unsigned perid;

	devnum = adevnum;
	initialized = false;
	regs = nullptr;

	if (!atsam2_sercom_enable(devnum, 0))
	{
		return false;
	}

	regs = (HW_SPI_REGS *)sercom_inst_list[devnum];

	regs->CTRLA.bit.ENABLE = 0; // disable
	//regs->CTRLA.bit.SWRST = 1; // reset
	//while (regs->SYNCBUSY.bit.SWRST) { } // wait for reset
	//regs->CTRLA.bit.SWRST = 0; // reset

	while (regs->SYNCBUSY.bit.ENABLE) { } // wait for sync

	uint32_t periphclock = (SystemCoreClock >> 1);

	unsigned brdiv = (periphclock / speed);
	if (brdiv < 1)  brdiv = 1;

	regs->BAUD.reg = (brdiv - 1);

	while (regs->SYNCBUSY.bit.CTRLB) { } // wait for sync

	// CTRLB
	regs->CTRLB.reg = 0
		| (1 << 17)  // RXEN: RX Enable
		| (0 << 14)  // AMODE(2): Address mode
		| (0 << 13)  // MSSEN: 1 = HW CS(SS) control + inter character spacing
		| (0 <<  9)  // SSDE: 1 = Slave Select Low Detect Enable
		| (0 <<  6)  // PLOADEN: 1 = data preload enable
		| (0 <<  0)  // CHSIZE(3): 0 = 8 bit, 1 = 9 bit, more options are not supported
	;

	while (regs->SYNCBUSY.bit.CTRLB) { } // wait for sync

	// CTRLA
	tmp = 0
		| (0 << 30)  // DORD: 0 = MSB First, 1 = LSB first
		| (0 << 29)  // CPOL: Clock Polarity, 1 = SCK is high on idle
		| (0 << 28)  // CPHA: 1 = late sample
		| (0 << 24)  // FORM(4): frame format, 0 = SPI, 2 = SPI with address
		| (3 << 20)  // DIPO(2): Data In Pinout, pad select for MISO
		| (0 << 16)  // DOPO(2): Data Out Pinout, 0 = P0:MOSI|P1:SCK|P2:SS
		| (0 <<  8)  // IBON: Immediate Buffer Overflow Notification
		| (0 <<  7)  // RUNSTDBY: Run In Standby, 1 = run in stdby
		| (3 <<  2)  // MODE(3): Mode, 3 = SPI Master
		| (0 <<  1)  // ENABLE
		| (0 <<  0)  // SWRST: Software Reset
	;

	if (lsb_first)        tmp |= (1 << 30);
	if (idleclk_high)     tmp |= (1 << 29);
	if (datasample_late)  tmp |= (1 << 28);

	regs->CTRLA.reg = tmp;
	regs->CTRLC.reg = (0 << 24);

	regs->DBGCTRL.reg = 0;

	while (regs->SYNCBUSY.bit.ENABLE) { } // wait for enable
	regs->CTRLA.reg = (tmp | (1 << 1)); // enable it
	while (regs->SYNCBUSY.bit.ENABLE) { } // wait for enable

	initialized = true;

	return true;
}

bool THwSpi_atsam_v2::TrySendData(unsigned short adata)
{
	if (regs->INTFLAG.bit.DRE)
	{
		regs->DATA.reg = adata;
		return true;
	}
	else
	{
		return false;
	}
}

bool THwSpi_atsam_v2::TryRecvData(unsigned short * dstptr)
{
	if (regs->INTFLAG.bit.RXC)
	{
		*dstptr = regs->DATA.reg;
		return true;
	}
	else
	{
		return false;
	}
}

void THwSpi_atsam_v2::DmaAssign(bool istx, THwDmaChannel * admach)
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


bool THwSpi_atsam_v2::DmaStartSend(THwDmaTransfer * axfer)
{
	if (!txdma)
	{
		return false;
	}

	txdma->StartTransfer(axfer);

	return true;
}

bool THwSpi_atsam_v2::DmaStartRecv(THwDmaTransfer * axfer)
{
	if (!rxdma)
	{
		return false;
	}

	rxdma->StartTransfer(axfer);

	return true;
}

bool THwSpi_atsam_v2::DmaSendCompleted()
{
	if (txdma && txdma->Active())
	{
		// Send DMA is still active
		return false;
	}

	return SendFinished();
}

bool THwSpi_atsam_v2::DmaRecvCompleted()
{
	if (rxdma && rxdma->Active())
	{
		// Recv DMA is still active
		return false;
	}

	return true;
}




