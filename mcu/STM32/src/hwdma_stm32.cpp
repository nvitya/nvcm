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
 *  file:     hwdma_stm32.cpp
 *  brief:    STM32 DMA
 *  version:  1.00
 *  date:     2018-02-10
 *  authors:  nvitya
*/

#include <stdio.h>
#include <stdarg.h>

#include "hwdma.h"

#if HWDMA_IMPLEMENTED

#if defined(MCUSF_F1) || defined(MCUSF_F0) || defined(MCUSF_L0) || defined(MCUSF_F3) || defined(MCUSF_G4)

#ifdef RCC_AHB1ENR_DMA1EN
  #define HWDMA_EN_REGISTER  RCC->AHB1ENR
  #define RCC_AHBENR_DMA1EN  RCC_AHB1ENR_DMA1EN
  #define RCC_AHBENR_DMA2EN  RCC_AHB1ENR_DMA2EN
#else
  #define HWDMA_EN_REGISTER  RCC->AHBENR
#endif

bool THwDmaChannel_stm32::Init(int admanum, int achannel, int arequest)  // admanum = 1..2, achannel = 1..8
{
	initialized = false;

	if ((achannel < 1) || (achannel > 8)) // check if the channel number is 1 based
	{
		__BKPT(); // invalid channel number
		return false;
	}

	dmanum   = (admanum  & 0x03);
	int      chid    = ((achannel-1) & 0x07);

	if (1 == dmanum)
	{
		HWDMA_EN_REGISTER |= RCC_AHBENR_DMA1EN;
		regs = (HW_DMA_REGS * )(DMA1_Channel1_BASE + chid * (DMA1_Channel2_BASE - DMA1_Channel1_BASE));
	}
#ifdef RCC_AHBENR_DMA2EN
	else if (2 == dmanum)
	{
		HWDMA_EN_REGISTER |= RCC_AHBENR_DMA2EN;
		regs = (HW_DMA_REGS * )(DMA2_Channel1_BASE + chid * (DMA2_Channel2_BASE - DMA2_Channel1_BASE));
	}
#endif
	else
	{
		return false;
	}

	chnum = achannel;

	DMA_TypeDef * dmaptr = DMA1;
#ifdef DMA2
	if (2 == dmanum)  dmaptr = DMA2;
#endif
	irqstreg = (unsigned *)&dmaptr->ISR;
	irqstclrreg = (unsigned *)&dmaptr->IFCR;
	crreg = (unsigned *)&regs->CCR;

	irqstshift = 4 * chid;

#ifdef DMA1_CSELR
	unsigned rqnum = (arequest & 0x0F);
	unsigned csr = DMA1_CSELR->CSELR;
	unsigned shnum = 4 * chid;
	csr &= ~(0xF << shnum);
	csr |=  (rqnum << shnum);
	DMA1_CSELR->CSELR = csr;
#endif

#ifdef RCC_AHB1ENR_DMAMUX1EN
  RCC->AHB1ENR |= RCC_AHB1ENR_DMAMUX1EN;
  int muxch = (2 == dmanum ? 8 : 0) + chid;
  DMAMUX1[muxch].CCR = 0
    | ((arequest & 0x7F) <<  0)  // DMAREQ_ID(7): DMA request identification
    | (0 <<  8)  // SOIE: Synchronization overrun interrupt enable
    | (0 <<  9)  // EGE: Event generation enable
    | (0 << 16)  // SE: Synchronization enable
    | (0 << 17)  // SPOL(2): Synchronization polarity
    | (0 << 19)  // NBREQ(4): Number of DMA requests minus 1 to forward
    | (0 << 24)  // SYNC_ID(5): Synchronization identification
  ;
#endif

	Prepare(true, nullptr, 0); // set some defaults

	initialized = true;

	return true;
}

#else

#define DMASTREAMS

const unsigned char stm32_dma_irq_status_shifts[8] = {0, 6, 16, 22, 0, 6, 16, 22};

bool THwDmaChannel_stm32::Init(int admanum, int astream, int achannel)
{
	initialized = false;

	int dma = (admanum & 0x03);
	int st  = (astream  & 0x07);
	int ch  = (achannel  & 0x07);

	if (st > 7)  return false;
	if (ch > 7)  return false;
	if (1 == dma)
	{
		RCC->AHB1ENR |= RCC_AHB1ENR_DMA1EN;
		regs = (HW_DMA_REGS * )(DMA1_Stream0_BASE + st * (DMA1_Stream1_BASE - DMA1_Stream0_BASE));
	}
	else if (2 == dma)
	{
		RCC->AHB1ENR |= RCC_AHB1ENR_DMA2EN;
		regs = (HW_DMA_REGS * )(DMA2_Stream0_BASE + st * (DMA2_Stream1_BASE - DMA2_Stream0_BASE));
	}
	else
	{
		return false;
	}

	streamnum = st;
	chnum = ch;
	crreg = (__IO unsigned *)&regs->CR;

	DMA_TypeDef * dmaptr = (2 == dma ? DMA2 : DMA1);
	if (st > 3)
	{
		irqstreg = (unsigned *)&dmaptr->HISR;
		irqstclrreg = (unsigned *)&dmaptr->HIFCR;
	}
	else
	{
		irqstreg = (unsigned *)&dmaptr->LISR;
		irqstclrreg = (unsigned *)&dmaptr->LIFCR;
	}

	irqstshift = stm32_dma_irq_status_shifts[st];

	Prepare(true, nullptr, 0); // set some defaults

	initialized = true;

	return true;
}

