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
 *  file:     hwadc_stm32.cpp
 *  brief:    STM32 Simple Internal ADC
 *  version:  1.00
 *  date:     2018-09-22
 *  authors:  nvitya
*/

#ifndef HWADC_STM32_H_
#define HWADC_STM32_H_

#define HWADC_PRE_ONLY
#include "hwadc.h"

#include "hwdma.h"

#define HWADC_REGS  ADC_TypeDef

#ifdef ADC12_COMMON
  #define HWADC_COMMON_REGS  ADC_Common_TypeDef
#else
  #define HWADC_COMMON_REGS  void
#endif

#define HWADC_MAX_CHANNELS  18
#define HWADC_DATA_LSHIFT    0

class THwAdc_stm32 : public THwAdc_pre
{
public:
	THwDmaChannel   dmach;
	THwDmaTransfer  dmaxfer;

	int             dmaalloc = -1;    // -1 = use some default, otherwise 0x103 = dma1/ch3
	int             dmastream = -1;   // -1 = use default dma stream

	uint32_t        channel_map = 0;  // by default convert only ch 0

	uint16_t        dmadata[HWADC_MAX_CHANNELS];    // puffer for data storage (transferred with the DMA)
	uint16_t *      databyid[HWADC_MAX_CHANNELS];

	uint8_t         dmadatacnt = 0;

	HWADC_REGS *          regs = nullptr;
	HWADC_COMMON_REGS *   commonregs = nullptr;

	bool            Init(int adevnum, uint32_t achannel_map);
	inline uint16_t ChValue(uint8_t ach) { return *(databyid[ach]); }

	void            StartFreeRun(uint32_t achsel);
	void            StopFreeRun();
	void            StartRecord(uint32_t achsel, uint32_t acount, uint16_t * adstptr);
	bool            RecordFinished() { return !dmach.Active(); }

public: // STM32 helper functions
	void            SetupChannels(uint32_t achsel);
	void            StartContConv();
};

#define HWADC_IMPL THwAdc_stm32

#endif /* HWADC_STM32_H_ */
