/* -----------------------------------------------------------------------------
 * This file is a part of the NVCM project: https://github.com/nvitya/nvcm
 * Copyright (c) 2019 Viktor Nagy, nvitya
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
 *  file:     hwqspi_atsam_v2.h
 *  brief:    ATSAM_V2 QSPI
 *  version:  1.00
 *  date:     2019-02-15
 *  authors:  nvitya
 *  note:
 *    the QSPI data access restricted to 32 bit here,
 *    32 bit data alignment must be taken care of !
*/

#include "platform.h"

#ifdef QSPI

#include "hwpins.h"
#include "hwqspi_atsam_v2.h"
#include "atsam_v2_utils.h"

#include "hwqspi.h" // for eclipse indexer

bool THwQspi_atsam_v2::InitInterface()
{
	// QSPI / SPIFI pins

	unsigned qspipincfg = PINCFG_AF_H | PINCFG_DRIVE_STRONG;

	hwpinctrl.PinSetup(PORTNUM_A,  8, qspipincfg);  // DATA0
	hwpinctrl.PinSetup(PORTNUM_A,  9, qspipincfg);  // DATA1
	hwpinctrl.PinSetup(PORTNUM_B, 10, qspipincfg);  // SCK
	hwpinctrl.PinSetup(PORTNUM_B, 11, qspipincfg | PINCFG_PULLUP);  // CS
	if (multi_line_count > 2)
	{
		hwpinctrl.PinSetup(PORTNUM_A, 10, qspipincfg);  // DATA2
		hwpinctrl.PinSetup(PORTNUM_A, 11, qspipincfg);  // DATA3
	}

	// MEM2MEM transfers, with SW trigger only
	txdma.Init(txdmachannel, 0); //QSPI_DMAC_ID_TX);
	rxdma.Init(rxdmachannel, 0); //QSPI_DMAC_ID_RX);

	return true;
}

bool THwQspi_atsam_v2::Init()
{
	unsigned tmp;

	initialized = false;

	if (!InitInterface())
	{
		return false;
	}

	if (!txdma.initialized || !rxdma.initialized)
	{
		return false;
	}

	atsam2_enable_mclk(true, 0, 13);

	// DDR mode is not supported. so the 2x clock is not required

	regs = QSPI;

	regs->CTRLA.bit.ENABLE = 0; // enable
	regs->CTRLA.bit.SWRST = 1;   // reset
	if (regs->CTRLA.bit.SWRST) { }

	regs->CTRLB.reg = 0
		| (1 <<  0)  // MODE: 1 = serial memory mode
		| (0 <<  1)  // LOOPEN:
		| (0 <<  2)  // WDRBT
		| (1 <<  4)	 // CSMODE(2), 1 = Last transfer bit
		| (0 <<  8)	 // DATALEN(4), 0 = 8 bits / transfer, 8 = 16 bits / transfer
		| (0 << 16)	 // DLYBCT(8)
		| (0 << 24)	 // DLYCS(8)
	;

	unsigned periphclock = SystemCoreClock;
	unsigned speeddiv = periphclock / speed;
	if (speeddiv * speed < periphclock)  ++speeddiv;

	regs->BAUD.reg = 0
		| (0 <<  0)	  // CPOL
		| (0 <<  1)		// CPHA
		| ((speeddiv - 1) <<  8)		// SCBR(8)
		| (0 << 24)	// DLYBS(8)
	;

	regs->INTENCLR.reg = 0x50F;  // disable interrupts
	regs->INTFLAG.reg  = 0x50F;  // clear interrupt flags

	regs->CTRLA.reg = (1 << 1);  // enable

	rxdma.Prepare(false, (void *)(QSPI_AHB), 0);
	txdma.Prepare(true,  (void *)(QSPI_AHB), 0);

	initialized = true;

	return true;
}

