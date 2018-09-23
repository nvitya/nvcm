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
 *  file:     hwadc.cpp
 *  brief:    STM32 Simple Internal ADC
 *  version:  1.00
 *  date:     2018-09-22
 *  authors:  nvitya
*/

#include "hwadc_stm32.h"
#include "clockcnt.h"

#if defined(MCUSF_F1)

// ADC_v1: F1

bool THwAdc_stm32::Init(int adevnum, uint32_t achannel_map)
{
	uint32_t tmp;

	initialized = false;

	devnum = adevnum;
	initialized = false;
	channel_map = achannel_map;

	regs = nullptr;
	if      (1 == devnum)
	{
		regs = ADC1;
		RCC->APB2ENR |= RCC_APB2ENR_ADC1EN;
	}
#ifdef ADC2
	else if (2 == devnum)
	{
		regs = ADC2;
		RCC->APB2ENR |= RCC_APB2ENR_ADC2EN;
	}
#endif
	else
	{
		return false;
	}

	// DMA (requires
	dmach.Init(1, 1, 0); // it has a fixed channel
	dmach.Prepare(false, (void *)&regs->DR, 0);

	// set ADC clock
	uint32_t baseclock = SystemCoreClock;
	uint32_t adcmaxclock = 14000000;
	uint32_t adcdiv = 2;
	while ((adcdiv < 8) && (baseclock / adcdiv > adcmaxclock))
	{
		adcdiv += 2;
	}
	RCC->CFGR &= ~RCC_CFGR_ADCPRE_Msk;
	RCC->CFGR |= (((adcdiv >> 1) - 1) << RCC_CFGR_ADCPRE_Pos);

	adc_clock = baseclock / adcdiv;

	// stop adc
	regs->CR2 &= ~(ADC_CR2_ADON);

	regs->CR1 = 0
	  | (0 << 23)  // AWDEN: analog watchdog enable
	  | (0 << 22)  // JAWDEN: analog watchdog enable for injected ch.
	  | (0 << 16)  // DUALMOD(4): dual mode selection, 0 = independent mode
	  | (0 << 13)  // DISCNUM(3): discontinuous mode channel count
	  | (0 << 12)  // JDISCEN: Discontinuous mode on injected channels
	  | (0 << 11)  // DISCEN: Discontinuous mode on regular channels
	  | (0 << 10)  // JAUTO: Automatic Injected Group conversion
	  | (0 <<  9)  // AWDSGL: Enable the watchdog on a single channel in scan mode
	  | (1 <<  8)  // SCAN: Scan mode
	  | (0 <<  7)  // JEOCIE: Interrupt enable for injected channels
	  | (0 <<  6)  // AWDIE: Analog watchdog interrupt enable
	  | (0 <<  5)  // EOCIE: Interrupt enable for EOC
	  | (0 <<  0)  // AWDCH(5): Analog watchdog channel select
	;

	regs->CR2 = 0
		| (1 << 23)  // TSVREFE: Temperature sensor and VREFINT enable
		| (0 << 22)  // SWSTART: Start conversion of regular channels
		| (0 << 21)  // JSWSTART: Start conversion of injected channels
		| (0 << 20)  // EXTTRIG: External trigger conversion mode for regular channels
		| (7 << 17)  // EXTSEL(3): External event select for regular group (see reference), 7 = SWSTART
		| (0 << 15)  // JEXTTRIG: External trigger conversion mode for injected channels
		| (0 << 12)  // JEXTSEL(3): External event select for injected group
		| (0 << 11)  // ALIGN: Data alignment, 0 = right, 1 = left
		| (1 <<  8)  // DMA: 1 = DMA enabled (only for ADC1, ADC2 does not have direct DMA connection)
		| (0 <<  3)  // RSTCAL: Reset calibration, 1 = initialize calibration register
		| (0 <<  2)  // CAL: A/D Calibration, 1 = enable calibration, 0 = calibration completed
		| (1 <<  1)  // CONT: Continuous conversion, 1 = continous conversion mode
		| (1 <<  0)  // ADON: A/D converter ON / OFF, 1 = enable ADC and start conversion
	;

/*
	delay_us(1);
	regs->CR2 |= ADC_CR2_CAL_Msk;
	while (regs->CR2 & ADC_CR2_CAL_Msk)
	{
		// wait until the calibration finished.
	}
*/

	// setup channel sampling time registers

	uint32_t stcode = 0; // sampling time code, use the fastest sampling
	// 0: 1.5 cycles
	// 1: 7.5 cycles
	// 2: 13.5 cycles
	// 3: 28.5 cycles
	// 4: 41.5 cycles
	// 5: 55.5 cycles
	// 6: 71.5 cycles
	// 7: 239.5 cycles

	int i;
	tmp = 0;
	for (i = 0; i < 8; ++i)
	{
		tmp |= (stcode << (i * 3));
	}
	regs->SMPR1 = tmp;
	tmp = 0;
	for (i = 0; i < 10; ++i)
	{
		tmp |= (stcode << (i * 3));
	}
	regs->SMPR2 = 0;

	// calculate the actual conversion rate

	// total conversion cycles:  12.5 ADC clocks + sampling time (= 1.5 ADC clocks)
	conv_adc_clocks = 14;
	act_conv_rate = adc_clock / conv_adc_clocks;

	// setup the regular sequence based on the channel map and start the cyclic conversion
	SetupChannels();

	initialized = true;
	return true;
}

