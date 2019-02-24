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

#ifndef HWDMA_ATSAM_V2_H_
#define HWDMA_ATSAM_V2_H_

#define HWDMA_PRE_ONLY
#include "hwdma.h"

#if defined(MCUSF_E5X)
  #define MAX_DMA_CHANNELS  32
#elif defined(MCUSF_C2X)
  #define MAX_DMA_CHANNELS  12
#else // D9-10
  #define MAX_DMA_CHANNELS   6
#endif


typedef struct TDmaChannelDesc
{
	__IO uint16_t   BTCTRL;
	__IO uint16_t   BTCNT;
	__IO uint32_t   SRCADDR;
	__IO uint32_t   DSTADDR;
	__IO uint32_t   DESCADDR;
//
} TDmaChannelDesc;

#define HW_DMA_REGS   TDmaChannelDesc
#define HW_DMA_CREGS  Dmac

class THwDmaChannel_atsam_v2 : public THwDmaChannel_pre
{
public:
	unsigned           chbit = 0;
	int                perid = -1;
	HW_DMA_REGS *      regs = nullptr;
	HW_DMA_REGS *      wbregs = nullptr;
	Dmac *             ctrlregs = nullptr;
#ifndef DMAC_CHID_OFFSET
	DmacChannel *      chregs = nullptr;
#endif

	bool Init(int achnum, int aperid);

	void Prepare(bool aistx, void * aperiphaddr, unsigned aflags);
	void Enable();

#ifdef DMAC_CHID_OFFSET
  void Disable();
#else
  inline void Disable() { chregs->CHCTRLA.bit.ENABLE = 0; }
#endif
	inline bool Enabled() { return ((wbregs->BTCTRL & 1) != 0); }
	//inline bool Enabled() { return (chregs->CHSTATUS.bit.BUSY != 0); }
	inline bool Active()  { return Enabled(); }

	inline void ClearIrqFlag()  { chregs->CHINTFLAG.reg = (1 << 1); } // clear transfer complete IRQ

	void PrepareTransfer(THwDmaTransfer * axfer);
	inline void StartPreparedTransfer()              { Enable(); }
};

#define HWDMACHANNEL_IMPL  THwDmaChannel_atsam_v2

#endif // def HWDMA_ATSAM_V2_H_
