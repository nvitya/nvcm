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

#include "hwadc_stm32.h"
#include "clockcnt.h"
#include "stm32_utils.h"

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
		| (1 << 11)  // ALIGN: Data alignment, 0 = right, 1 = left
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
	StartFreeRun(channel_map);

	initialized = true;
	return true;
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
	else if (3 == devnum)
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
		| (1 << 11)  // ALIGN: Data alignment, 0 = right, 1 = left
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
	StartFreeRun(channel_map);

	initialized = true;
	return true;
}

#elif defined(MCUSF_G4) || defined(MCUSF_H7)

bool THwAdc_stm32::Init(int adevnum, uint32_t achannel_map)
{
	initialized = false;

	devnum = adevnum;
	initialized = false;
	channel_map = achannel_map;

	uint8_t dmamux;
	uint16_t defaultdma;

	regs = nullptr;
	if      (1 == devnum)
	{
		#if defined(RCC_AHB1ENR_ADC12EN) // H7
	  	RCC->AHB1ENR |= RCC_AHB1ENR_ADC12EN;
			dmamux = 9;
		#else
		  RCC->AHB2ENR |= RCC_AHB2ENR_ADC12EN;
			dmamux = 5;
    #endif

		regs = ADC1;
		commonregs = ADC12_COMMON;
		defaultdma = 0x201;
	}
#ifdef ADC2
	else if (2 == devnum)
	{
		#if defined(RCC_AHB1ENR_ADC12EN) // H7
			RCC->AHB1ENR |= RCC_AHB1ENR_ADC12EN;
			dmamux = 10;
		#else
			RCC->AHB2ENR |= RCC_AHB2ENR_ADC12EN;
			dmamux = 36;
		#endif
		regs = ADC2;
		commonregs = ADC12_COMMON;
		defaultdma = 0x202;
	}
#endif
#ifdef ADC3
	else if (3 == devnum)
	{
		#if defined(RCC_AHB4ENR_ADC3EN)  // H7
		  RCC->AHB4ENR |= RCC_AHB4ENR_ADC3EN;
			commonregs = ADC3_COMMON;
			dmamux = 115;
		#else
  		RCC->AHB2ENR |= RCC_AHB2ENR_ADC345EN;
			commonregs = ADC345_COMMON;
			dmamux = 37;
    #endif

		regs = ADC3;
		defaultdma = 0x203;
	}
#endif
#ifdef ADC4
	else if (4 == devnum)
	{
		regs = ADC4;
		commonregs = ADC345_COMMON;
		RCC->AHB2ENR |= RCC_AHB2ENR_ADC345EN;
		dmamux = 38;
		defaultdma = 0x204;
	}
#endif
#ifdef ADC5
	else if (5 == devnum)
	{
		regs = ADC5;
		commonregs = ADC345_COMMON;
		RCC->AHB2ENR |= RCC_AHB2ENR_ADC345EN;
		dmamux = 39;
		defaultdma = 0x205;
	}
#endif
	else
	{
		return false;
	}

	if (dmaalloc < 0)  dmaalloc = defaultdma;

	dmach.Init((dmaalloc >> 8), dmaalloc & 15, dmamux);  // re-init the DMA channel with the proper DMAMUX
	dmach.Prepare(false, (void *)&regs->DR, 0);

	// current setup: ADC clock = System Clock (AHB clock) / 4
	// this gives at 168 MHz CPU speed the maximal 42 MHz ADC clock (for single ended mode)
	adc_clock = stm32_bus_speed(0) / 4;

	// ADC Common register
	commonregs->CCR = 0
		| (1 << 24)  // VBATSEL: 1 = VBAT ch enable
		| (1 << 23)  // VSENSESEL: 1 = temp sensor channel enable
		| (1 << 22)  // VREFEN: 1 = Vrefint channel enable
		| (0 << 18)  // PRESC(4): ADC prescaler, 0 = do not divide
		| (3 << 16)  // CLKMODE(2): ADC clock mode, 3 = AHB clock / 4 (synchronous clock mode)
//		| (0 << 16)  // CLKMODE(2): ADC clock mode, 3 = AHB clock / 4 (synchronous clock mode)
		| (0 << 14)  // MDMA(2): Direct memory access mode for dual ADC mode, 0 = disabled
		| (0 << 13)  // DMACFG: Dual mode DMA config, 0 = one shot, 1 = circular
		| (0 <<  8)  // DELAY(4): Delay between 2 sampling phases (for dual / triple mode)
		| (0 <<  0)  // MULTI(5): Dual ADC selection, 0 = independent mode
	;


	// stop adc adc

	if (regs->CR & ADC_CR_ADEN)
	{
		regs->CR |= ADC_CR_ADDIS;
		while (regs->CR & ADC_CR_ADDIS) { } // wait until disabled
	}

	regs->DIFSEL = 0; // all channels are single ended

	regs->CR = 0
	  | (0 << 31)  // ADCAL: ADC calibration
	  | (0 << 30)  // ADCALDIF: Differential mode for calibration
	  | (0 << 29)  // DEEPPWD: Deep-power-down enable
	  | (1 << 28)  // ADVREGEN: ADC voltage regulator enable
	  | (0 <<  8)  // BOOST(3): 3 = boost on (H7 required for ADC clocks > 20 MHz)
	  | (0 <<  5)  // JADSTP: ADC stop of injected conversion command
	  | (0 <<  4)  // ADSTP: ADC stop of regular conversion command
	  | (0 <<  3)  // JADSTART: ADC start of injected conversion
	  | (0 <<  2)  // ADSTART: ADC start of regular conversion
	  | (0 <<  1)  // ADDIS: ADC disable command
	  | (0 <<  0)  // ADEN: ADC enable control
	;

	delay_us(30); // wait for ADC the reference voltage

	// start the calibration
	regs->CR |= ADC_CR_ADCAL;
	while (regs->CR & ADC_CR_ADCAL)
	{
		// wait for the calibration
	}

	// enable the ADC to be able to modify the other registers
	regs->CR |= ADC_CR_ADEN;

	regs->CFGR = 0
		| (1 << 31)  // JQDIS: Injected Queue disable
		| (0 << 26)  // AWD1CH(5): Analog watchdog 1 channel selection
		| (0 << 25)  // JAUTO: Automatic injected group conversion
		| (0 << 24)  // JAWD1EN: Analog watchdog 1 enable on injected channels
		| (0 << 23)  // AWD1EN: Analog watchdog 1 enable on regular channels
		| (0 << 22)  // AWD1SGL: Enable the watchdog 1 on a single channel or on all channels
		| (0 << 21)  // JQM: JSQR queue mode
		| (0 << 20)  // JDISCEN: Discontinuous mode on injected channels
		| (0 << 17)  // DISCNUM(3): Discontinuous mode channel count
		| (0 << 16)  // DISCEN: Discontinuous mode for regular channels
		| (1 << 15)  // ALIGN: Data alignment, 0 = right alignment, 1 = left alignment
		| (0 << 14)  // AUTDLY: Delayed conversion mode
		| (1 << 13)  // CONT: Single / continuous conversion mode for regular conversions
		| (1 << 12)  // OVRMOD: Overrun mode, 1 = overwrite
		| (0 << 10)  // EXTEN(2): External trigger enable and polarity selection for regular channels
		| (0 <<  5)  // EXTSEL(5): External trigger selection for regular group
//G4
		| (0 <<  3)  // RES(2): Data resolution, 0 = 12 bit, 1 = 10 bit, 2 = 8 bit, 3 = 6 bit
//H7 V
//		| (0 <<  2)  // RES(3): Data resolution, 0 = 16 bit, 6 = 12 bit, 3 = 10 bit, 7 = 8 bit, 3 = 6 bit

		| (1 <<  1)  // DMACFG: Direct memory access configuration, 0 = One shot mode, 1 = circular mode
		| (1 <<  0)  // DMAEN: Direct memory access enable
	;

	regs->CFGR2 = 0
		| (0 << 27)  // SMPTRIG: Sampling time control trigger mode
		| (0 << 26)  // BULB: Bulb sampling mode
		| (0 << 25)  // SWTRIG: Software trigger bit for sampling time control trigger mode
		| (0 << 16)  // GCOMP: Gain compensation mode
		| (0 << 10)  // ROVSM: Regular Oversampling mode
		| (0 <<  9)  // TROVS: Triggered Regular Oversampling
		| (0 <<  5)  // OVSS(4): Oversampling shift
		| (0 <<  2)  // OVSR(3): Oversampling ratio
		| (0 <<  1)  // JOVSE: Injected Oversampling Enable
		| (0 <<  0)  // ROVSE: Regular Oversampling Enable
	;

	// disable ADC Watchdogs
  #if defined(ADC_LTR_LT)
	  regs->LTR1 = 0;
	  regs->HTR1 = 0;
	  regs->LTR2 = 0;
	  regs->HTR2 = 0;
	  regs->LTR3 = 0;
	  regs->HTR3 = 0;
  #else
		regs->TR1 = 0;
		regs->TR2 = 0;
		regs->TR3 = 0;
  #endif

	regs->AWD2CR = 0;
	regs->AWD3CR = 0;

	// setup sampling time

	uint32_t stcode = 0; // sampling time code, index for the following array
	uint32_t sampling_clocks[8] = {3, 7, 13, 25, 48, 93, 248, 641};  // actually its 0.5 CLK less
	uint32_t target_sampling_clocks = (sampling_time_ns * 1000) / (adc_clock / 1000);
	while ((stcode < 7) && (sampling_clocks[stcode] < target_sampling_clocks))
	{
		++stcode;
	}

	int i;
	uint32_t tmp = 0;
	for (i = 0; i < 9; ++i)
	{
		tmp |= (stcode << (i * 3));
	}
	regs->SMPR1 = tmp;
	regs->SMPR2 = tmp;

	// total conversion cycles:  12 ADC clocks + sampling time
	conv_adc_clocks = sampling_clocks[stcode] + 12;
	act_conv_rate = adc_clock / conv_adc_clocks;

	// setup the regular sequence based on the channel map and start the cyclic conversion
	StartFreeRun(channel_map);

	initialized = true;
	return true;
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
	  | (1 <<  5)  // ALIGN: 0 = right data alignment, 1 = left data aligment
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
	StartFreeRun(channel_map);

	initialized = true;
	return true;
}

