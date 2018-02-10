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

	return true;
}

#endif

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

void THwDmaChannel_atsam::Prepare(bool aistx, void * aperiphaddr, unsigned aflags)
{
	istx = aistx;
	periphaddr = aperiphaddr;
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

void THwDmaChannel_atsam::Enable()
{
	// start the channel
	XDMAC->XDMAC_GE = chbit;
}

bool THwDmaChannel_atsam::Enabled()
{
	return ((XDMAC->XDMAC_GS & chbit) != 0);
}

bool THwDmaChannel_atsam::StartTransfer(THwDmaTransfer * axfer)
{
	unsigned sizecode = 0;
	if (axfer->bytewidth == 2)  sizecode = 1;
	else if (axfer->bytewidth == 4)  sizecode = 2;

	unsigned dsync, sam, dam, sif, dif;

	if (istx)
	{
		dsync = 1;
		sam = (axfer->addrinc ? 3 : 0);
		dam = 0;
		sif = 0;
		dif = 1;  // IF1 required for peripheral bus access
		regs->XDMAC_CSA = (uint32_t)axfer->srcaddr;
		regs->XDMAC_CDA = (uint32_t)periphaddr;
	}
	else
	{
		dsync = 0;
		sam = 0;
		dam = (axfer->addrinc ? 3 : 0);
		sif = 1;  // IF1 required for peripheral bus access
		dif = 0;
		regs->XDMAC_CSA = (uint32_t)periphaddr;
		regs->XDMAC_CDA = (uint32_t)axfer->dstaddr;
	}

	regs->XDMAC_CUBC = axfer->count;
	regs->XDMAC_CBC = 0;

	regs->XDMAC_CC = 0
		| (1  <<  0)  // TYPE: 1 = Peripheral - Memory transfer, 0 = MEM2MEM
		| (0  <<  1)  // MBSIZE(2): burst size
		| (dsync <<  4)  // DSYNC
		| (0  <<  6)  // SWREQ
		| (0  <<  7)  // MEMSET, 1 = zero memory
		| (0  <<  8)  // CSIZE(3): chunk size
		| (sizecode << 11)  // DWIDTH(2): data width
		| (sif  << 13)  // SIF: 1 = APB/RAM, 0 = CCM/RAM
		| (dif  << 14)  // DIF: 1 = APB/RAM, 0 = CCM/RAM
		| (sam  << 16)  // SAM(2)
		| (dam  << 18)  // DAM(2)
		| (perid << 24)
	;

	Enable();

	return true;
}

bool THwDmaChannel_atsam::StartMemToMem(THwDmaTransfer * axfer)
{
	int sizecode = 0;
	if (axfer->bytewidth == 2)  sizecode = 1;
	else if (axfer->bytewidth == 4)  sizecode = 2;

	// auto detect interface based on the address
	unsigned sif, dif;
	unsigned addr;
	addr = (uint32_t)axfer->srcaddr;
	regs->XDMAC_CSA = addr;
	if (((addr >> 20) == 0x200) || ((addr >> 20) == 0x000))  sif = 0;  else sif = 1;
	addr = (uint32_t)axfer->dstaddr;
	regs->XDMAC_CDA = addr;
	if (((addr >> 20) == 0x200) || ((addr >> 20) == 0x000))  dif = 0;  else dif = 1;

	regs->XDMAC_CUBC = axfer->count;

	regs->XDMAC_CC = 0
		| (0  <<  0)  // TYPE: 1 = Peripheral - Memory transfer, 0 = MEM2MEM
		| (0  <<  1)  // MBSIZE(2): burst size
		| (0  <<  4)  // DSYNC
		| (1  <<  6)  // SWREQ
		| (0  <<  7)  // MEMSET, 1 = zero memory
		| (0  <<  8)  // CSIZE(3): chunk size
		| (sizecode << 11)  // DWIDTH(2): data width
		| (sif << 13)  // SIF: 1 = APB/RAM, 0 = CCM/RAM
		| (dif << 14)  // DIF: 1 = APB/RAM, 0 = CCM/RAM
		| (1  << 16)  // SAM(2)
		| (1  << 18)  // DAM(2)
	;

	Enable();  // start the transfer

	return true;
}

#endif
