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
 *  file:     hwadc_atsam.cpp
 *  brief:    ATSAM Simple Internal ADC
 *  version:  1.00
 *  date:     2018-09-24
 *  authors:  nvitya
*/

#include "hwadc_atsam.h"
#include "clockcnt.h"

#if defined(ADC)

bool THwAdc_atsam::Init(int adevnum, uint32_t achannel_map)
{
	uint32_t tmp;
	unsigned perid;
	unsigned periphclock = SystemCoreClock;

	initialized = false;

	devnum = adevnum;
	initialized = false;
	channel_map = achannel_map;

	regs = nullptr;
	if      ((1 == devnum) || (0 == devnum))
	{
		regs = ADC;
		perid = ID_ADC;
	}
	else
	{
		return false;
	}

	// Enable the peripheral
	if (perid < 32)
	{
		PMC->PMC_PCER0 = (1 << perid);
	}
	else
	{
		PMC->PMC_PCER1 = (1 << (perid-32));
	}

	regs->ADC_CR = 0
		| (1 <<  0)  // SWRST: Software Reset
		| (0 <<  1)  // START: Start Conversion
		| (0 <<  3)  // AUTOCAL: Automatic Calibration of ADC, 1 = start calibration sequence
	;

	regs->ADC_ACR = 0
		| (1 <<  4)  // TSON: Temperature Sensor On
		| (0 <<  8)  // IBCTL(2): ADC Bias Current Control
	;

	// enable the selected channels
	regs->ADC_CHDR = (~channel_map & 0xFFFF);
	regs->ADC_CHER = channel_map; // enable channels

	// set ADC clock
	uint32_t adcmaxclock = 22000000;
	uint32_t baseclock = periphclock / 2;
	uint32_t adcdiv = 1;
	while ((adcdiv < 255) && (baseclock / adcdiv > adcmaxclock))
	{
		adcdiv += 1;
	}

	adc_clock = baseclock / adcdiv;

	regs->ADC_MR = 0
	  | (0 <<  0)  // TRGEN: Trigger Enable, 0 = HW trigger disabled
	  | (0 <<  1)  // TRGSEL(3): Trigger Selection
	  | (0 <<  5)  // SLEEP: Sleep Mode, 0 = normal
	  | (0 <<  6)  // FWUP: Fast Wake Up
	  | (1 <<  7)  // FREERUN: Free Run Mode, 1 = never wait for any trigger
	  | ((adcdiv - 1) <<  8)  // PRESCAL(8): Prescaler Rate Selection
	  | (0 << 16)  // STARTUP(4): Startup Time
	  | (0 << 20)  // SETTLING(2): Analog Settling Time
	  | (0 << 23)  // ANACH: Analog Change, 0 = common DIFF0, GAIN0, OFF0, 1 = individual DIFF, GAIN, OFF settings
	  | (0 << 24)  // TRACKTIM(4): Tracking Time
	  | (2 << 28)  // TRANSFER(2): Hold Time
	  | (0 << 31)  // USEQ: Use Sequence Enable, 0 = channel number order, 1 = user sequence in ADC_SEQRx
	;

	regs->ADC_EMR = 0
		| (0 <<  0)  // CMPMODE(2): Comparison Mode
		| (0 <<  4)  // CMPSEL(4): Comparison Selected Channel
		| (0 <<  9)  // CMPALL: Compare All Channels
		| (0 << 24)  // TAG: Tag of the ADC_LCDR, 1 = channel number added to the result register
	;

	regs->ADC_IDR = 0xFFFFFFFF; // disable interrupts

	regs->ADC_CGR = 0; // single ended gain = 1 for all channels
	regs->ADC_COR = 0; // single ended mode, no offset for all channels

	// calculate the actual conversion rate
	conv_adc_clocks = 22;
	act_conv_rate = adc_clock / conv_adc_clocks;

	// Start the conversion
	regs->ADC_CR = 0
		| (0 <<  0)  // SWRST: Software Reset
		| (1 <<  1)  // START: Start Conversion
		| (1 <<  3)  // AUTOCAL: Automatic Calibration of ADC, 1 = start calibration sequence
	;

	initialized = true;
	return true;
}

#elif defined(AFEC0)