#else

#warning "ADC implementation is missing!"

#define ADC_IMPL_MISSING

bool THwAdc_stm32::Init(int adevnum, uint32_t achannel_map)
{
	return false;
}

#endif

#ifndef ADC_IMPL_MISSING

// Shared functions

#if defined(MCUSF_G4) || defined(MCUSF_H7) || defined(MCUSF_F3)
  #define HWADC_SQREG_SHIFT  6
  #define STM32_FASTADC
#else
  #define HWADC_SQREG_SHIFT  5
#endif

void THwAdc_stm32::SetupChannels(uint32_t achsel)
{
	channel_map = achsel;

	uint32_t ch;
	uint32_t sqr[4] = {0, 0, 0, 0};
	uint32_t * psqr = &sqr[0];
	uint32_t bitshift = 0;
#ifdef STM32_FASTADC
	bitshift = HWADC_SQREG_SHIFT;  // the sequence length is on the bits 0..3 here
#endif

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

			bitshift += HWADC_SQREG_SHIFT;
			if (bitshift > 25)
			{
				// go to the next register
				bitshift = 0;
				++psqr;
			}
		}
	}

#if defined(MCUSF_F0)
	regs->CHSELR = channel_map;

#elif defined(STM32_FASTADC)
	regs->SQR1 = sqr[0] | (dmadatacnt-1);  // this contains the sequence length too
	regs->SQR2 = sqr[1];
	regs->SQR3 = sqr[2];
	regs->SQR4 = sqr[3];

  #if defined(ADC_PCSEL_PCSEL_0)
	regs->PCSEL = channel_map;
  #endif

