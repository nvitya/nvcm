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
 *  file:     hwadc_lpc.h
 *  brief:    LPC Simple Internal ADC
 *  version:  1.00
 *  date:     2018-09-29
 *  authors:  nvitya
*/

#ifndef HWADC_LPC_H_
#define HWADC_LPC_H_

#define HWADC_PRE_ONLY
#include "hwadc.h"

#define HW_ADC_REGS  LPC_ADC_T

#define HWADC_MAX_CHANNELS  8
#define HWADC_DATA_LSHIFT   0


class THwAdc_lpc : public THwAdc_pre
{
public:
	uint32_t        channel_map = 0;

	THwDmaChannel   dmach;
	THwDmaTransfer  dmaxfer;
	int             dmachannel = 6;
	uint8_t         dmarqid = 13;

	HW_ADC_REGS *   regs = nullptr;

	bool            Init(int adevnum, uint32_t achannel_map);
	inline uint16_t ChValue(uint8_t ach) { return (regs->DR[ach] & 0xFFC0); }  // always left aligned

	void            StartFreeRun(uint32_t achsel);
	void            StopFreeRun();
	void            StartRecord(uint32_t achsel, uint32_t acount, uint16_t * adstptr);
	bool            RecordFinished() { return !dmach.Active(); }

public:
	uint32_t        adcdiv = 0;
};

#define HWADC_IMPL THwAdc_lpc

#endif /* HWADC_LPC_H_ */
