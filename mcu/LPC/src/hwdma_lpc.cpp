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
 *  file:     hwdma_lpc.cpp
 *  brief:    LPC DMA
 *  version:  1.00
 *  date:     2018-02-10
 *  authors:  nvitya
*/

#include <stdio.h>
#include <stdarg.h>

#include "hwdma.h"

#define MAX_DMA_CHANNELS  GPDMA_NUMBER_CHANNELS

/* DmaChannel::Init for LPC:
 *
 * achnum:
 *   bit0..7:    channel id
 *   bit8..15:   0-15 = peripheral request id
 *   bit16..23:  0-3 = peripheral request mux (DMA mux) for the request id
 *
 *   for request ids search DMAMUX in the LPC43xx reference manual
*/
bool THwDmaChannel_lpc::Init(int achnum)
{
	unsigned tmp;

	initialized = false;

	chnum = (achnum & 0xFF);
	perid = ((achnum >> 8) & 0xF);  // rq id
	unsigned rqmux = ((achnum >> 16) & 0x3);

	if ((chnum < 0) || (chnum >= MAX_DMA_CHANNELS))
	{
		chnum = -1;
		return false;
	}

	chbit = (1 << chnum);
  regs = (HW_DMA_REGS *)&(LPC_GPDMA->CH[chnum]);

	// DMA Global

	LPC_GPDMA->CONFIG = 0x01;  // enable the DMA (with little endian settings)

	// DMA request mux
	tmp = LPC_CREG->DMAMUX;
	tmp &= ~(    3 << (perid << 1));
	tmp |=  (rqmux << (perid << 1));
	LPC_CREG->DMAMUX = tmp;

	LPC_GPDMA->SYNC |= (1 << perid);	 // enable SYNC logic

	initialized = true;

	return true;
}

void THwDmaChannel_lpc::Prepare(bool aistx, void * aperiphaddr, unsigned aflags)
{
	istx = aistx;
	periphaddr = aperiphaddr;

	// prepare the channel config register

	unsigned tmp = 0
		| (0 <<  0)			// enable
		|	(0 <<  1)			// SRCPERIPHERAL(4): src peripheral
		| (0 <<  6)			// DESTPERIPHERAL(4): dst peripheral
		| (0 << 11)			// FLOWCNTRL(3): 2 = per -> mem, 1 = mem -> per
		| (0 << 14)			// 1 = enable Error interrupt
		| (0 << 15)			// 1 = enable TC interrupt
	;


	if (aistx)
	{
		regs->DESTADDR = (unsigned)(periphaddr);
		tmp |= (perid << 6); // set destination peripheral
		tmp |= (1 << 11); // mem -> per
	}
	else
	{
		regs->SRCADDR = (unsigned)(periphaddr);
		tmp |= (perid << 1); // set source peripheral
		tmp |= (2 << 11); // per -> mem
	}

	regs->LLI = 0;
	regs->CONFIG = tmp;
}

void THwDmaChannel_lpc::Disable()
{
	regs->CONFIG &= ~1;

	// todo: wait until it is disabled
}

void THwDmaChannel_lpc::Enable()
{
	regs->CONFIG |= 1;
}

bool THwDmaChannel_lpc::Active()
{
	return ((regs->CONFIG & 1) != 0); // same as enabled
}

bool THwDmaChannel_lpc::Enabled()
{
	return ((regs->CONFIG & 1) != 0);
}

void THwDmaChannel_lpc::PrepareTransfer(THwDmaTransfer * axfer)
{
	// compose the DMA control register

	unsigned count = (axfer->count & 0xFFF);

	unsigned ctrl = 0
		| (0 <<  0)  // TRANSFERSIZE(12)
		| (0 << 12)  // SBSIZE(3), 0 = 1x, 1 = 4x, 2 = 8x
		| (0 << 15)  // DBSIZE(3)
		| (0 << 18)  // SWIDTH(3), 0 = 8 bit, 1 = 16 bit, 2 = 32 bit
		| (0 << 21)  // DWIDTH(3), 0 = 8 bit, 1 = 16 bit, 2 = 32 bit
		| (0 << 24)  // S: Src AHB Bus,  0 = memory access only, 1 = per/mem access bus
		| (0 << 25)  // D: Dst AHB Bus,  0 = memory access only, 1 = per/mem access bus
		| (0 << 26)  // SI: Source Increment
		| (0 << 27)  // DI: Destination Increment
		| (0 << 31)  // TI: Terminal interrupt enable
	;

	unsigned unaligned = 0;
	if (istx)
	{
		regs->SRCADDR = unsigned(axfer->srcaddr);
		unaligned = ((axfer->bytewidth == 4) && ((unsigned(axfer->srcaddr) & 3) != 0));

		ctrl |= (1 << 25); // select peripheral bus for DST
		if ((axfer->flags & DMATR_NO_ADDR_INC) == 0)  ctrl |= (1 << 26); // increment source address
	}
	else
	{
		regs->DESTADDR = unsigned(axfer->dstaddr);
		unaligned = ((axfer->bytewidth == 4) && ((unsigned(axfer->dstaddr) & 3) != 0));

		ctrl |= (1 << 24); // select peripheral bus for SRC
		if ((axfer->flags & DMATR_NO_ADDR_INC) == 0)  ctrl |= (1 << 27); // increment DST address
	}

	if (unaligned)
	{
		// unaligned 4 byte tx, using 8 bit transfers with 4 x burst
		ctrl |= ((1 << 12) | (1 << 15));
		count = (count << 2);
	}
	else
	{
		// aligned tx
		if (axfer->bytewidth == 4)
		{
			ctrl |= ((2 << 18) | (2 << 21));
		}
		else if (axfer->bytewidth == 2)
		{
			ctrl |= ((1 << 18) | (1 << 21));
		}
		// (8 bit transfers set by default)
	}

	ctrl |= count;

	regs->CONTROL = ctrl;  // 0x9 48 0001
}

