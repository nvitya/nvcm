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
 *  file:     hwlcdctrl_stm32.h
 *  brief:    STM32 Integrated LCD controller implementation
 *  version:  1.00
 *  date:     2018-12-09
 *  authors:  nvitya
*/

#ifndef HWLCDCTRL_STM32_H_
#define HWLCDCTRL_STM32_H_

#define HWLCDCTRL_PRE_ONLY
#include "hwlcdctrl.h"

#if defined(LTDC_SRCR_IMR)

class THwLcdCtrl_stm32 : public THwLcdCtrl_pre
{
public:
	bool                  Init(uint16_t awidth, uint16_t aheight, void * aframebuffer);

public:
	LTDC_TypeDef *        regs = nullptr;
	LTDC_Layer_TypeDef *  lregs = nullptr;
};

#define HWLCDCTRL_IMPL THwLcdCtrl_stm32

#endif

#endif // def HWLCDCTRL_STM32_H_
