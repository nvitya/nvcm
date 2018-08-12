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
 *  file:     hwdma_atsam.cpp
 *  brief:    ATSAM DMA
 *  version:  1.00
 *  date:     2018-02-10
 *  authors:  nvitya
*/

#include <stdio.h>
#include <stdarg.h>

#include "hwdma.h"

#if defined(HW_DMA_ALT_REGS)

bool THwDmaChannel_atsam::InitPeriphDma(bool aistx, void * aregs, void * aaltregs)  // special function for Atmel PDMA
{
	initialized = false;

	if (aregs)
	{
		altregs = (HW_DMA_ALT_REGS *)(unsigned(aregs) + 0x100);
	}
	else if (aaltregs)
	{
		altregs = (HW_DMA_ALT_REGS *)(unsigned(aaltregs) + 0x100);
	}
	else
	{
		return false;
	}

	istx = aistx;

	initialized = true;

	return true;
}

void THwDmaChannel_atsam::Disable()
{
	if (istx)
	{
		altregs->PTCR = 0x200;
	}
	else
	{
		altregs->PTCR = 0x002;
	}
}

void THwDmaChannel_atsam::Enable()
{
	// start the channel
	if (istx)
	{
		altregs->PTCR = 0x100;
	}
	else
	{
		altregs->PTCR = 0x001;
	}
}

bool THwDmaChannel_atsam::Enabled()
{
	if (istx)
	{
		return (altregs->TCR > 0);
		//return ((altregs->PTSR & 0x100) != 0);
	}
	else
	{
		return (altregs->RCR > 0);
		//return ((altregs->PTSR & 0x001) != 0);
	}
}

void THwDmaChannel_atsam::PrepareTransfer(THwDmaTransfer * axfer)
{
	if (istx)
	{
		altregs->TPR = (uint32_t)axfer->srcaddr;
		altregs->TCR = axfer->count;
		altregs->TNCR = 0;
		altregs->TNPR = 0;
	}
	else
	{
		altregs->RPR = (uint32_t)axfer->dstaddr;
		altregs->RCR = axfer->count;
		altregs->RNCR = 0;
		altregs->RNPR = 0;
	}
}

#endif

void THwDmaChannel_atsam::Prepare(bool aistx, void * aperiphaddr, unsigned aflags)
{
	istx = aistx;
	periphaddr = aperiphaddr;
}

#ifdef XDMAC

#define MAX_DMA_CHANNELS  XDMACCHID_NUMBER

bool THwDmaChannel_atsam::Init(int achnum, int aperid)  // perid = peripheral request id
{
	initialized = false;

	PMC->PMC_PCER1 = (1 << 26); // enable XDMAC

	chnum = achnum;
	perid = aperid;

	if ((chnum < 0) || (chnum >= MAX_DMA_CHANNELS))
	{
		chnum = -1;
		return false;
	}

	chbit = (1 << chnum);
  regs = (HW_DMA_REGS *)&(XDMAC->XDMAC_CHID[chnum]);

	regs->XDMAC_CNDC = 0; // disable next descriptor fetch
	regs->XDMAC_CBC = 0;
	regs->XDMAC_CDS_MSP = 0;
	regs->XDMAC_CSUS = 0;
	regs->XDMAC_CDUS = 0;

	initialized = true;

	return true;
}

void THwDmaChannel_atsam::Disable()
{
	XDMAC->XDMAC_GD = chbit;

	// wait until it is disabled
	while (XDMAC->XDMAC_GS & chbit)
	{
		// wait
	}
}

void THwDmaChannel_atsam::PrepareTransfer(THwDmaTransfer * axfer)
{
	// prepare all bits with 0
	uint32_t ccreg = 0
			| (0  <<  0)  // TYPE: 1 = Peripheral - Memory transfer, 0 = MEM2MEM
			| (0  <<  1)  // MBSIZE(2): burst size
			| (0  <<  4)  // DSYNC
			| (0  <<  6)  // SWREQ
			| (0  <<  7)  // MEMSET, 1 = zero memory
			| (0  <<  8)  // CSIZE(3): chunk size
			| (0  << 11)  // DWIDTH(2): data width, 0 = 1 byte by default
			| (0  << 13)  // SIF: 1 = APB/RAM, 0 = CCM/RAM
			| (0  << 14)  // DIF: 1 = APB/RAM, 0 = CCM/RAM
			| (0  << 16)  // SAM(2)
			| (0  << 18)  // DAM(2)
			| (perid << 24)
  ;

	if      (axfer->bytewidth == 4)  ccreg |= (2 << 11);
	else if (axfer->bytewidth == 2)  ccreg |= (1 << 11);

	if (axfer->flags & DMATR_MEM_TO_MEM)
	{
		ccreg |= (0
			| (1 <<  0)   // TYPE: 1 = Peripheral - Memory transfer
		  | (1 <<  4)   // DSYNC = 1
			| (1 <<  6)   // SWREQ = 1 to start without HW signal
		);
		ccreg |= (1 << 16); // SAM = 1
		ccreg |= (1 << 18); // DAM = 1

	  // auto detect interface based on the address
		unsigned addr;
		addr = (uint32_t)axfer->srcaddr;
		regs->XDMAC_CSA = addr;
		if (((addr >> 20) != 0x200) && ((addr >> 20) != 0x000))  ccreg |= (1 << 13); // SIF = 1
		addr = (uint32_t)axfer->dstaddr;
		regs->XDMAC_CDA = addr;
		if (((addr >> 20) != 0x200) && ((addr >> 20) != 0x000))  ccreg |= (1 << 14); // DIF = 1
	}
	else
	{
		// PER <-> MEM transfer

		if (istx)
		{
			ccreg |= (0
				| (1  <<  0)  // TYPE: 1 = Peripheral - Memory transfer
			  | (1 << 4)    // DSYNC = 1;
			  | (1 << 14)   // DIF = 1, IF1 required for peripheral bus access
			  | (0 << 13)   // SIF = 0
			);

			if ((axfer->flags & DMATR_NO_ADDR_INC) == 0)
			{
				ccreg |= (3 << 16); // SAM == 3
			}
			regs->XDMAC_CSA = (uint32_t)axfer->srcaddr;
			regs->XDMAC_CDA = (uint32_t)periphaddr;
		}
		else
		{
			ccreg |= (0
  			| (1 <<  0)  // TYPE: 1 = Peripheral - Memory transfer
			  | (0 <<  4)  // DSYNC = 0;
			  | (0 << 14)  // DIF = 0
			  | (1 << 13)  // SIF = 1, IF1 required for peripheral bus access
			);

			if ((axfer->flags & DMATR_NO_ADDR_INC) == 0)
			{
				ccreg |= (3 << 18); // DAM == 3
			}
			regs->XDMAC_CSA = (uint32_t)periphaddr;
			regs->XDMAC_CDA = (uint32_t)axfer->dstaddr;
		}
	}

	//regs->XDMAC_CBC = 0; // already set to 0 at the Init()

	regs->XDMAC_CUBC = axfer->count;
	regs->XDMAC_CC = ccreg;
}

#endif
