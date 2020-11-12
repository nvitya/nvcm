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
 *  file:     hwpwm_stm32.h
 *  brief:    STM32 PWM Driver
 *  version:  1.00
 *  date:     2019-04-08
 *  authors:  nvitya
*/


#ifndef SRC_HWPWM_STM32_H_
#define SRC_HWPWM_STM32_H_

#define HWPWM_PRE_ONLY
#include "hwpwm.h"

class THwPwmChannel_stm32 : public THwPwmChannel_pre
{
public:
	bool          Init(int atimernum, int achnum, int aoutnum);

	void          SetOnClocks(uint16_t aclocks);
	void          Enable();
	void          Disable();
	inline bool   Enabled() { return ((regs->CCER & outenbit) != 0); }

	void          SetFrequency(uint32_t afrequency);

public:
	TIM_TypeDef *           regs = nullptr;

	uint32_t                chpos = 0;     // = chnum - 1
	uint32_t						    outenbit = 0;
	volatile uint32_t *     valreg = nullptr;
};

#define HWPWM_IMPL THwPwmChannel_stm32

#endif /* SRC_HWPWM_STM32_H_ */