int THwQspi_atsam_v2::StartReadData(unsigned acmd, unsigned address, void * dstptr, unsigned len)
{
	if (busy)
	{
		return ERROR_BUSY;
	}

	istx = false;
	dataptr = (uint8_t *)dstptr;
	datalen = len;
	remainingbytes = datalen;

  unsigned cmd = (acmd & 0xFF);

  regs->INSTRCTRL.reg = (cmd | ((dummydata & 0xFF) << 16)); // set instruction and opcode

	unsigned ifr = 0
		| (0 <<  0)    // WIDTH(3)
		| (1 <<  4)    // INSTEN, 1 = send the instruction code
		| (0 <<  5)    // ADDREN
		| (0 <<  6)    // OPTEN
		| (0 <<  7)    // DATAEN
		| (0 <<  8)    // OPTL(2)
		| (0 << 10)    // ADDRL
		| (0 << 12)    // TRFTYP(2)
		| (0 << 14)    // CRMODE
		| (0 << 15)    // DDREN
		| (0 << 16)    // DUMMYLEN(5)
	;

	// set WIDTH
	unsigned fields = ((acmd >> 8) & 0xF);
	if (fields != 0)
	{
		unsigned multiiooffs;
		if (multi_line_count == 4)  multiiooffs = 1;
		else                        multiiooffs = 0;

		if      (fields == 0xE)  ifr |= (3 + multiiooffs);  // SMM S opcode, M others
		else if (fields == 0x8)  ifr |= (1 + multiiooffs);  // SSM S opcode, S addres + dummy, M data
		else if (fields == 0xF)  ifr |= (5 + multiiooffs);  // MMM M all
	}

	unsigned char calen = ((acmd >> 16) & 0xF);
	if (calen != 0)
	{
		regs->INSTRADDR.reg = address;
		if (8 == calen)  calen = addrlen;
		if (4 == calen)  ifr |= (1 << 10); // 32 bit address, 24 bit otherwise
		else if (3 != calen)
		{
			// other address length are not supported!
			__BKPT();
		}

		ifr |= (1 << 5); // send address

		qspidatamem = (uint32_t *)(QSPI_AHB + address);
	}
	else
	{
		qspidatamem = (uint32_t *)(QSPI_AHB);
	}


	unsigned trftyp;
	unsigned dcnt;

	if (0xEB == cmd)
	{
		// special quad read command with 1 byte opcode
		ifr |= 0
			| (1 <<  6)  // enable option
			| (3 <<  8)  // 8 bit option data (2 cycles)
		;

		dcnt = 4;
		trftyp = 1;
	}
	else if (0xBB == cmd)
	{
		dcnt = 4;
		trftyp = 1;
	}
	else if (0x0B == cmd)
	{
		dcnt = 8;
		trftyp = 1;
	}
	else
	{
		dcnt = ((acmd >> 20) & 0xF);
		if (8 == dcnt)  dcnt = dummysize;
		dcnt <<= 3;
		trftyp = 0;
	}

	ifr |= ((trftyp << 12) | (dcnt << 16));

	if (datalen > 0)  ifr |= (1 << 7);

	regs->INSTRFRAME.reg = ifr;

	if (regs->INSTRFRAME.reg) { } // to synchronize bus accesses

	// always use DMA for data fetch otherwise it will be blocked for longer time
	dmaused = (remainingbytes >= 4);
	//dmaused = false;
	if (dmaused)
	{
		xfer.srcaddr = qspidatamem;
		xfer.dstaddr = dstptr;
		xfer.bytewidth = 4;
		xfer.count = (remainingbytes >> 2);
		xfer.flags = DMATR_MEM_TO_MEM;
		rxdma.StartTransfer(&xfer);

		remainingbytes = 0;
	}

	// the operation should be started...

	runstate = 0;

	busy = true;

	return ERROR_OK;
}

