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
 *  file:     hwdma_lpc_v3.h
 *  brief:    LPC_V3 DMA
 *  version:  1.00
 *  date:     2018-02-10
 *  authors:  nvitya
*/

#ifndef HWDMA_LPC_V3_H_
#define HWDMA_LPC_V3_H_

#define HWDMA_PRE_ONLY
#include "hwdma.h"

// DMA descriptor for LPC54

typedef struct TDmaDesc
{
	unsigned     XFERCFG;  // not used for the first descriptor (in the descriptor table)
	void *       SRCEND;
	void *       DSTEND;
	TDmaDesc *   NEXT;
//
} TDmaDesc;

// Better definition for the LPC54 DMA registers

typedef struct TDmaChRegs
{
  __IO uint32_t CFG;
  __I  uint32_t CTLSTAT;
  __IO uint32_t XFERCFG;     // for the first transfer

       	 	 	 	 	 	 	 	 	 uint32_t RESERVED_0;
//
} TDmaChRegs;

typedef struct TDmaRegs
{
	// DMA unit control registers

  __IO uint32_t CTRL;
  __I  uint32_t INTSTAT;
  __IO uint32_t SRAMBASE;

                            	uint8_t RESERVED_00[20];

  // Grouped Channel Bit Registers

	__IO uint32_t ENABLESET;
			                      	uint32_t RESERVED_0;
	__O  uint32_t ENABLECLR;
			                   	 	  uint32_t RESERVED_1;
	__I  uint32_t ACTIVE;
			 	 	 	 	 	 	 	 	 	 	 	 	uint32_t RESERVED_2;
	__I  uint32_t BUSY;
			 	 	 	 	 	 	 	 	 	 	 	  uint32_t RESERVED_3;
	__IO uint32_t ERRINT;
			 	 	 	 	 	 	 	 	 	 	 	 	uint32_t RESERVED_4;
	__IO uint32_t INTENSET;
			 	 	 	 	 	 	 	 	 	 	 	  uint32_t RESERVED_5;
	__O  uint32_t INTENCLR;
			 	 	 	 	 	 	 	 	 	 	 	  uint32_t RESERVED_6;
	__IO uint32_t INTA;
			 	 	 	 	 	 	 	 	 	 	 	  uint32_t RESERVED_7;
	__IO uint32_t INTB;
			 	 	 	 	 	 	 	 	 	 	 	 	uint32_t RESERVED_8;
	__O  uint32_t SETVALID;
			 	 	 	 	 	 	 	 	 	 	 	  uint32_t RESERVED_9;
	__O  uint32_t SETTRIG;
			 	 	 	 	 	 	 	 	 	 	 	  uint32_t RESERVED_10;
	__O  uint32_t ABORT;

       	 	 	 	 	 	 	 	 	 	 	  uint8_t RESERVED_01[900];

  // Channel Registers (CFG, CTLSTAT, XFERCFG):

  TDmaChRegs    CHANNELS[30];
//
} TDmaRegs;

#define HW_DMA       ((TDmaRegs *)DMA0_BASE)
#define HW_DMA_REGS  TDmaChRegs

class THwDmaChannel_lpc_v3 : public THwDmaChannel_pre
{
public:
	unsigned           chbit = 0;
	int                perid = -1;
	TDmaChRegs *       regs = nullptr;
	TDmaDesc *         firstdesc = nullptr;

	bool Init(int achnum);
	bool InitPeriphDma(bool aistx, void * aregs, void * aaltregs);  // special function for Atmel PDMA

	void Prepare(bool aistx, void * aperiphaddr, unsigned aflags);

	void Disable() { HW_DMA->ENABLECLR = chbit;  while (HW_DMA->BUSY & chbit) ; }
	void Enable()  { HW_DMA->ENABLESET = chbit; }

	bool Enabled() { return ((HW_DMA->ENABLESET & chbit) != 0); }
	bool Active()  { return ((HW_DMA->ACTIVE & chbit) != 0); }

	bool StartTransfer(THwDmaTransfer * axfer);
	bool StartMemToMem(THwDmaTransfer * axfer);
};

#define HWDMACHANNEL_IMPL  THwDmaChannel_lpc_v3

#endif // def HWDMA_LPC_V3_H_