bool THwAdc_atsam::Init(int adevnum, uint32_t achannel_map)
{
	uint32_t tmp;
	unsigned perid;
	unsigned periphclock = SystemCoreClock;

	initialized = false;

	devnum = adevnum;
	initialized = false;
	channel_map = achannel_map;

	regs = nullptr;
	if      (0 == devnum)
	{
		regs = AFEC0;
		perid = ID_AFEC0;
	}
#if defined(AFEC1)
	else if (1 == devnum)
	{
		regs = AFEC1;
		perid = ID_AFEC1;
	}
#endif
	else
	{
		return false;
	}

	// Enable the peripheral
	if (perid < 32)
	{
		PMC->PMC_PCER0 = (1 << perid);
	}
	else
	{
		PMC->PMC_PCER1 = (1 << (perid-32));
	}

	regs->AFEC_CR = 0
		| (1 <<  0)  // SWRST: Software Reset
		| (0 <<  1)  // START: Start Conversion
	;

	regs->AFEC_ACR = 0
		| (1 <<  2)  // PGA0EN: 1 = Programmable gain amplifier 0 enabled
		| (1 <<  3)  // PGA1EN: 1 = Programmable gain amplifier 1 enabled
		| (3 <<  8)  // IBCTL(2): ADC Bias Current Control
	;

	regs->AFEC_SHMR = 0; // single sample and hold mode

	// enable the selected channels
	regs->AFEC_CHDR = (~channel_map & 0xFFFF);
	regs->AFEC_CHER = channel_map; // enable channels

	// set ADC clock
	uint32_t adcmaxclock = 40000000;
	uint32_t baseclock = periphclock / 2;
	uint32_t adcdiv = 1;
	while ((adcdiv < 255) && (baseclock / adcdiv > adcmaxclock))
	{
		adcdiv += 1;
	}

	adc_clock = baseclock / adcdiv;

	regs->AFEC_MR = 0
	  | (0 <<  0)  // TRGEN: Trigger Enable, 0 = HW trigger disabled
	  | (0 <<  1)  // TRGSEL(3): Trigger Selection
	  | (0 <<  5)  // SLEEP: Sleep Mode, 0 = normal
	  | (0 <<  6)  // FWUP: Fast Wake Up
	  | (1 <<  7)  // FREERUN: Free Run Mode, 1 = never wait for any trigger
	  | ((adcdiv - 1) <<  8)  // PRESCAL(8): Prescaler Rate Selection
	  | (0 << 16)  // STARTUP(4): Startup Time
	  | (1 << 23)  // ONE: must be 1
	  | (0 << 24)  // TRACKTIM(4): Tracking Time
	  | (0 << 28)  // TRANSFER(2): should be 0
	  | (0 << 31)  // USEQ: Use Sequence Enable, 0 = channel number order, 1 = user sequence in ADC_SEQRx
	  | (0 << 25)  // STM: single trigger mode
	  | (0 << 28)  // SIGNMODE(2): 0 = unsigned conversions
	;

	regs->AFEC_EMR = 0
		| (0 <<  0)  // CMPMODE(2): Comparison Mode
		| (0 <<  3)  // CMPSEL(5): Comparison Selected Channel
		| (0 <<  9)  // CMPALL: Compare All Channels
		| (0 << 16)  // RES(3): 0 = no averaging
		| (0 << 24)  // TAG: Tag of the ADC_LCDR, 1 = channel number added to the result register
	;

	regs->AFEC_IDR = 0xFFFFFFFF; // disable interrupts

	regs->AFEC_DIFFR = 0x0000; // select single ended mode

	regs->AFEC_CGR = 0x00000000; // gain = 1

	// calculate the actual conversion rate
	conv_adc_clocks = 21;
	act_conv_rate = adc_clock / conv_adc_clocks;

	// program the offset registers, this is required for the single ended mode
	int i;
	for (i = 0; i < 16; ++i)
	{
		regs->AFEC_CSELR = i;
		regs->AFEC_COCR = 512;
	}

	// Start the conversion
	regs->AFEC_CR = 0
		| (0 <<  0)  // SWRST: Software Reset
		| (1 <<  1)  // START: Start Conversion
	;

	initialized = true;
	return true;
}

uint16_t THwAdc_atsam::ChValue(uint8_t ach)
{
	regs->AFEC_CSELR = ach;
	return (regs->AFEC_CDR & 0x0FFF);
}

#else
  #warning "ADC is not implemented for this ATSAM subfamily"
#endif
