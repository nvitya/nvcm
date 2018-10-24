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
 *  file:     hwdma_lpc.h
 *  brief:    LPC DMA
 *  version:  1.00
 *  date:     2018-02-10
 *  authors:  nvitya
*/

#ifndef HWDMA_LPC_H_
#define HWDMA_LPC_H_

#define HWDMA_PRE_ONLY
#include "hwdma.h"

class THwDmaChannel_lpc : public THwDmaChannel_pre
{
public:
	unsigned           chbit = 0;
	int                perid = -1;
	HW_DMA_REGS *      regs = nullptr;
#ifdef HW_DMA_ALT_REGS
  HW_DMA_ALT_REGS *  altregs = nullptr;  // Some Atmel systems have two different DMA system
#endif

	bool Init(int achnum, int aperid, int arqmux);
	bool InitPeriphDma(bool aistx, void * aregs, void * aaltregs);  // special function for Atmel PDMA

	void Prepare(bool aistx, void * aperiphaddr, unsigned aflags);
	void Disable();
	void Enable();

	bool Enabled();
	bool Active();

	void PrepareTransfer(THwDmaTransfer * axfer);
	inline void StartPreparedTransfer() { Enable(); }
};

#define HWDMACHANNEL_IMPL  THwDmaChannel_lpc

#endif // def HWDMA_LPC_H_
