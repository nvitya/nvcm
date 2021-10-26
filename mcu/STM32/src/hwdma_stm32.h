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
 *  file:     hwdma_stm32.h
 *  brief:    STM32 DMA
 *  version:  1.00
 *  date:     2018-02-10
 *  authors:  nvitya
*/

#ifndef HWDMA_STM32_H_
#define HWDMA_STM32_H_

#include "platform.h"

#if !defined(HWDMA_MXB)

#define HWDMA_PRE_ONLY
#include "hwdma.h"

#ifdef DMA1_Stream0_BASE
  #define DMASTREAMS
#endif

#ifdef DMASTREAMS
  #define DMA_NDTR_REG  NDTR
#else
  #define DMA_NDTR_REG  CNDTR
#endif

class THwDmaChannel_stm32 : public THwDmaChannel_pre
{
public: // special STM32 specific settings
	unsigned           per_burst = 0;  // 0 = single, 1 = 4 beats, 2 = 8 beats, 3 = 16 beats
	unsigned           mem_burst = 0;  // 0 = single, 1 = 4 beats, 2 = 8 beats, 3 = 16 beats
	unsigned           per_flow_controller = 0;  // 0 = DMA, 1 = peripheral

public:
	int                dmanum = 1;
	HW_DMA_REGS *      regs = nullptr;

#ifdef DMASTREAMS
  uint8_t            streamnum;

 #ifdef DMAMUX1
    bool Init(int admanum, int astream, int arequest); // H7
 #else
    bool Init(int admanum, int astream, int achannel); // F4, F7
 #endif
#else
	bool Init(int admanum, int achannel, int arequest);
#endif

	void Prepare(bool aistx, void * aperiphaddr, unsigned aflags);
	void Disable();
	void Enable();

	inline bool Enabled()        { return ((*crreg & 1) != 0); }

#ifndef DMASTREAMS
	// the EN flag based termination check does not work on G4
  inline bool Active() 				 { return (regs->DMA_NDTR_REG != 0);	}
#else
	// using the NDTR register for the termination is not reliable
	// because sometimes it might overflow due bursts
	inline bool Active() 				 { return ((*crreg & 1) != 0);	}
#endif

	inline uint16_t Remaining()  { return regs->DMA_NDTR_REG; }

	bool StartTransfer(THwDmaTransfer * axfer);
	bool StartMemToMem(THwDmaTransfer * axfer);

	void PrepareTransfer(THwDmaTransfer * axfer);
	inline void StartPreparedTransfer() { Enable(); }

	inline void ClearIrqFlag()
	{
		#ifndef DMASTREAMS
			*irqstclrreg = (0x0F << irqstshift);
		#else
			*irqstclrreg = (0x3F << irqstshift);
		#endif
	}

public:
  __IO unsigned *    crreg;

  __IO unsigned *    irqstreg;
  __IO unsigned *    irqstclrreg;
  unsigned           irqstshift;

};

#define HWDMACHANNEL_IMPL  THwDmaChannel_stm32

#endif // !defined(MCUSF_H7)

#endif // def HWDMA_STM32_H_