void THwAdc_stm32::SetupChannels()
{
	uint32_t ch;
	uint32_t sqr[3] = {0, 0, 0};
	uint32_t * psqr = &sqr[0];
	uint32_t bitshift = 0;

	for (ch = 0; ch < HWADC_MAX_CHANNELS; ++ch)
	{
		dmadata[ch] = 0x1111 + ch; // set some markers for diagnostics
		databyid[ch] = &dmadata[ch]; // initialize with valid pointers
	}

	dmadatacnt = 0;
	for (ch = 0; ch < HWADC_MAX_CHANNELS; ++ch)
	{
		if (channel_map & (1 << ch))
		{
			// add this channel
			*psqr |= (ch << bitshift);
			databyid[ch] = &dmadata[dmadatacnt]; // set the decode map

			++dmadatacnt;

			bitshift += 5;
			if (bitshift > 25)
			{
				// go to the next register
				bitshift = 0;
				++psqr;
			}
		}
	}

	regs->SQR3 = sqr[0];
	regs->SQR2 = sqr[1];
	regs->SQR1 = (sqr[2] & 0x000FFFFF) | ((dmadatacnt-1) << 20);  // this contains the sequence length too

  // prepare the DMA transfer

	dmaxfer.bytewidth = 2;
	dmaxfer.count = dmadatacnt;
	dmaxfer.dstaddr = &dmadata[0];
	dmaxfer.flags = DMATR_CIRCULAR; // ST supports it !
	dmach.StartTransfer(&dmaxfer);

	// and start the conversion
	regs->CR2 |= (ADC_CR2_SWSTART | ADC_CR2_ADON | ADC_CR2_EXTTRIG);
}

#elif defined(MCUSF_F7) || defined(MCUSF_F4)

