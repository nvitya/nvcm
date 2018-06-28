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
 *  file:     hwqspi_lpc.cpp
 *  brief:    LPC QSPI
 *  version:  1.00
 *  date:     2018-02-10
 *  authors:  nvitya
*/

#include "platform.h"
//#include "lpc_utils.h"
#include <hwqspi.h>

bool THwQspi_lpc_v3::InitInterface()
{
	// QSPI / SPIFI pins, no alternatives !

	unsigned qspipincfg = PINCFG_AF_6; // | PINCFG_PULLUP | PINCFG_SPEED_FAST;

	hwpinctrl.PinSetup(0, 23, qspipincfg);  // SPIFI_CS
	hwpinctrl.PinSetup(0, 24, qspipincfg);  // SPIFI_IO0
	hwpinctrl.PinSetup(0, 25, qspipincfg);  // SPIFI_IO1
	hwpinctrl.PinSetup(0, 26, qspipincfg);  // SPIFI_SCK
	hwpinctrl.PinSetup(0, 27, qspipincfg);  // SPIFI_IO3
	hwpinctrl.PinSetup(0, 28, qspipincfg);  // SPIFI_IO2

	// The DMA channel 18 is tied to the SPIFI / QSPI !
	txdma.Init(18);
	// use the same channel for tx and rx
	rxdma.Init(18);

	return true;
}

bool THwQspi_lpc_v3::Init()
{
	initialized = false;

	if (!InitInterface())
	{
		return false;
	}

	if (!txdma.initialized || !rxdma.initialized)
	{
		return false;
	}

	// Pins are already configured for the SPIFI

	SYSCON->AHBCLKCTRLSET[0] = (1 << 10); // enable SPIFI


	SYSCON->SPIFICLKSEL = 3; // select the 48 MHz free running oscillator as clock source
	unsigned basespeed = 48000000;


	unsigned speeddiv = basespeed / speed;
	if (speeddiv * speed < basespeed)  ++speeddiv;

	SYSCON->SPIFICLKDIV = (speeddiv - 1);

	regs = (HW_QSPI_REGS *)SPIFI0_BASE;

	// SPIFI reset

	regs->STAT = 0x10;   // reset
	while (regs->STAT & 0x10)
	{
		// wait for the reset
	}

	regs->IDATA  = 0x0;  	// dummy data
	regs->CLIMIT = 0x0;   //

	// calculate base CTRL register
	ctrlbase = 0
		| (0xFFFF << 0)  	// TIMEOUT
		| (1 << 16)	  // minimum CS high time
		| (1 << 21)		// 1 = disable prefetch
		| (0 << 22)		// 1 = enable interrupt
		| (1 << 23)		// mode 3 (clock high by default)
		| (1 << 27)		// disable prefetch
		| (0 << 28)		// 0 = quad protocol, 1 = dual protocol
		| (1 << 29)		// 1 = sampling at rising edge, 0 = sampling at falling edge
		| (0 << 30)		// 0 = internal clock (1 = use feedback clock)
		| (1 << 31) 	// 1 = enable DMA
	;

	if (multi_line_count == 2)  ctrlbase |= (1 << 28);

	regs->CTRL = ctrlbase;

	rxdma.Prepare(false, (void *)&(regs->DATA), 0);
	txdma.Prepare(true,  (void *)&(regs->DATA), 0);

	initialized = true;

	return true;
}

