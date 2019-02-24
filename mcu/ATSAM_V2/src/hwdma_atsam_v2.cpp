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
 *  file:     hwdma_atsam_v2.h
 *  brief:    ATSAM V2 DMA
 *  version:  1.00
 *  date:     2019-01-18
 *  authors:  nvitya
*/

#include <stdio.h>
#include <stdarg.h>

#include "hwdma.h"
#include "atsam_v2_utils.h"

__attribute__((aligned(16)))
TDmaChannelDesc  dma_channel_descriptors[MAX_DMA_CHANNELS];

__attribute__((aligned(16)))
TDmaChannelDesc  dma_channel_wb_descriptors[MAX_DMA_CHANNELS];

bool dma_global_initialized = false;

#if 1

bool THwDmaChannel_atsam_v2::Init(int achnum, int aperid)  // perid = peripheral request id
{
	initialized = false;

	// DMA clock is enabled by default

  ctrlregs = DMAC;  // for faster addressing

  if (!dma_global_initialized)
  {
  	// global dma initialization

		ctrlregs->CTRL.bit.DMAENABLE = false; // disable first
		ctrlregs->CTRL.bit.SWRST = true;
		while (ctrlregs->CTRL.bit.SWRST) { }

		// use the same address
		ctrlregs->BASEADDR.reg = (uint32_t)&dma_channel_descriptors[0];
		ctrlregs->WRBADDR.reg  = (uint32_t)&dma_channel_wb_descriptors[0];

		ctrlregs->DBGCTRL.bit.DBGRUN = 1;

		ctrlregs->CTRL.bit.LVLEN0 = 1;
		ctrlregs->CTRL.bit.LVLEN1 = 1;
		ctrlregs->CTRL.bit.LVLEN2 = 1;
		ctrlregs->CTRL.bit.LVLEN3 = 1;

		//ctrlregs->PRICTRL0.reg = 0x84838281;

		ctrlregs->CTRL.bit.DMAENABLE = true;

		dma_global_initialized = true;
  }

  // channel init

	chnum = achnum;
	perid = aperid;

	if ((chnum < 0) || (chnum >= MAX_DMA_CHANNELS))
	{
		chnum = -1;
		return false;
	}

	chbit = (1 << chnum);
  regs = &dma_channel_descriptors[chnum];
  wbregs = &dma_channel_wb_descriptors[chnum];

  uint32_t trigact = 2; // TRIGACT(2): 0 = block, 2 = beat, 3 = transaction
  if (perid == 0)	trigact = 3; // for sw triggers run the whole transaction through

#ifdef DMAC_CHID_OFFSET

  ctrlregs->CHID.reg = chnum;  // select the channel

  ctrlregs->CHCTRLA.bit.ENABLE = 0; // disable
  ctrlregs->CHCTRLA.bit.RUNSTDBY = 1;

  ctrlregs->CHCTRLB.reg = 0
  	| (0 << 24) // CMD(2)
  	| (trigact << 22) // TRIGACT(2): 0 = block, 2 = beat, 3 = transaction
  	| (perid   <<  8) // TRIGSRC(6): 0 = SW only
  	| (0 <<  5) // LVL(2):
  	| (0 <<  4) // EVOE: 1 = event output enable
  	| (0 <<  3) // EVIE: 1 = event input enable
  	| (0 <<  0) // EVACT(3):
  ;

  ctrlregs->CHINTENCLR.reg = 0x7;

#else

  chregs = &(ctrlregs->Channel[chnum]);

  chregs->CHCTRLA.bit.ENABLE = 0; // disable

  chregs->CHCTRLA.reg = 0
  	| (0 << 28)  // THESHOLD(2): 0 = 1 beat
  	| (0 << 24)  // BURSTLEN(4): 0 = single
  	| (trigact << 20)  // TRIGACT(2): 0 = block, 2 = burst/beat, 3 = transaction
  	| (perid   <<  8)  // TRIGSRC(8):
  	| (1 <<  6)  // RUNSTDBY:
  	| (0 <<  1)  // ENABLE:
  	| (0 <<  0)  // SWRST:
  ;

  chregs->CHPRILVL.reg = priority;
  chregs->CHEVCTRL.reg = 0; // disable event generation

  chregs->CHINTENCLR.reg = 0x7;

#endif

  // initialize the descriptor

  regs->BTCTRL = 0;   // invalidate the channel descriptor
  regs->DESCADDR = 0; // terminate the chain here

	initialized = true;

	return true;
}

