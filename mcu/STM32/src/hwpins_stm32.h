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
 *  file:     hwpins_stm32.h
 *  brief:    STM32 Pin/Pad and GPIO configuration
 *  version:  1.00
 *  date:     2018-02-10
 *  authors:  nvitya
*/

#ifndef _HWPINS_STM32_H
#define _HWPINS_STM32_H

#define HWPINS_PRE_ONLY
#include "hwpins.h"

class THwPinCtrl_stm32 : public THwPinCtrl_pre
{
public:
	// platform specific
	bool PinSetup(int aportnum, int apinnum, unsigned flags);

	GPIO_TypeDef * GetGpioRegs(int aportnum);

	bool GpioPortEnable(int aportnum);

	void GpioSet(int aportnum, int apinnum, int value);

	inline bool GpioSetup(int aportnum, int apinnum, unsigned flags)  { return PinSetup(aportnum, apinnum, flags); }

	void GpioIrqSetup(int aportnum, int apinnum, int amode); // not implemented yet
};

class TGpioPort_stm32 : public TGpioPort_pre
{
public:
	void Assign(int aportnum);
	void Set(unsigned value);

public:
	GPIO_TypeDef *       regs = nullptr;
	volatile unsigned *  portptr = nullptr;
};

class TGpioPin_stm32 : public TGpioPin_common
{
public:
	GPIO_TypeDef *   regs = nullptr;
	unsigned *       togglebitptr = nullptr;

	bool Setup(unsigned flags);
	void Assign(int aportnum, int apinnum, bool ainvert);

	void Toggle();

	void SwitchDirection(int adirection);
};

#define HWPINCTRL_IMPL   THwPinCtrl_stm32
#define HWGPIOPORT_IMPL  TGpioPort_stm32
#define HWGPIOPIN_IMPL   TGpioPin_stm32

#endif /* HWPINS_STM32_H_ */