int THwQspi_atsam_v2::StartWriteData(unsigned acmd, unsigned address, void * srcptr, unsigned len)
{
	if (busy)
	{
		return ERROR_BUSY;
	}

	if (busy)
	{
		return ERROR_BUSY;
	}

	istx = false;
	dataptr = (uint8_t *)srcptr;
	datalen = len;
	remainingbytes = datalen;

  unsigned cmd = (acmd & 0xFF);

  regs->INSTRCTRL.reg = (cmd | ((dummydata & 0xFF) << 16)); // set instruction and opcode

	unsigned ifr = 0
		| (0 <<  0)    // WIDTH(3)
		| (1 <<  4)    // INSTEN, 1 = send the instruction code
		| (0 <<  5)    // ADDREN
		| (0 <<  6)    // OPTEN
		| (0 <<  7)    // DATAEN
		| (0 <<  8)    // OPTL(2)
		| (0 << 10)    // ADDRL
		| (0 << 12)    // TRFTYP(2)
		| (0 << 14)    // CRMODE
		| (0 << 15)    // DDREN
		| (0 << 16)    // DUMMYLEN(5)
	;

	// set WIDTH
	unsigned fields = ((acmd >> 8) & 0xF);
	if (fields != 0)
	{
		unsigned multiiooffs;
		if (multi_line_count == 4)  multiiooffs = 1;
		else                        multiiooffs = 0;

		if      (fields == 0xE)  ifr |= (3 + multiiooffs);  // SMM S opcode, M others
		else if (fields == 0x8)  ifr |= (1 + multiiooffs);  // SSM S opcode, S addres + dummy, M data
		else if (fields == 0xF)  ifr |= (5 + multiiooffs);  // MMM M all
	}

	unsigned char calen = ((acmd >> 16) & 0xF);
	if (calen != 0)
	{
		//regs->QSPI_IAR = address;
		if (8 == calen)  calen = addrlen;
		if (4 == calen)  ifr |= (1 << 10); // 32 bit address, 24 bit otherwise
		else if (3 != calen)
		{
			// other address length are not supported!
			__BKPT();
		}

		ifr |= (1 << 5); // send address

		qspidatamem = (uint32_t *)(QSPI_AHB + address);
	}
	else
	{
		qspidatamem = (uint32_t *)(QSPI_AHB);
	}


	unsigned dcnt = ((acmd >> 20) & 0xF);
	if (8 == dcnt)  dcnt = dummysize;

	unsigned trftyp;
	if ((0x32 == cmd) || (0x02 == cmd))
	{
		// write memory
		trftyp = 3;
	}
	else
	{
		trftyp = 2;
	}

	ifr |= ((trftyp << 12) | (dcnt << 19));

	if (datalen > 0)  ifr |= (1 << 7);

	regs->INSTRFRAME.reg = ifr;

	if (regs->INSTRFRAME.reg) { } // to synchronize bus accesses

	// always use DMA for data fetch otherwise it will be blocked for longer time
	dmaused = (remainingbytes >= 4);
	if (dmaused)
	{
		xfer.srcaddr = srcptr;
		xfer.dstaddr = qspidatamem;
		xfer.bytewidth = 4;
		xfer.count = (remainingbytes >> 2);
		xfer.flags = DMATR_MEM_TO_MEM;
		txdma.StartTransfer(&xfer);

		remainingbytes = 0;
	}

	// the operation should be started...

	runstate = 0;

	busy = true;

	return ERROR_OK;
}


void THwQspi_atsam_v2::Run()
{
	if (!busy)
	{
		return;
	}

	unsigned sr = 0;

	if (istx)
	{
		if (dmaused && txdma.Enabled())
		{
			return;
		}

		// send the remaining bytes, BLOCKING !
		while (remainingbytes >= 4)
		{
			*qspidatamem++ = *((uint32_t *)dataptr);
			dataptr += 4;
			remainingbytes -= 4;;
		}
	}
	else
	{
		if (dmaused && rxdma.Enabled())
		{
			return;
		}

		// get the remaining bytes, BLOCKING !
		while (remainingbytes >= 4)
		{
			*((uint32_t *)dataptr) = *qspidatamem++;
			dataptr += 4;
			remainingbytes -= 4;
		}
	}


	if (0 == runstate)
	{
		regs->CTRLA.reg = (1 << 1) | (1 << 24);  // signal last transfer, keep enabled
		runstate = 1;
		return;
	}

	// wait until CS pulled back
	if (regs->STATUS.bit.CSSTATUS == 0)
	{
		// instruction not finished yet
		return;
	}

	busy = false;
}

#endif
