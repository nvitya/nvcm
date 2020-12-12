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
 *  file:     hwdma_stm32_mxb.cpp
 *  brief:    3x DMA for the STM32H7 series: MDMA + (x)DMA + BDMA
 *  version:  1.00
 *  date:     2020-12-11
 *  authors:  nvitya
*/

#include <stdio.h>
#include <stdarg.h>

#include "hwdma.h"

#if defined(HWDMA_MXB)

const unsigned char stm32_dma_irq_status_shifts[8] = {0, 6, 16, 22, 0, 6, 16, 22};

bool THwDmaChannel_stm32::Init(int admanum, int achannel, int arequest)
{
	initialized = false;

	mregs = nullptr;
	xregs = nullptr;
	bregs = nullptr;

	int dma = (admanum & 0x07);
	int ch  = (achannel & 0x07);

	if (0 == dma)  // MDMA
	{
		ch = (achannel & 0x0F);
		RCC->AHB3ENR |= RCC_AHB3ENR_MDMAEN;
		mregs = (MDMA_Channel_TypeDef * )(MDMA_Channel0_BASE + ch * (MDMA_Channel1_BASE - MDMA_Channel0_BASE));
	}
	else if (1 == dma)  // xDMA1
	{
		RCC->AHB1ENR |= RCC_AHB1ENR_DMA1EN;
		xregs = (DMA_Stream_TypeDef * )(DMA1_Stream0_BASE + ch * (DMA1_Stream1_BASE - DMA1_Stream0_BASE));
	}
	else if (2 == dma) // xDMA2
	{
		RCC->AHB1ENR |= RCC_AHB1ENR_DMA2EN;
		xregs = (DMA_Stream_TypeDef * )(DMA2_Stream0_BASE + ch * (DMA2_Stream1_BASE - DMA2_Stream0_BASE));
	}
	else if (3 == dma) // BDMA
	{
		RCC->AHB4ENR |= RCC_AHB4ENR_BDMAEN;
		bregs = (BDMA_Channel_TypeDef * )(BDMA_Channel0_BASE + ch * (BDMA_Channel1_BASE - BDMA_Channel0_BASE));
	}
	else
	{
		return false;
	}

	dmanum = dma;
	chnum = ch;
	rqnum = arequest;
	//streamnum = st;

	if (xregs)
	{
		ndtreg = &xregs->NDTR;
		crreg = (__IO uint32_t *)&xregs->CR;

		DMA_TypeDef * dmaptr = (2 == dma ? DMA2 : DMA1);
		if (chnum > 3)
		{
			irqstreg = (uint32_t *)&dmaptr->HISR;
			irqstclrreg = (uint32_t *)&dmaptr->HIFCR;
		}
		else
		{
			irqstreg = (uint32_t *)&dmaptr->LISR;
			irqstclrreg = (uint32_t *)&dmaptr->LIFCR;
		}

		irqstshift = stm32_dma_irq_status_shifts[chnum];
		irqclrmask = (0xF << irqstshift);

		int muxch = (2 == dmanum ? 8 : 0) + chnum;
		DMAMUX1[muxch].CCR = 0
			| ((rqnum & 0x7F) <<  0)  // DMAREQ_ID(7): DMA request identification
			| (0 <<  8)  // SOIE: Synchronization overrun interrupt enable
			| (0 <<  9)  // EGE: Event generation enable
			| (0 << 16)  // SE: Synchronization enable
			| (0 << 17)  // SPOL(2): Synchronization polarity
			| (0 << 19)  // NBREQ(4): Number of DMA requests minus 1 to forward
			| (0 << 24)  // SYNC_ID(5): Synchronization identification
		;
	}
	else if (bregs)
	{
		ndtreg = &bregs->CNDTR;
		crreg = (__IO uint32_t *)&bregs->CCR;
		irqstshift = (chnum * 4);
		irqstreg = (uint32_t *)&BDMA->ISR;
		irqstclrreg = (uint32_t *)&BDMA->IFCR;
		irqclrmask = (0xF << irqstshift);

		DMAMUX2[chnum].CCR = 0
			| ((rqnum & 0x7F) <<  0)  // DMAREQ_ID(7): DMA request identification
			| (0 <<  8)  // SOIE: Synchronization overrun interrupt enable
			| (0 <<  9)  // EGE: Event generation enable
			| (0 << 16)  // SE: Synchronization enable
			| (0 << 17)  // SPOL(2): Synchronization polarity
			| (0 << 19)  // NBREQ(4): Number of DMA requests minus 1 to forward
			| (0 << 24)  // SYNC_ID(5): Synchronization identification
		;
	}
	else if (mregs)
	{
		ndtreg = &mregs->CBNDTR;
		crreg = (__IO uint32_t *)&mregs->CCR;
		irqstreg = (uint32_t *)&mregs->CISR;
		irqstclrreg = (uint32_t *)&mregs->CIFCR;
		irqclrmask = 0x1F;
	}

	Prepare(true, nullptr, 0); // set some defaults

	initialized = true;

	return true;
}