#else
	regs->SQR3 = sqr[0];
	regs->SQR2 = sqr[1];
	regs->SQR1 = (sqr[2] & 0x000FFFFF) | ((dmadatacnt-1) << 20);  // this contains the sequence length too
#endif
}

void THwAdc_stm32::StartFreeRun(uint32_t achsel)
{
	SetupChannels(achsel);

  // prepare the DMA transfer

	dmaxfer.bytewidth = 2;
	dmaxfer.count = dmadatacnt;
	dmaxfer.dstaddr = &dmadata[0];
	dmaxfer.flags = DMATR_CIRCULAR; // ST supports it !
	dmach.StartTransfer(&dmaxfer);

	StartContConv();
}

void THwAdc_stm32::StopFreeRun()
{
	dmach.Disable();

	// disable continuous mode
#if defined(MCUSF_F0)
	regs->CR |= (1 << 4); // stop the ADC
	while (regs->CR & (1 << 4))
	{
		// wait until stopped
	}
#elif defined(STM32_FASTADC)
	regs->CR |= ADC_CR_ADSTP;
	while (regs->CR & ADC_CR_ADSTP)
	{
		// wait until stopped
	}
#else
	regs->CR2 &= ~ADC_CR2_CONT;  // disable the continuous mode
	delay_us(10);
#endif
}

void THwAdc_stm32::StartContConv()
{
#if defined(MCUSF_F0)
	// and start the conversion
	regs->CR |= (1 << 2); // start the ADC
#elif defined(MCUSF_F1)
	// and start the conversion
	regs->CR2 |= ADC_CR2_CONT;  // enable the continuous mode
	regs->CR2 |= (ADC_CR2_SWSTART | ADC_CR2_ADON | ADC_CR2_EXTTRIG);
#elif defined(STM32_FASTADC)
	regs->CFGR |= ADC_CFGR_CONT;  // enable the continuous mode
	regs->CR |= ADC_CR_ADSTART;
#else
	// and start the conversion
	regs->CR2 |= ADC_CR2_CONT;  // enable the continuous mode
	regs->CR2 |= (ADC_CR2_SWSTART | ADC_CR2_ADON);
#endif
}


void THwAdc_stm32::StartRecord(uint32_t achsel, uint32_t acount, uint16_t * adstptr)
{
	StopFreeRun();

	SetupChannels(achsel); // this makes now a little bit more than what is necessary, but it is simpler this way

	dmaxfer.dstaddr = adstptr;
	dmaxfer.count = acount;
	dmaxfer.bytewidth = 2;
	dmaxfer.flags = 0; // not circular this time
	dmach.StartTransfer(&dmaxfer);

	StartContConv();
}

#endif
