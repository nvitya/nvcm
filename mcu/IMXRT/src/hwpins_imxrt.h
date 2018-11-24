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
 *  file:     hwpins_imxrt.h
 *  brief:    IMXRT Pin/Pad and GPIO configuration
 *  version:  1.00
 *  date:     2018-11-23
 *  authors:  nvitya
*/

#ifndef _HWPINS_IMXRT_H
#define _HWPINS_IMXRT_H

#define HWPINS_PRE_ONLY
#include "hwpins.h"

#include "imxrt_pads.h"

#if defined(MCUSF_1020)
  #include "imxrt_iomuxc_1020.h"
#else
  #error "subfamily not supported."
#endif

#define MAX_GPIO_PORT_NUMBER  5

class THwPinCtrl_imxrt : public THwPinCtrl_pre
{
public:
	// IMXRT specific, best used with the definitions in imxrt_pads.h
	bool PadSetup(int padnum, unsigned flags);

	// IMXRT special for the defines in the imxrt_iomuxc.h, do not use for GPIO
	void PadSetup(uint32_t muxreg, uint8_t muxmode, uint32_t inputreg, uint8_t daisy, uint32_t confreg, unsigned flags);

	void SetPadControl(unsigned * padctrlreg, unsigned flags);

	bool InputSelect(int afuncid, unsigned ainput); // IMXRT Specific for INPUT DAISY registers

	HW_GPIO_REGS * GetGpioRegs(int aportnum);

	bool GpioPortEnable(int aportnum);

	void GpioSet(int aportnum, int apinnum, int value);

	bool GpioSetup(int aportnum, int apinnum, unsigned flags);
};

class TGpioPort_imxrt : public TGpioPort_pre
{
public:
	HW_GPIO_REGS *   regs = nullptr;
	unsigned *       drshadow;

	void Assign(int aportnum);
	void Set(unsigned value);

public:
	volatile unsigned * portptr = nullptr;
};

class TGpioPin_imxrt : public TGpioPin_common
{
public:
	HW_GPIO_REGS *   regs = nullptr;
	unsigned *       togglebitptr = nullptr;

	bool Setup(unsigned flags);
	void Assign(int aportnum, int apinnum, bool ainvert);

	inline void Toggle() { *togglebitptr = setbitvalue; }

	void SwitchDirection(int adirection);
};

#define HWPINCTRL_IMPL   THwPinCtrl_imxrt
#define HWGPIOPORT_IMPL  TGpioPort_imxrt
#define HWGPIOPIN_IMPL   TGpioPin_imxrt

#endif /* HWPINS_IMXRT_H_ */
