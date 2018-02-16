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
 *  file:     hwdma_lpc_v3.cpp
 *  brief:    LPC_V3 DMA
 *  version:  1.00
 *  date:     2018-02-10
 *  authors:  nvitya
*/

#include "hwpins.h"
#include "hwdma.h"

#include "traces.h"

TDmaDesc  dmachtable[32] __attribute__((aligned(512)));  // must be aligned to 512 byte boundary !

bool THwDmaChannel_lpc_v3::Init(int achnum)  // chnum: 0..29
{
	initialized = false;

	SYSCON->AHBCLKCTRLSET[0] = (1 << 20);  // Enable DMA Clock
	DMA0->CTRL |= 1; // enable the DMA

	// check if DMA table is initialized
	if (HW_DMA->SRAMBASE != (uint32_t)&dmachtable[0])
	{
		HW_DMA->SRAMBASE = (uint32_t)&dmachtable[0];
	}

	if ((achnum < 0) || (achnum > 29))
	{
		return false;
	}

	chnum = achnum;
	chbit = (1 << chnum);
	regs = &(HW_DMA->CHANNELS[achnum]);

	firstdesc = &dmachtable[chnum];

	Prepare(true, nullptr, 0); // set some defaults

	initialized = true;

	return true;
}

void THwDmaChannel_lpc_v3::Prepare(bool aistx, void * aperiphaddr, unsigned aflags)
{
	Disable();

	istx = aistx;
	periphaddr = aperiphaddr;

	regs->CFG = 0
		| (1 << 0)    // 1 = Enable Periph. Request
		| (0 << 1)    // 1 = Enable Hardware Trigger
		| ((priority & 3) << 16)   // (3) Channel priority (0 = highest)
	;

	regs->XFERCFG = 0; // empty, invalid config

	Enable();
}

bool THwDmaChannel_lpc_v3::StartTransfer(THwDmaTransfer * axfer)
{
	Disable();

	register unsigned cntm1 = axfer->count - 1;
	register unsigned bytewidth = axfer->bytewidth;

	register unsigned xfercfg = 0
		| (1 << 0)    // CFGVALID
		| (0 << 1)    // RELOAD
		| (1 << 2)    // SWTRIG		 !!!
		| (0 << 3)    // CLRTRIG
		| (0 << 4)    // SETINTA
		| (0 << 5)    // SETINTB

		| (0 <<  8)    // WIDTH(2)
		| (0 << 12)    // SRCINC(2)
		| (0 << 14)    // DSTINC(2)

		| (cntm1 << 16)    // XFERCOUNT(9): transfer count - 1 !!!!
	;

	if (bytewidth == 4)
	{
		xfercfg |= (2 << 8);
	}
	else if (bytewidth == 2)
	{
		xfercfg |= (1 << 8);
	}

	if (istx)
	{
		xfercfg |= (1 << 12); // SRC increment with the given width

		firstdesc->SRCEND = (char *)(unsigned(axfer->srcaddr) + cntm1 * bytewidth);
		firstdesc->DSTEND = periphaddr;
	}
	else
	{
		xfercfg |= (1 << 14); // DST increment with the given width

		firstdesc->SRCEND = periphaddr;
		firstdesc->DSTEND = (char *)(unsigned(axfer->dstaddr) + cntm1 * bytewidth);
	}

	firstdesc->NEXT = nullptr;

	//regs->CTLSTAT = 0;
	regs->XFERCFG = xfercfg;

	Enable();

	return true;
}