bool THwAdc_stm32::Init(int adevnum, uint32_t achannel_map)
{
	uint32_t tmp;

	initialized = false;

	devnum = adevnum;
	initialized = false;
	channel_map = achannel_map;

	regs = nullptr;
	if      (1 == devnum)
	{
		regs = ADC1;
		RCC->APB2ENR |= RCC_APB2ENR_ADC1EN;
		if (dmastream == 4)
		{
			dmach.Init(2, 4, 0); // alternative
		}
		else
		{
			dmach.Init(2, 0, 0); // default
		}
	}
#ifdef ADC2
	else if (2 == devnum)
	{
		regs = ADC2;
		RCC->APB2ENR |= RCC_APB2ENR_ADC2EN;
		if (dmastream == 3)
		{
			dmach.Init(2, 3, 1); // alternative
		}
		else
		{
			dmach.Init(2, 2, 1); // default
		}
	}
#endif
#ifdef ADC3
	else if (2 == devnum)
	{
		regs = ADC3;
		RCC->APB2ENR |= RCC_APB2ENR_ADC3EN;
		if (dmastream == 0)
		{
			dmach.Init(2, 0, 2); // alternative
		}
		else
		{
			dmach.Init(2, 1, 2); // default
		}
	}
#endif
	else
	{
		return false;
	}

	dmach.Prepare(false, (void *)&regs->DR, 0);

	// set ADC clock
	uint32_t baseclock = SystemCoreClock / 2;  // usually 84, 90 or 108
	uint32_t adcmaxclock = 36000000;
	uint32_t adcdiv = 2;
	while ((adcdiv < 8) && (baseclock / adcdiv > adcmaxclock))
	{
		adcdiv += 2;
	}

	adc_clock = baseclock / adcdiv;

	// ADC Common register
	ADC->CCR = 0
		| (1 << 23)  // TSVREFE: 1 = temp sensor and internal reference enable
		| (0 << 22)  // VBATEN: VBAT ch enable
		| (((adcdiv >> 1) - 1) << 16)  // ADCPRE(2): ADC prescaler
		| (0 << 14)  // DMA: Direct memory access mode for multi ADC mode
		| (0 << 13)  // DDS: DMA disable selection (for multi-ADC mode)
		| (0 <<  8)  // DELAY(4): Delay between 2 sampling phases (for dual / triple mode)
		| (0 <<  0)  // MULTI(5): Multi ADC mode selection, 0 = independent mode
	;


	// stop adc
	regs->CR2 &= ~(ADC_CR2_ADON);

	regs->CR1 = 0
	  | (0 << 26)  // OVRIE: Overrun interrupt enable
	  | (0 << 24)  // RES(2): Resolution, 0 = 12 bit, 1 = 10 bit, 2 = 8 bit, 3 = 6 bit
	  | (0 << 23)  // AWDEN: analog watchdog enable
	  | (0 << 22)  // JAWDEN: analog watchdog enable for injected ch.
	  | (0 << 13)  // DISCNUM(3): discontinuous mode channel count
	  | (0 << 12)  // JDISCEN: Discontinuous mode on injected channels
	  | (0 << 11)  // DISCEN: Discontinuous mode on regular channels
	  | (0 << 10)  // JAUTO: Automatic Injected Group conversion
	  | (0 <<  9)  // AWDSGL: Enable the watchdog on a single channel in scan mode
	  | (1 <<  8)  // SCAN: Scan mode
	  | (0 <<  7)  // JEOCIE: Interrupt enable for injected channels
	  | (0 <<  6)  // AWDIE: Analog watchdog interrupt enable
	  | (0 <<  5)  // EOCIE: Interrupt enable for EOC
	  | (0 <<  0)  // AWDCH(5): Analog watchdog channel select
	;

	regs->CR2 = 0
		| (0 << 30)  // SWSTART: Start conversion of regular channels
		| (0 << 28)  // EXTEN(2): External trigger enable for regular channels, 0 = disabled, 1 = rising edge, 2 = falling, 3 both
		| (0 << 24)  // EXTSEL(4): External event select for regular group (see reference)
		| (0 << 22)  // JSWSTART: Start conversion of injected channels
		| (0 << 20)  // JEXTEN(2): External trigger enable for injected channels, 0 = disabled, 1 = rising edge, 2 = falling, 3 both
		| (0 << 16)  // JEXTSEL(4): External event select for injected group (see reference)
		| (0 << 11)  // ALIGN: Data alignment, 0 = right, 1 = left
		| (0 << 10)  // EOCS: End of conversion selection, 0 = sequence end
		| (1 <<  9)  // DDS: DMA disable selection (for single ADC mode)
		| (1 <<  8)  // DMA: 1 = DMA enabled (only for ADC1, ADC2 does not have direct DMA connection)
		| (1 <<  1)  // CONT: Continuous conversion, 1 = continous conversion mode
		| (1 <<  0)  // ADON: A/D converter ON / OFF, 1 = enable ADC and start conversion
	;

	// setup channel sampling time registers

	uint32_t stcode = 0; // sampling time code, index for the following array
	uint32_t sampling_clocks[8] = {3, 15, 28, 56, 84, 112, 144, 480};

	int i;
	tmp = 0;
	for (i = 0; i < 9; ++i)
	{
		tmp |= (stcode << (i * 3));
	}
	regs->SMPR1 = tmp;
	tmp = 0;
	for (i = 0; i < 10; ++i)
	{
		tmp |= (stcode << (i * 3));
	}
	regs->SMPR2 = 0;

	// calculate the actual conversion rate

	// total conversion cycles:  15 ADC clocks + sampling time (= 3 ADC clocks)
	conv_adc_clocks = sampling_clocks[stcode] + 15;
	act_conv_rate = adc_clock / conv_adc_clocks;

	// setup the regular sequence based on the channel map and start the cyclic conversion
	SetupChannels();

	initialized = true;
	return true;
}

