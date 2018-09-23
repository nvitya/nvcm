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
 *  file:     hwadc.h
 *  brief:    Simple internal ADC vendor-independent definitions
 *  version:  1.00
 *  date:     2018-09-22
 *  authors:  nvitya
*/

#ifndef _HWADC_H_PRE_
#define _HWADC_H_PRE_

#include "platform.h"
#include "hwpins.h"
#include "hwdma.h"
#include "errors.h"

class THwAdc_pre
{
public:	// settings
	bool 					 initialized = false;

	int      			 devnum = -1;

	uint32_t       sampling_time_ns = 0; // 0 = smallest possible
	uint32_t       adc_clock = 0;        // actual ADC clock
	uint32_t       conv_adc_clocks = 0;  // ADC clocks required to one conversion
	uint32_t       act_conv_rate = 0;    // actual conversion rate in Hz
};

#endif // ndef _HWADC_H_PRE_

#ifndef HWADC_PRE_ONLY

//-----------------------------------------------------------------------------

#ifndef HWADC_H_
#define HWADC_H_

#include "mcu_impl.h"

#ifndef HWADC_IMPL

class THwAdc_noimpl : public THwAdc_pre
{
public: // mandatory
	uint16_t ChValue(uint8_t ach) { return 0; }
	bool Init(int adevnum, uint32_t achannel_map)  { return false; }
};

#define HWADC_IMPL   THwAdc_noimpl

#endif // ndef HWADC_IMPL

//-----------------------------------------------------------------------------

class THwAdc : public HWADC_IMPL
{
};

#endif // HWADC_H_

#else
  #undef HWADC_PRE_ONLY
#endif
