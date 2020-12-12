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
 *  file:     hwpwm_stm32.cpp
 *  brief:    STM32 PWM Driver
 *  version:  1.00
 *  date:     2019-04-08
 *  authors:  nvitya
*/

#include "hwpwm.h"

#include "stm32_utils.h"

#ifdef RCC_APB1ENR1_TIM2EN
  #define RCC_APB1ENR_TIM2EN   RCC_APB1ENR1_TIM2EN
  #define RCC_APB1ENR_TIM3EN   RCC_APB1ENR1_TIM3EN
  #define RCC_APB1ENR_TIM4EN   RCC_APB1ENR1_TIM4EN
  #define RCC_APB1ENR_TIM5EN   RCC_APB1ENR1_TIM5EN
  #define RCC_APB1ENR_TIM6EN   RCC_APB1ENR1_TIM6EN
  #define RCC_APB1ENR_TIM7EN   RCC_APB1ENR1_TIM7EN
#elif defined(RCC_APB1LENR_TIM2EN)
  #define RCC_APB1ENR_TIM2EN   RCC_APB1LENR_TIM2EN
  #define RCC_APB1ENR_TIM3EN   RCC_APB1LENR_TIM3EN
  #define RCC_APB1ENR_TIM4EN   RCC_APB1LENR_TIM4EN
  #define RCC_APB1ENR_TIM5EN   RCC_APB1LENR_TIM5EN
  #define RCC_APB1ENR_TIM6EN   RCC_APB1LENR_TIM6EN
  #define RCC_APB1ENR_TIM7EN   RCC_APB1LENR_TIM7EN
#endif