void THwAdc_stm32::SetupChannels()
{
	uint32_t ch;
	uint32_t sqr[3] = {0, 0, 0};
	uint32_t * psqr = &sqr[0];
	uint32_t bitshift = 0;

	for (ch = 0; ch < HWADC_MAX_CHANNELS; ++ch)
	{
		dmadata[ch] = 0x1111 + ch; // set some markers for diagnostics
		databyid[ch] = &dmadata[ch]; // initialize with valid pointers
	}

	dmadatacnt = 0;
	for (ch = 0; ch < HWADC_MAX_CHANNELS; ++ch)
	{
		if (channel_map & (1 << ch))
		{
			// add this channel
			*psqr |= (ch << bitshift);
			databyid[ch] = &dmadata[dmadatacnt]; // set the decode map

			++dmadatacnt;

			bitshift += 5;
			if (bitshift > 25)
			{
				// go to the next register
				bitshift = 0;
				++psqr;
			}
		}
	}

	regs->SQR3 = sqr[0];
	regs->SQR2 = sqr[1];
	regs->SQR1 = (sqr[2] & 0x000FFFFF) | ((dmadatacnt-1) << 20);  // this contains the sequence length too

  // prepare the DMA transfer

	dmaxfer.bytewidth = 2;
	dmaxfer.count = dmadatacnt;
	dmaxfer.dstaddr = &dmadata[0];
	dmaxfer.flags = DMATR_CIRCULAR; // ST supports it !
	dmach.StartTransfer(&dmaxfer);

	// and start the conversion
	regs->CR2 |= (ADC_CR2_SWSTART | ADC_CR2_ADON);
}


#elif defined(MCUSF_F0)

// ADC_v2

