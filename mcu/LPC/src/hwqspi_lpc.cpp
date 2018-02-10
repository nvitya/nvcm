/*
 * hwqspi_lpc.cpp
 *
 *  Created on: 2018. jan. 17.
 *      Author: vitya
 */

#include "platform.h"
#include "lpc_utils.h"
#include <hwqspi.h>

bool THwQspi_lpc::InitInterface()
{
	// QSPI / SPIFI pins

	unsigned qspipincfg = PINCFG_AF_3 | PINCFG_INPUT | PINCFG_PULLUP | PINCFG_SPEED_FAST;

	hwpinctrl.PinSetup(3, 3, qspipincfg);  // SPIFI_SCK
	hwpinctrl.PinSetup(3, 4, qspipincfg);  // SPIFI_SIO3
	hwpinctrl.PinSetup(3, 5, qspipincfg);  // SPIFI_SIO2
	hwpinctrl.PinSetup(3, 6, qspipincfg);  // SPIFI_MISO
	hwpinctrl.PinSetup(3, 7, qspipincfg);  // SPIFI_MOSI
	hwpinctrl.PinSetup(3, 8, qspipincfg);  // SPIFI_CS

	// The DMA channel 7 is used by the QSPI !
	txdma.Init(0x000007);  // perid = 0, DMA mux = 0
	// use the same channel for tx and rx
	rxdma.Init(0x000007);

	return true;
}

bool THwQspi_lpc::Init()
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

	unsigned speeddiv = SystemCoreClock / speed;
	if (speeddiv * speed < SystemCoreClock)  ++speeddiv;

	lpc_set_clock_divider(CLK_IDIV_E, CLKIN_MAINPLL, speeddiv);

	lpc_set_base_clock(CLK_BASE_SPIFI, CLKIN_IDIVE, false, false); // select the IDIVE as the base clock for the SPIFI

	lpc_enable_clock(CLK_MX_SPIFI, 1);

	regs = (SPIFI_REGS_T *)LPC_SPIFI_BASE;

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

int THwQspi_lpc::StartReadData(unsigned acmd, unsigned address, void * dstptr, unsigned len)
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
			xfer.addrinc = true;
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

int THwQspi_lpc::StartWriteData(unsigned acmd, unsigned address, void * srcptr, unsigned len)
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
		xfer.addrinc = true;
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


void THwQspi_lpc::Run()
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
			regs->DATA8 = *dataptr++;
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
			*dataptr++ = regs->DATA8;
			--remainingbytes;
		}

	}

	busy = false;
}