bool THwPwmChannel_stm32::Init(int atimernum, int achnum, int aoutnum) // outnum: 0 = A, 1 = B
{
	initialized = false;
	advanced_timer = false;

	devnum = atimernum;
	chnum = achnum;
	outnum = (aoutnum & 1);

	regs = nullptr;

  if (false)
  {

  }
#ifdef TIM1
	if (1 == devnum)
	{
		regs = TIM1;
		RCC->APB2ENR |= RCC_APB2ENR_TIM1EN;
		advanced_timer = true;
	}
#endif
#ifdef TIM2
	else if (2 == devnum)
	{
		regs = TIM2;
		APB1ENR_REGISTER |= RCC_APB1ENR_TIM2EN;
	}
#endif
#ifdef TIM3
	else if (3 == devnum)
	{
		regs = TIM3;
		APB1ENR_REGISTER |= RCC_APB1ENR_TIM3EN;
	}
#endif
#ifdef TIM4
	else if (4 == devnum)
	{
		regs = TIM4;
		APB1ENR_REGISTER |= RCC_APB1ENR_TIM4EN;
	}
#endif
#ifdef TIM5
	else if (5 == devnum)
	{
		regs = TIM5;
		APB1ENR_REGISTER |= RCC_APB1ENR_TIM5EN;
	}
#endif
#ifdef TIM6
	else if (6 == devnum)
	{
		regs = TIM6;
		APB1ENR_REGISTER |= RCC_APB1ENR_TIM6EN;
	}
#endif
#ifdef TIM7
	else if (7 == devnum)
	{
		regs = TIM7;
		APB1ENR_REGISTER |= RCC_APB1ENR_TIM7EN;
	}
#endif
#ifdef TIM8
	else if (8 == devnum)
	{
		regs = TIM8;
		RCC->APB2ENR |= RCC_APB2ENR_TIM8EN;
		advanced_timer = true;
	}
#endif
#ifdef TIM9
	else if (9 == devnum)
	{
		regs = TIM9;
		RCC->APB2ENR |= RCC_APB2ENR_TIM9EN;
	}
#endif
#ifdef TIM10
	else if (10 == devnum)
	{
		regs = TIM10;
		RCC->APB2ENR |= RCC_APB2ENR_TIM10EN;
	}
#endif
#ifdef TIM11
	else if (11 == devnum)
	{
		regs = TIM11;
		RCC->APB2ENR |= RCC_APB2ENR_TIM11EN;
	}
#endif

	if (!regs)
	{
		return false;
	}

	if (1 == chnum)
	{
		valreg = (volatile uint32_t *)&(regs->CCR1);
	}
	else if (2 == chnum)
	{
		valreg = (volatile uint32_t *)&(regs->CCR2);
	}
	else if (3 == chnum)
	{
		valreg = (volatile uint32_t *)&(regs->CCR3);
	}
	else if (4 == chnum)
	{
		valreg = (volatile uint32_t *)&(regs->CCR4);
	}
	else
	{
		return false;
	}

	chpos = (chnum - 1);


	regs->SMCR = 0;

	SetFrequency(frequency);

	*valreg = 0;

	// setup the capture and compare mode register

	uint32_t chv = 0
		| (0 <<  7)  // OCxCE: Output compare 1 clear enable
		| (6 <<  4)  // OCxM(3): Output compare 1 mode, 6 = PWM mode 1
		| (1 <<  3)  // OCxPE: 0 = can be written any time, 1 = CCR activated at each update event
		| (0 <<  2)  // OCxFE: Output compare 1 fast enable
		| (0 <<  0)  // CCxS(2): Capture/Compare 1 selection, 0 = CHx is output
	;

	uint32_t vshift = 8 * (chpos & 1);

	volatile uint32_t * ccmr;
	if (chnum >= 3)
	{
		ccmr = (volatile uint32_t *)&regs->CCMR2;
	}
	else
	{
		ccmr = (volatile uint32_t *)&regs->CCMR1;
	}

	*ccmr &= ~(0xFF << vshift);
	*ccmr |=  (chv  << vshift);

	outenbit = (1 << (chpos << 2));


	uint32_t ccer =	regs->CCER;
	ccer &= ~(0xF << (chpos << 2)); // set normal polarity, disable output
	if (inverted)
	{
		ccer |= (2 << (chpos << 2));  // set inverted polarity
	}
	regs->CCER = ccer;

	regs->DCR = 0;
	//regs->AF1 = 0;
	//regs->AF2 = 0;
	regs->DIER = 0;

	if (advanced_timer)
	{
		regs->BDTR = (0
		  | (1 << 15)  // MOE: main output enable
		  | (1 << 14)  // AOE: automatic output enable
		  | (0 << 12)  // BKE: 0 = no brake handling
		);
	}

	regs->CR1 = 0
	  | (0 <<  8)  // CKD(2): clock division
	  | (0 <<  7)  // ARPE: Auto-reload preload enable, 1 = buffered ARR
	  | (0 <<  5)  // CMS(2): Center aligned mode, 0 = edge aligned
	  | (0 <<  4)  // DIR: 0 = upcounter, 1 = downcounter
	  | (0 <<  3)  // OPM: one pulse mode
	  | (0 <<  2)  // URS: Update request source
	  | (0 <<  1)  // UDIS: Update disable
	  | (1 <<  0)  // CEN: Counter enable
	;

	regs->CR2 = 0;

	regs->EGR = 1;

	initialized = true;
	return true;
}

void THwPwmChannel_stm32::SetFrequency(uint32_t afrequency)
{
	frequency = afrequency;

	uint32_t baseclock = SystemCoreClock;

	uint32_t prescaler = 0;
	do
	{
		++prescaler;
		periodclocks = baseclock / (prescaler * frequency);
	}
	while (periodclocks > 65535);

	regs->PSC = prescaler - 1; // prescaler
	regs->ARR = periodclocks;
}


void THwPwmChannel_stm32::SetOnClocks(uint16_t aclocks)
{
	*valreg = aclocks;
	regs->EGR = (1 << 0);  // UG
}

void THwPwmChannel_stm32::Enable()
{
	regs->CCER |= outenbit;
}

void THwPwmChannel_stm32::Disable()
{
	regs->CCER &= ~outenbit;
}