bool THwAdc_stm32::Init(int adevnum, uint32_t achannel_map)
{
	uint32_t tmp;

	initialized = false;

	devnum = adevnum;
	initialized = false;
	channel_map = achannel_map;

	regs = nullptr;
	if      (1 == devnum)
	{
		regs = ADC1;
		RCC->APB2ENR |= RCC_APB2ENR_ADC1EN;
	}
#ifdef ADC2
	else if (2 == devnum)
	{
		regs = ADC2;
		RCC->APB2ENR |= RCC_APB2ENR_ADC2EN;
	}
#endif
	else
	{
		return false;
	}

	// DMA (requires
	dmach.Init(1, 1, 0); // it has a fixed channel
	dmach.Prepare(false, (void *)&regs->DR, 0);

	ADC1_COMMON->CCR = 0
		| (0 << 24)  // VBATEN: VBAT ch enable
		| (0 << 23)  // TSEN: temp sensor
		| (1 << 22)  // VREFEN: internal reference enable
	;

	regs->CR = 0;

	regs->CR = (1u << 31); // start calibration

	delay_us(10);

	while (regs->CR & (1u << 31))
	{
		// wait until the calibration is ready
	}

	delay_us(10);

	regs->CR = (1 << 0); // enable the ADC

	delay_us(10);

	while ((regs->CR & 1) == 0)
	{
		// wait until enabled
	}

	regs->IER = 0; // disable interrupts

	// CFGR1

	regs->CFGR1 = 0
	  | (0 << 26)  // AWDCH(5):  analogue watchdog channel select
	  | (0 << 23)  // AWDEN: 1 = enable AWD
	  | (0 << 22)  // AWDSGL: single ch. AWD
	  | (0 << 16)  // DISCEN: 1 = discontinous mode enabled
	  | (0 << 15)  // AUTOFF: 1 = auto off mode enabled
	  | (0 << 14)  // WAIT: 1 = wait conversion mode on
	  | (1 << 13)  // CONT: 1 = continous conversion mode
	  | (1 << 12)  // OVRMOD: 1 = the ADC_DR will be overwritten on overrun
	  | (0 << 10)  // EXTEN(2): external trigger enable
	  | (0 <<  6)  // EXTSEL(3): external trigger select
	  | (0 <<  5)  // ALIGN: 0 = right data alignment, 1 = left data aligment
	  | (0 <<  3)  // RES(2): resolution, 0 = 12 bit, 1 = 10 bit, 2 = 8 bit, 3 = 6 bit
	  | (0 <<  2)  // SCANDIR: 0 = upward scan (ch. 0 -> 18), 1 = backward scan (ch. 18 -> 0)
	  | (1 <<  1)  // DMACFG: 0 = one shot DMA mode, 1 = circular DMA mode
	  | (1 <<  0)  // DMAEN: 1 = DMA enabled
	;

	regs->CFGR2 = 0
	  | (0 << 30)  // CLKMODE(2), 0 = ADCCLK (14 MHz), 1 = PCLK / 2, 2 = PCLK / 4
	;

	adc_clock = 14000000; // dedicated internal clock

	// setup channel sampling time registers

	uint32_t stcode = 0; // sampling time code, use the fastest sampling
	// 0: 1.5 cycles
	// 1: 7.5 cycles
	// 2: 13.5 cycles
	// 3: 28.5 cycles
	// 4: 41.5 cycles
	// 5: 55.5 cycles
	// 6: 71.5 cycles
	// 7: 239.5 cycles
	regs->SMPR = stcode;

	regs->TR = 0; // no analogue watchdog

	// calculate the actual conversion rate

	// total conversion cycles:  12.5 ADC clocks + sampling time (= 1.5 ADC clocks)
	conv_adc_clocks = 14;
	act_conv_rate = adc_clock / conv_adc_clocks;

	// setup the regular sequence based on the channel map and start the cyclic conversion
	SetupChannels();

	initialized = true;
	return true;
}

void THwAdc_stm32::SetupChannels()
{
	uint32_t ch;
	uint32_t sqr[3] = {0, 0, 0};
	uint32_t * psqr = &sqr[0];
	uint32_t bitshift = 0;

	for (ch = 0; ch < HWADC_MAX_CHANNELS; ++ch)
	{
		dmadata[ch] = 0x1111 + ch; // set some markers for diagnostics
		databyid[ch] = &dmadata[ch]; // initialize with valid pointers
	}

	dmadatacnt = 0;
	for (ch = 0; ch < HWADC_MAX_CHANNELS; ++ch)
	{
		if (channel_map & (1 << ch))
		{
			// add this channel
			databyid[ch] = &dmadata[dmadatacnt]; // set the decode map
			++dmadatacnt;
		}
	}

	regs->CHSELR = channel_map;

  // prepare the DMA transfer

	dmaxfer.bytewidth = 2;
	dmaxfer.count = dmadatacnt;
	dmaxfer.dstaddr = &dmadata[0];
	dmaxfer.flags = DMATR_CIRCULAR; // ST supports it !
	dmach.StartTransfer(&dmaxfer);

	// and start the conversion
	regs->CR |= (1 << 2); // start the ADC
}

#else

#warning "ADC implementation is missing!"

bool THwAdc_stm32::Init(int adevnum, uint32_t achannel_map)
{
	return false;
}

#endif