void THwDmaChannel_stm32::Prepare(bool aistx, void * aperiphaddr, unsigned aflags)
{
	istx = aistx;
	periphaddr = aperiphaddr;
}

void THwDmaChannel_stm32::Disable()
{
	*crreg &= ~1;

	// wait until it is disabled
	while (*crreg & 1)
	{
		// wait
	}

	*ndtreg = 0;
}

void THwDmaChannel_stm32::Enable()
{
	// start the channel
	*crreg |= 1;
}

static unsigned get_busid_by_address(void * aaddr)
{
	uint32_t addr = (uint32_t)aaddr;
	if (addr < 0x24000000)
	{
		return 1;
	}

	return 0;
}

void THwDmaChannel_stm32::PrepareTransfer(THwDmaTransfer * axfer)
{
  Disable();  // this is important here
	ClearIrqFlag();

	int sizecode = 0;
	if (axfer->bytewidth == 2)  sizecode = 1;
	else if (axfer->bytewidth == 4)  sizecode = 2;
	else if (axfer->bytewidth == 8)  sizecode = 3;

	int dircode;
	if (axfer->flags & DMATR_MEM_TO_MEM)
	{
		dircode = 2;
	}
	else if (istx)
	{
		dircode = 1;
	}
	else
	{
		dircode = 0;
	}

	int meminc = (axfer->flags & DMATR_NO_ADDR_INC ? 0 : 1);

	uint32_t circ = (axfer->flags & DMATR_CIRCULAR ? 1 : 0);
	uint32_t inte = (axfer->flags & DMATR_IRQ ? 1 : 0);


	if (mregs) // MDMA
	{
		unsigned sbus = 0;
		unsigned dbus = 0;

		unsigned sinc = 0;
		unsigned dinc = 0;

		if (axfer->flags & DMATR_MEM_TO_MEM)
		{
			// DIR=0:
			mregs->CSAR = (uint32_t)axfer->srcaddr;
			mregs->CDAR = (uint32_t)axfer->dstaddr;
			sbus = get_busid_by_address(axfer->srcaddr);
			dbus = get_busid_by_address(axfer->dstaddr);
			sinc = 2;
			dinc = 2;
		}
		else if (istx)
		{
			mregs->CSAR = (uint32_t)axfer->srcaddr;
			mregs->CDAR = (uint32_t)periphaddr;
			sbus = get_busid_by_address(axfer->srcaddr);
			sinc = (meminc << 1);
		}
		else
		{
			mregs->CSAR = (uint32_t)periphaddr;
			mregs->CDAR = (uint32_t)axfer->dstaddr;
			dbus = get_busid_by_address(axfer->dstaddr);
			dinc = (meminc << 1);
		}

		// channel configuration
		mregs->CCR = 0
			| (0        << 16)  // SWRQ: Software request
			| (0        << 14)  // WEX: 1 = exchange the 32 bits in every 64 bit
			| (0        << 13)  // HEX: 1 = exchange the 16 bits in every 32 bit
			| (0        << 12)  // BEX: 1 = exchange bytes in every 16 bit
			| ((priority & 3) << 6) // PL(2): priority level
			| (inte     <<  2)  // CTCIE: Channel Transfer complete interrupt enable
			| (0        <<  0)  // EN: keep not enabled
		;

		unsigned tlen = ((1 << sizecode) - 1);

		// transfer configuration
		mregs->CTCR = 0
			| (0        << 31)  // BWM: 0 = the destination is not bufferable
			| (0        << 30)  // SWRM: 1 = HW requests ignored
			| (0        << 28)  // TRGM(2): Trigger Mode, 0 = one buffer transfer per trigger
			| (0        << 26)  // PAM(2): 0 = right aligned
			| (0        << 25)  // PKE: PAck Enable
			| (tlen     << 18)  // TLEN(7): Buffer Transfer Length (number of bytes - 1)
			| (0        << 15)  // DBURST(3): destination burst, 0 = single
			| (0        << 12)  // SBURST(3): source burst, 0 = single
			| (sizecode << 10)  // DINCOS(2): dest. increment offset, 0=8bit, 1=16bit, 2=32bit, 3=64bit
			| (sizecode <<  8)  // SINCOS(2): src. increment offset, 0=8bit, 1=16bit, 2=32bit, 3=64bit
			| (sizecode <<  6)  // DSIZE(2)
			| (sizecode <<  4)  // SSIZE(2)
			| (dinc     <<  2)  // DINC(2): 0 = no dst. increment, 2 = +DINCOS, 3 = -DINCOS
			| (sinc     <<  0)  // SINC(2): 0 = no src. increment, 2 = +SINCOS, 3 = -SINCOS
		;

		mregs->CTBR = 0
			| (sbus  << 17) // DBUS: destination bus, 0 = system/AXI, 1 = AHB/TCM
			| (sbus  << 16) // SBUS: source bus, 0 = system/AXI, 1 = AHB/TCM
			| (rqnum <<  0) // TSEL(6): Trigger Select
		;

		mregs->CBNDTR = 0
			| (0 << 20) // BRC(12): block repeat count
			| (uint32_t)axfer->count  // no block repeat stuff here
		;

		mregs->CBRUR = 0;

		mregs->CLAR = 0;
		mregs->CMAR = 0;
		mregs->CMDR = 0;
	}
	else
	{
		if (xregs)
		{
			xregs->CR = 0
				| (0  << 23)        // MBURST(2): memory burst, 0 = single transfer
				| (0  << 21)        // PBURST(2): peripheral burst
				| (0  << 19)        // CT: current target (for double buffer mode)
				| (0  << 18)        // DBM: double buffer mode
				| ((priority & 3) << 16) // PL(2): priority level
				| (0  << 15)        // PINCOS: peripheral increment offset
				| (sizecode << 13)  // MSIZE(2): Memory data size, 8 bit
				| (sizecode << 11)  // PSIZE(2): Periph data size, 8 bit
				| (meminc   << 10)  // MINC: Memory increment mode
				| (0  <<  9)        // PINC: Peripheral increment mode
				| (circ <<  8)      // CIRC: Circular mode
				| (dircode <<  6)   // DIR(2): Data transfer direction
				| (0  <<  5)        // PFCTRL: Peripheral flow controller, 0 = DMA is the flow controller
				| (inte << 4)       // TCIE: TCIE: Transfer complete interrupt enable
				| (0  <<  1)        // (3): error interrupts
				| (0  <<  0)        // EN: keep not enabled
			;

			if (axfer->flags & DMATR_MEM_TO_MEM)
			{
				xregs->PAR = (uint32_t)axfer->srcaddr;
				xregs->M0AR = (uint32_t)axfer->dstaddr;
			}
			else if (istx)
			{
				xregs->PAR = (uint32_t)periphaddr;
				xregs->M0AR = (uint32_t)axfer->srcaddr;
			}
			else
			{
				xregs->PAR = (uint32_t)periphaddr;
				xregs->M0AR = (uint32_t)axfer->dstaddr;
			}

			xregs->NDTR = (uint32_t)axfer->count;
		}
		else if (bregs)
		{
			bregs->CCR = 0
				| (0  << 16)        // CT: current target
				| (0  << 15)        // DBM: double buffer mode
				| ((priority & 3) << 12) // PL(2): priority level
				| (sizecode << 10)  // MSIZE(2): Memory data size, 0 = 8 bit
				| (sizecode <<  8)  // PSIZE(2): Periph data size, 0 = 8 bit
				| (meminc   <<  7)  // MINC: Memory increment mode
				| (0        <<  6)  // PINC: Peripheral increment mode
				| (circ     <<  5)  // CIRC: Circular mode
				| ((dircode & 1) <<  4)  // DIR: Data transfer direction, 0 = per->mem, 1 = mem->per
				| (inte     <<  1)  // TCIE: TCIE: Transfer complete interrupt enable
				| (0        <<  0)  // EN: keep not enabled
			;

			if (axfer->flags & DMATR_MEM_TO_MEM)
			{
				// DIR=0:
				bregs->CM0AR = (uint32_t)axfer->dstaddr;
				bregs->CM1AR = (uint32_t)axfer->srcaddr;
			}
			else if (istx)
			{
				bregs->CPAR = (uint32_t)periphaddr;
				bregs->CM0AR = (uint32_t)axfer->srcaddr;
				bregs->CM1AR = (uint32_t)axfer->srcaddr;
			}
			else
			{
				bregs->CPAR = (uint32_t)periphaddr;
				bregs->CM0AR = (uint32_t)axfer->dstaddr;
				bregs->CM1AR = (uint32_t)axfer->dstaddr;
			}

			bregs->CNDTR = (uint32_t)axfer->count;
		}
	}
}

#endif // if HWDMA_IMPLEMENTED