void THwDmaChannel_atsam_v2::Prepare(bool aistx, void * aperiphaddr, unsigned aflags)
{
	istx = aistx;
	periphaddr = aperiphaddr;
}

#ifdef DMAC_CHID_OFFSET

void THwDmaChannel_atsam_v2::Disable()
{
	// TODO: disable interrupts
  ctrlregs->CHID.reg = chnum;  // select the channel
  ctrlregs->CHCTRLA.bit.ENABLE = 0; // disable
}

void THwDmaChannel_atsam_v2::Enable()
{
	// TODO: disable interrupts
  ctrlregs->CHID.reg = chnum;  // select the channel
  ctrlregs->CHCTRLA.bit.ENABLE = 1;

  if (perid == 0)  ctrlregs->SWTRIGCTRL.reg |= chbit; // software trigger
}

#else

void THwDmaChannel_atsam_v2::Enable()
{
	chregs->CHCTRLA.bit.ENABLE = 1;
  if (perid == 0) 	ctrlregs->SWTRIGCTRL.reg |= chbit; // software trigger
}

#endif

void THwDmaChannel_atsam_v2::PrepareTransfer(THwDmaTransfer * axfer)
{
	uint16_t ccreg = 0
	  | (0 << 13)  // STEPSIZE(3): 0 = beat size
	  | (0 << 12)  // STEPSEL: 0 = dst, 1 = src
	  | (0 << 11)  // DSTINC: 1 = increment dst address
	  | (0 << 10)  // SRCINC:	1 = increment src address
	  | (0 <<  8)  // BEATSIZE(2): 0 = 8 bit, 1 = 16 bit, 2 = 32 bit
	  | (0 <<  3)  // BLOCKACT(2): 0 = disable channel, 1 = disable + int, 2 = suspend, 3 = suspend + int
	  | (0 <<  1)  //	EVOSEL(2): 0 = event generation disabled
	  | (1 <<  0)  // VALID: 1 = descriptor valid
	;

	if      (axfer->bytewidth == 4)  ccreg |= (2 << 8);
	else if (axfer->bytewidth == 2)  ccreg |= (1 << 8);

	// you have to set the last address for the incremented ones !
	uint32_t addrplus = (axfer->bytewidth * axfer->count);
	uint32_t addrplus2 = addrplus;

	if (axfer->flags & DMATR_MEM_TO_MEM)
	{
		if ((axfer->flags & DMATR_NO_SRC_INC) == 0)
		{
			ccreg |= (1 << 10);
		}
		else
		{
			addrplus = 0;
		}

		if ((axfer->flags & DMATR_NO_DST_INC) == 0)
		{
			ccreg |= (1 << 11);
		}
		else
		{
			addrplus2 = 0;
		}

		regs->SRCADDR = ((uint32_t)axfer->srcaddr) + addrplus;
		regs->DSTADDR = ((uint32_t)axfer->dstaddr) + addrplus2;
	}
	else
	{
		// PER <-> MEM transfer

		if (istx)
		{
			ccreg |= (1 << 12);  // STEPSEL

			if ((axfer->flags & DMATR_NO_SRC_INC) == 0)
			{
				ccreg |= (1 << 10);
			}
			else
			{
				addrplus = 0;
			}

			regs->SRCADDR = ((uint32_t)axfer->srcaddr) + addrplus;
			regs->DSTADDR = (uint32_t)periphaddr;
		}
		else
		{
			if ((axfer->flags & DMATR_NO_DST_INC) == 0)
			{
				ccreg |= (1 << 11);
			}
			else
			{
				addrplus = 0;
			}

			regs->SRCADDR = (uint32_t)periphaddr;
			regs->DSTADDR = ((uint32_t)axfer->dstaddr) + addrplus;
		}
	}

	regs->BTCNT = axfer->count;
	regs->BTCTRL = ccreg;

	if (axfer->flags & DMATR_IRQ)
	{
		chregs->CHINTENSET.reg = (1 << 1); // enable transfer complete irq
	}
	else
	{
		chregs->CHINTENSET.reg = (1 << 1); // disable transfer complete irq
	}
}

#endif
