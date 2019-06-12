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
 *  file:     hwpins_xmc.h
 *  brief:    XMC Pin/Pad and GPIO configuration
 *  version:  1.00
 *  date:     2018-02-10
 *  authors:  nvitya
*/

#ifndef _HWPINS_XMC_H
#define _HWPINS_XMC_H

#define HWPINS_PRE_ONLY
#include "hwpins.h"

// Extra control flags for XMC

#define PINCFG_HWC_MASK   0xF000000
#define PINCFG_HWC_SHIFT  24
#define PINCFG_HWC_0      0x8000000
#define PINCFG_HWC_1      0x9000000
#define PINCFG_HWC_2      0xA000000
#define PINCFG_HWC_3      0xB000000
#define PINCFG_HWC_4      0xC000000
#define PINCFG_HWC_5      0xD000000
#define PINCFG_HWC_6      0xE000000
#define PINCFG_HWC_7      0xF000000

class THwPinCtrl_xmc : public THwPinCtrl_pre
{
public:
	// platform specific
	bool PinSetup(int aportnum, int apinnum, unsigned flags);

	HW_GPIO_REGS * GetGpioRegs(int aportnum);

	bool GpioPortEnable(int aportnum);

	void GpioSet(int aportnum, int apinnum, int value);

	inline bool GpioSetup(int aportnum, int apinnum, unsigned flags)  { return PinSetup(aportnum, apinnum, flags); }

	void GpioIrqSetup(int aportnum, int apinnum, int amode); // not implemented yet
};

class TGpioPort_xmc : public TGpioPort_pre
{
public:
	void Assign(int aportnum);
	void Set(unsigned value);

public:
	HW_GPIO_REGS *       regs = nullptr;
	volatile unsigned *  portptr = nullptr;
};

class TGpioPin_xmc : public TGpioPin_common
{
public:
	HW_GPIO_REGS *   regs = nullptr;

	bool Setup(unsigned flags);
	void Assign(int aportnum, int apinnum, bool ainvert);

	void Toggle();

	void SwitchDirection(int adirection);
};

#define HWPINCTRL_IMPL   THwPinCtrl_xmc
#define HWGPIOPORT_IMPL  TGpioPort_xmc
#define HWGPIOPIN_IMPL   TGpioPin_xmc

#endif /* HWPINS_XMC_H_ */
