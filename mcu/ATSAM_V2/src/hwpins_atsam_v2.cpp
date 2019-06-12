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
 *  file:     hwpins_atsam_v2.cpp
 *  brief:    ATSAM_V2 Pin/Pad and GPIO configuration
 *  version:  1.00
 *  date:     2018-02-10
 *  authors:  nvitya
*/

#include "platform.h"
#include "hwpins.h"

#define MAX_PORT_NUMBER   PORT_GROUPS

HW_GPIO_REGS * THwPinCtrl_atsam_v2::GetGpioRegs(int aportnum)
{
	if ((aportnum < 0) || (aportnum >= MAX_PORT_NUMBER))
	{
		return nullptr;
	}
	else
	{
		return (HW_GPIO_REGS *)&(PORT->Group[aportnum]);
	}
}


bool THwPinCtrl_atsam_v2::PinSetup(int aportnum, int apinnum, unsigned flags)
{
	HW_GPIO_REGS * regs = GetGpioRegs(aportnum);
	if (!regs)
	{
		return false;
	}

	if ((apinnum < 0) || (apinnum > 31))
	{
		return false;
	}

	// 1. turn on port power
	GpioPortEnable(aportnum);

	// prepare pin configuration
	unsigned n = (1 << 1); // enable input

	if (flags & PINCFG_DRIVE_STRONG)
	{
		n |= (1 << 6);
	}

	if (flags & PINCFG_AF_MASK)
	{
		n |= (1 << 0);

		unsigned pmidx = (apinnum >> 1);
		unsigned pmshift = (apinnum & 1) * 4;

		regs->PMUX[pmidx].reg &= ~(0xF << pmshift);
		regs->PMUX[pmidx].reg |= (((flags >> PINCFG_AF_SHIFT) & 0xF) << pmshift);
	}
	else
	{
		// GPIO

		if (flags & PINCFG_OUTPUT)
		{
		  if (flags & PINCFG_GPIO_INIT_1)
		  {
		  	regs->OUTSET.reg = (1 << apinnum);
		  }
		  else
		  {
		  	regs->OUTCLR.reg = (1 << apinnum);
		  }
			regs->DIRSET.reg = (1 << apinnum);
		}
		else
		{
			regs->DIRCLR.reg = (1 << apinnum);
		}
	}

  regs->PINCFG[apinnum].reg = n;

  if (flags & PINCFG_PULLUP)
  {
  	regs->PINCFG[apinnum].reg |= (1 << 2);
  	regs->OUTSET.reg = (1 < apinnum);
  }

  if (flags & PINCFG_PULLDOWN)
  {
  	regs->PINCFG[apinnum].reg |= (1 << 2);
  	regs->OUTCLR.reg = (1 < apinnum);
  }

  return true;
}

bool THwPinCtrl_atsam_v2::GpioPortEnable(int aportnum)
{
	if ((aportnum < 0) || (aportnum >= MAX_PORT_NUMBER))
	{
		return false;
	}

#ifdef MCLK_APBBMASK_MASK
	MCLK->APBBMASK.reg |= MCLK_APBBMASK_PORT;
#else
	PM->APBBMASK.reg |= PM_APBBMASK_PORT;
#endif

  return true;
}

void THwPinCtrl_atsam_v2::GpioSet(int aportnum, int apinnum, int value)
{
	HW_GPIO_REGS * regs = (HW_GPIO_REGS *)&(PORT->Group[aportnum]);

  if (1 == value)
  {
  	regs->OUTSET.reg = (1 << apinnum);
  }
  else if (value & 2) // toggle
  {
  	regs->OUTTGL.reg = (1 << apinnum);
  }
  else
  {
  	regs->OUTCLR.reg = (1 << apinnum);
  }
}

void THwPinCtrl_atsam_v2::GpioIrqSetup(int aportnum, int apinnum, int amode)
{

}

// GPIO Port

void TGpioPort_atsam_v2::Assign(int aportnum)
{
	regs = hwpinctrl.GetGpioRegs(aportnum);
}

void TGpioPort_atsam_v2::Set(unsigned value)
{
	regs->OUT.reg = value;
}

// GPIO Pin

void TGpioPin_atsam_v2::Assign(int aportnum, int apinnum, bool ainvert)
{
	portnum = aportnum;
  pinnum = apinnum;
  inverted = ainvert;

	regs = hwpinctrl.GetGpioRegs(aportnum);
	if (!regs)
	{
		return;
	}

	setbitvalue = (1 << pinnum);
	clrbitvalue = (1 << pinnum);
  getbitptr = (unsigned *)&(regs->IN);
  getbitshift = pinnum;
  togglebitptr = (unsigned *)&(regs->OUTTGL.reg);

  if (ainvert)
  {
    setbitptr = (unsigned *)&(regs->OUTCLR);
    clrbitptr = (unsigned *)&(regs->OUTSET);
  }
  else
  {
    setbitptr = (unsigned *)&(regs->OUTSET);
    clrbitptr = (unsigned *)&(regs->OUTCLR);
  }
}

void TGpioPin_atsam_v2::SwitchDirection(int adirection)
{
	if (adirection)
	{
		regs->DIRSET.reg = setbitvalue;
	}
	else
	{
		regs->DIRCLR.reg = setbitvalue;
	}
}