#endif

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

#ifndef DMASTREAMS
	regs->CNDTR = 0; // this is required (at least on the F303) to properly stop the channel after broken transaction
#else
	regs->NDTR = 0;
#endif
}

void THwDmaChannel_stm32::Enable()
{
	// start the channel
	*crreg |= 1;
}

void THwDmaChannel_stm32::PrepareTransfer(THwDmaTransfer * axfer)
{
  Disable();  // this is important here
	ClearIrqFlag();

	int sizecode = 0;
	if (axfer->bytewidth == 2)  sizecode = 1;
	else if (axfer->bytewidth == 4)  sizecode = 2;

	int meminc = (axfer->flags & DMATR_NO_ADDR_INC ? 0 : 1);

	uint32_t circ = (axfer->flags & DMATR_CIRCULAR ? 1 : 0);
	uint32_t inte = (axfer->flags & DMATR_IRQ ? 1 : 0);

#ifndef DMASTREAMS // simpler DMA

	int dircode = (istx ? 1 : 0);
	uint32_t mem2mem = (axfer->flags & DMATR_MEM_TO_MEM ? 1 : 0);

	regs->CCR = 0
		| (mem2mem << 14)   // MEM2MEM: 1 = memory to memory mode
		| ((priority & 3) << 12)  // PL(2): priority level
		| (sizecode << 10)  // MSIZE(2): Memory data size, 8 bit
		| (sizecode <<  8)  // PSIZE(2): Periph data size, 8 bit
		| (meminc   <<  7)  // MINC: Memory increment mode
		| (0  <<  6)        // PINC: Peripheral increment mode
		| (circ <<  5)      // CIRC: Circular mode
		| (dircode <<  4)   // DIR(2): Data transfer direction (init with 0)
		| (0 <<  3)         // TEIE: Transfer error interrupt enable
		| (0 <<  2)         // HTIE: Half transfer interrupt enable
		| (inte <<  1)      // TCIE: TCIE: Transfer complete interrupt enable
		| (0  <<  0)        // EN: keep not enabled
	;

	regs->CPAR = (uint32_t)periphaddr;

	if (istx)
	{
		regs->CMAR = (uint32_t)axfer->srcaddr;
	}
	else
	{
		regs->CMAR = (uint32_t)axfer->dstaddr;
	}

	regs->CNDTR = (uint32_t)axfer->count;

#else  // more advanced DMA

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

	regs->CR = 0
		| (chnum << 25)     // CHSEL(3): set the channel
		| (0  << 23)        // MBURST(2): memory burst, 0 = single transfer
		| (0  << 21)        // PBURST(2): peripheral burst
		| (0  << 19)        // CT: current target
		| (0  << 18)        // DBM: double buffer mode
		| ((priority & 3) << 16) // PL(2): priority level
		| (0  << 15)        // PINCOS: peripheral increment offset
		| (sizecode << 13)  // MSIZE(2): Memory data size, 8 bit
		| (sizecode << 11)  // PSIZE(2): Periph data size, 8 bit
		| (meminc   << 10)  // MINC: Memory increment mode
		| (0  <<  9)        // PINC: Peripheral increment mode
		| (circ <<  8)      // CIRC: Circular mode
		| (dircode <<  6)   // DIR(2): Data transfer direction (init with 0)
		| (0  <<  5)        // PFCTRL: Peripheral flow controller, 0 = DMA is the flow controller
		| (inte << 4)       // TCIE: TCIE: Transfer complete interrupt enable
		| (0  <<  1)        // (3): error interrupts
		| (0  <<  0)        // EN: keep not enabled
	;

	if (axfer->flags & DMATR_MEM_TO_MEM)
	{
		regs->PAR = (uint32_t)axfer->srcaddr;
		regs->M0AR = (uint32_t)axfer->dstaddr;
	}
	else if (istx)
	{
		regs->PAR = (uint32_t)periphaddr;
		regs->M0AR = (uint32_t)axfer->srcaddr;
	}
	else
	{
		regs->PAR = (uint32_t)periphaddr;
		regs->M0AR = (uint32_t)axfer->dstaddr;
	}

	regs->NDTR = (uint32_t)axfer->count;

#endif

}

#endif // if HWDMA_IMPLEMENTED
