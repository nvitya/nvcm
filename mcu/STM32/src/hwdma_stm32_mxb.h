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
 *  file:     hwdma_stm32_mxb.h
 *  brief:    3x DMA for the STM32H7 series: MDMA + (x)DMA + BDMA
 *  version:  1.00
 *  date:     2020-12-11
 *  authors:  nvitya
 *  notes:
*/

#ifndef HWDMA_STM32_MXB_H_
#define HWDMA_STM32_MXB_H_

#include "platform.h"

#if defined(HWDMA_MXB)

#define HWDMA_PRE_ONLY
#include "hwdma.h"

#define HWDMA_MDMA  0
#define HWDMA_DMA1  1
#define HWDMA_DMA2  2
#define HWDMA_BDMA  3

class THwDmaChannel_stm32 : public THwDmaChannel_pre
{
public: // special STM32 specific settings
	unsigned           per_burst = 0;  // 0 = single, 1 = 4 beats, 2 = 8 beats, 3 = 16 beats
	unsigned           mem_burst = 0;  // 0 = single, 1 = 4 beats, 2 = 8 beats, 3 = 16 beats
	unsigned           per_flow_controller = 0;  // 0 = DMA, 1 = peripheral

public:
	MDMA_Channel_TypeDef *  mregs = nullptr;
	DMA_Stream_TypeDef *    xregs = nullptr;
	BDMA_Channel_TypeDef *  bregs = nullptr;


	int                     dmanum = 1;
	unsigned                rqnum;
  //uint8_t                 streamnum = 0;

  bool Init(int admanum, int achannel, int arequest); // admanum: 0=MDMA, 1=DMA1, 2=DMA2, 3=BDMA

	void Prepare(bool aistx, void * aperiphaddr, unsigned aflags);
	void Disable();
	void Enable();

	inline bool Enabled()        { return ((*crreg & 1) != 0); }

	// using the NDTR register for the termination is not reliable
	// because sometimes it might overflow due bursts
	//inline bool Active() 				 { return ((*ndtreg & 0xFFFF) != 0);	}
	inline bool Active()        { return ((*crreg & 1) != 0); }

	inline uint16_t Remaining()  { return (*ndtreg & 0xFFFF); }
	inline void ClearIrqFlag()   { *irqstclrreg = irqclrmask; }

	bool StartTransfer(THwDmaTransfer * axfer);
	bool StartMemToMem(THwDmaTransfer * axfer);

	void PrepareTransfer(THwDmaTransfer * axfer);
	inline void StartPreparedTransfer() { Enable(); }

public:
	uint32_t           irqclrmask = 0;
  __IO uint32_t *    crreg;
	__IO uint32_t *    ndtreg = nullptr;

  __IO uint32_t *    irqstreg;
  __IO uint32_t *    irqstclrreg;
  unsigned           irqstshift;

};

#define HWDMACHANNEL_IMPL  THwDmaChannel_stm32

#endif // defined(HWDMA_MXB)

#endif // def HWDMA_STM32_MXB_H_
