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
 *  file:     hwqspi_atsam.h
 *  brief:    ATSAM QSPI
 *  version:  1.00
 *  date:     2018-02-10
 *  authors:  nvitya
*/

#include "platform.h"
#include "hwpins.h"
#include "hwqspi.h"

#ifdef QSPI

bool THwQspi_atsam::InitInterface()
{
	// QSPI / SPIFI pins

	unsigned qspipincfg = PINCFG_AF_A | PINCFG_INPUT | PINCFG_PULLUP | PINCFG_SPEED_FAST;

	hwpinctrl.PinSetup(PORTNUM_A, 11, qspipincfg);  // CS
	hwpinctrl.PinSetup(PORTNUM_A, 13, qspipincfg);  // QIO0
	hwpinctrl.PinSetup(PORTNUM_A, 12, qspipincfg);  // QIO1
	hwpinctrl.PinSetup(PORTNUM_A, 17, qspipincfg);  // QIO2
	hwpinctrl.PinSetup(PORTNUM_D, 31, qspipincfg);  // QIO3
	hwpinctrl.PinSetup(PORTNUM_A, 14, qspipincfg);  // SCK

	txdma.Init(txdmachannel, 5);
	rxdma.Init(rxdmachannel, 6);

	return true;
}

bool THwQspi_atsam::Init()
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

	PMC->PMC_PCER1 = (1 << (ID_QSPI-32));  // Enable the QSPI unit

	regs = QSPI;

	regs->QSPI_CR = QSPI_CR_QSPIDIS; // disable

	regs->QSPI_CR = QSPI_CR_SWRST;   // reset

	regs->QSPI_MR = 0
		| (1 <<  0)	  // SMM: 1 = serial memory mode
		| (0 <<  1)		// LBB
		| (0 <<  2)		// WDRBT
		| (1 <<  4)		// CSMODE(2), 1 = Last transfer bit
		| (0 <<  8)		// NBITS(4), 0 = 8 bits / transfer, 8 = 16 bits / transfer
		| (0 << 16)	// DLYBCT(8)
		| (0 << 24)	// DLYCS(8)
	;

	unsigned periphclock = SystemCoreClock;
	if (periphclock > 150000000)  periphclock = periphclock / 2;

	unsigned speeddiv = periphclock / speed;
	if (speeddiv * speed < periphclock)  ++speeddiv;

	regs->QSPI_SCR = 0
		| (0 <<  0)	  // CPOL
		| (0 <<  1)		// CPHA
		| ((speeddiv - 1) <<  8)		// SCBR(8)
		| (0 << 24)	// DLYBS(8)
	;

	regs->QSPI_IDR = 0x70F;  // disable interrupts
	regs->QSPI_SMR = 0;

	regs->QSPI_CR = QSPI_CR_QSPIEN;

	rxdma.Prepare(false, (void *)(0x80000000), 0);
	txdma.Prepare(true,  (void *)(0x80000000), 0);

	initialized = true;

	return true;
}

int THwQspi_atsam::StartReadData(unsigned acmd, unsigned address, void * dstptr, unsigned len)
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

  regs->QSPI_ICR = (cmd | ((dummydata & 0xFF) << 16)); // set instruction and opcode

	unsigned ifr = 0
		| (0 <<  0)    // WIDTH(3)
		| (1 <<  4)    // INSTEN, 1 = send the instruction code
		| (0 <<  5)    // ADDREN
		| (0 <<  6)    // OPTEN
		| (0 <<  7)    // DATAEN
		| (0 <<  8)    // OPTL(2)
		| (0 << 10)    // ADDRL
		| (0 << 12)    // TRFTYP(2)
		| (0 << 14)    // CRM
		| (0 << 16)    // NBNUM(5)
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
		regs->QSPI_IAR = address;
		if (8 == calen)  calen = addrlen;
		if (4 == calen)  ifr |= (1 << 10); // 32 bit address, 24 bit otherwise
		else if (3 != calen)
		{
			// other address length are not supported!
			__BKPT();
		}

		ifr |= (1 << 5); // send address

		qspidatamem = (unsigned char *)(QSPIMEM_ADDR + address);
	}
	else
	{
		qspidatamem = (unsigned char *)(QSPIMEM_ADDR);
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

	regs->QSPI_IFR = ifr;

	ifr = regs->QSPI_IFR; // to synchronize bus accesses

	// always use DMA for data fetch otherwise it will be blocked for longer time
	dmaused = (remainingbytes > 1);

	//dmaused = false;

	if (dmaused)
	{
		xfer.srcaddr = qspidatamem;
		xfer.dstaddr = dstptr;
		xfer.bytewidth = 1;
		xfer.count = remainingbytes;
		xfer.flags = DMATR_MEM_TO_MEM;
		rxdma.StartTransfer(&xfer);

		remainingbytes = 0;
	}

	// the operation should be started...

	runstate = 0;

	busy = true;

	return ERROR_OK;
}

int THwQspi_atsam::StartWriteData(unsigned acmd, unsigned address, void * srcptr, unsigned len)
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

  regs->QSPI_ICR = (cmd | ((dummydata & 0xFF) << 16)); // set instruction and opcode

	unsigned ifr = 0
		| (0 <<  0)    // WIDTH(3)
		| (1 <<  4)    // INSTEN, 1 = send the instruction code
		| (0 <<  5)    // ADDREN
		| (0 <<  6)    // OPTEN
		| (0 <<  7)    // DATAEN
		| (0 <<  8)    // OPTL(2)
		| (0 << 10)    // ADDRL
		| (0 << 12)    // TRFTYP(2)
		| (0 << 14)    // CRM
		| (0 << 16)    // NBNUM(5)
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

		qspidatamem = (unsigned char *)(QSPIMEM_ADDR + address);
	}
	else
	{
		qspidatamem = (unsigned char *)(QSPIMEM_ADDR);
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

	regs->QSPI_IFR = ifr;

	ifr = regs->QSPI_IFR; // to synchronize bus accesses

	// always use DMA for data fetch otherwise it will be blocked for longer time
	dmaused = (remainingbytes > 1);
	if (dmaused)
	{
		xfer.srcaddr = srcptr;
		xfer.dstaddr = qspidatamem;
		xfer.bytewidth = 1;
		xfer.count = remainingbytes;
		xfer.flags = DMATR_MEM_TO_MEM;
		txdma.StartTransfer(&xfer);

		remainingbytes = 0;
	}

	// the operation should be started...

	runstate = 0;

	busy = true;

	return ERROR_OK;
}


void THwQspi_atsam::Run()
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
		while (remainingbytes > 0)
		{
			*qspidatamem++ = *dataptr++;
			--remainingbytes;
		}

/*
		sr = regs->QSPI_SR;

		if (!(sr & QSPI_SR_TXEMPTY))
		{
			return;
		}
*/
	}
	else
	{
		if (dmaused && rxdma.Enabled())
		{
			return;
		}

		// get the remaining bytes, BLOCKING !
		while (remainingbytes > 0)
		{
			*dataptr++ = *qspidatamem++;
			--remainingbytes;
		}
	}


	if (0 == runstate)
	{
		regs->QSPI_CR = QSPI_CR_LASTXFER;  // signal last transfer
		runstate = 1;
		return;
	}

	// wait until CS pulled back
	if ((regs->QSPI_SR & QSPI_SR_CSS) == 0)
	{
		// instruction not finished yet
		return;
	}

	busy = false;
}

#endif