int THwQspi_lpc_v3::StartReadData(unsigned acmd, unsigned address, void * dstptr, unsigned len)
{
	if (busy)
	{
		return ERROR_BUSY;
	}

	istx = false;
	dataptr = (uint8_t *)dstptr;
	datalen = len;
	remainingbytes = datalen;

	regs->STAT = 0x20; // clear interrupt;

	unsigned frame = 1; // send command code only by default
	unsigned char calen = ((acmd >> 16) & 0xF);
	if (calen != 0)
	{
		regs->ADDR = address;
		if (8 == calen)
		{
			frame += addrlen;  // use the default address size
		}
		else
		{
			frame += calen;
		}
	}

	unsigned fields = ((acmd >> 8) & 0xF);
	if (fields != 0)
	{
		if      (fields == 0xE)  fields = 2;  // S opcode, M others
		else if (fields == 0x8)  fields = 1;  // S opcode, S addres, S dummy, M data
		else if (fields == 0xF)  fields = 3;  // M all
		else                     fields = 0;  // S all
	}

	unsigned dcnt = ((acmd >> 20) & 0xF);
	if (8 == dcnt)  dcnt = dummysize;

	unsigned cmd = 0
		|	(len << 0)      // Data length
		|	(0 << 14)			  // POLL
		|	(0 << 15)			  // 0 = input/read, 1 = output/write
		|	(dcnt   << 16)	// INTLEN (bytes after address), 2 = 8 dummy clocks (in dual mode)
		|	(fields << 19)	// FIELDFORM: 2 = opcode is serial, others are dual/quad
		|	(frame  << 21)	// FRAMEFORM: 3 = opcode + 2 bytes of address
		|	((acmd & 0xFF) << 24)
	;

	dmaused = (len > 4);

	if (dmaused)
	{
		rxdma.Prepare(false, (void *)&(regs->DATA), 0);  // we are using the same DMA channel, so it must be switched to rx

		// The LPC SPIFI DMA transfer works only with 4 byte units !!!
		// the DMA requests are generated after 4 full bytes are received

		if (len > 0)
		{
			xfer.bytewidth = 4;
			xfer.count = ((len + 3) >> 2);
			xfer.flags = 0;
			xfer.dstaddr = dstptr;

	  	dataptr += (xfer.count << 2);
	  	remainingbytes &= 0x03;

			rxdma.StartTransfer(&xfer);
		}
	}

	regs->CMD = cmd;

	// the operation should be started...

	busy = true;

	return ERROR_OK;
}

int THwQspi_lpc_v3::StartWriteData(unsigned acmd, unsigned address, void * srcptr, unsigned len)
{
	if (busy)
	{
		return ERROR_BUSY;
	}

	istx = true;
	dataptr = (uint8_t *)srcptr;
	datalen = len;
	remainingbytes = datalen;

	regs->STAT = 0x20; // clear interrupt;

	unsigned frame = 1; // send command code only by default
	unsigned char calen = ((acmd >> 16) & 0xF);
	if (calen != 0)
	{
		regs->ADDR = address;
		if (8 == calen)
		{
			frame += addrlen;  // use the default address size
		}
		else
		{
			frame += calen;
		}
	}

	unsigned fields = ((acmd >> 8) & 0xF);
	if (fields != 0)
	{
		if      (fields == 0xE)  fields = 2;  // S opcode, M others
		else if (fields == 0x8)  fields = 1;  // S opcode, S addres, S dummy, M data
		else if (fields == 0xF)  fields = 3;  // M all
		else                     fields = 0;  // S all
	}

	unsigned dcnt = 0;  // no dummy at the writes

	unsigned cmd = 0
		|	(len << 0)      // Data length
		|	(0 << 14)			  // POLL
		|	(1 << 15)			  // 0 = input/read, 1 = output/write
		|	(dcnt   << 16)	// INTLEN (bytes after address), 2 = 8 dummy clocks (in dual mode)
		|	(fields << 19)	// FIELDFORM: 2 = opcode is serial, others are dual/quad
		|	(frame  << 21)	// FRAMEFORM: 4 = opcode + 3 bytes of address
		|	((acmd & 0xFF) << 24)
	;

	dmaused = (len > 4);

	if (dmaused)
	{
		txdma.Prepare(true, (void *)&(regs->DATA), 0);  // we are using the same DMA channel, so it must be switched to rx

		// The LPC SPIFI DMA transfer works only with 4 byte units !!!
		// the DMA requests are generated after 4 full bytes are received

		xfer.bytewidth = 4;
		xfer.count = ((len + 3) >> 2);
		xfer.flags = 0;
		xfer.srcaddr = dataptr;

		dataptr += (xfer.count << 2);
		remainingbytes &= 0x03;

		txdma.StartTransfer(&xfer);
	}

	regs->CMD = cmd;

	// the operation should be started...

	busy = true;

	if (dmaused)
	{
  	//txdma.StartTransfer(&xfer);
	}
	else
	{
		Run(); // to fill the transmit FIFO
	}

	return ERROR_OK;
}


void THwQspi_lpc_v3::Run()
{
	if (!busy)
	{
		return;
	}

	if (istx)
	{
		if (dmaused && txdma.Enabled())
		{
			return;
		}

		// send the tail
		while (remainingbytes > 0)
		{
			*(uint8_t *)&(regs->DATA) = *dataptr++;
			--remainingbytes;
		}

		if (regs->STAT & 0x02)
		{
			return;
		}
	}
	else
	{
		if (regs->STAT & 0x02)
		{
			return;
		}

		if (dmaused && rxdma.Enabled())
		{
			return;
		}

		// get the tail
		while (remainingbytes > 0)
		{
			*dataptr++ = *(uint8_t *)&(regs->DATA);
			--remainingbytes;
		}

	}

	busy = false;
}

