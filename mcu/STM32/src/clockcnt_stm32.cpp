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
 *  file:     clockcnt_stm32.cpp
 *  brief:    STM32 Clock Counter for M0 MCUs
 *  version:  1.00
 *  date:     2018-02-10
 *  authors:  nvitya
*/

#include "platform.h"

#if __CORTEX_M < 3

// clock timer initialization for Cortex-M0 processors

void clockcnt_init()
{
#if defined(TIM14)
	RCC->APB1ENR |= RCC_APB1ENR_TIM14EN;
  #define CCTIMER  TIM14

#else
	RCC->APB2ENR |= RCC_APB2ENR_TIM21EN;
  #define CCTIMER  TIM21
#endif

	CCTIMER->CR1 = 0;
	CCTIMER->PSC = 0; // count every clock
	CCTIMER->CR1 = 1;
	CCTIMER->EGR = 1; // reinit, start the timer
}

#endif

