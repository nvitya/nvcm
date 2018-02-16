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
 *  file:     hwpins_lpc_v3.cpp
 *  brief:    LPC_V3 Pin/Pad and GPIO configuration
 *  version:  1.00
 *  date:     2018-02-10
 *  authors:  nvitya
*/

#include "platform.h"
#include "hwpins.h"

#define MAX_PORT_NUMBER  5

HW_GPIO_REGS * THwPinCtrl_lpc_v3::GetGpioRegs(int aportnum)
{
	if ((aportnum < 0) || (aportnum >= MAX_PORT_NUMBER))
	{
		return nullptr;
	}
	return (HW_GPIO_REGS *)GPIO;
}

bool THwPinCtrl_lpc_v3::PinSetup(int aportnum, int apinnum, unsigned flags)
{
	// LPC8xx version

	if ((aportnum < 0) || (aportnum >= MAX_PORT_NUMBER))
	{
		return false;
	}

	if ((apinnum < 0) || (apinnum > 31))
	{
		return false;
	}

	unsigned n = 0;
	unsigned * icptr;

	GPIO_Type * regs = GPIO;

	SYSCON->AHBCLKCTRLSET[0] = 1 << 13; // enable IOCON clock

	icptr = (unsigned *)(IOCON_BASE);
	icptr += ((aportnum << 5) + apinnum);

	if (flags & PINCFG_AF_MASK)
	{
		n |= (((flags & PINCFG_AF_MASK) >> PINCFG_AF_SHIFT) & 0x7);
	}

	if (flags & PINCFG_PULLDOWN)
	{
		n |= (1 << 4);
	}

	if (flags & PINCFG_PULLUP)
	{
		n |= (1 << 5);
	}

	if ((flags & PINCFG_ANALOGUE) == 0)
	{
		n |= (1 << 8);
	}

	if ((flags & PINCFG_SPEED_MASK) == PINCFG_SPEED_FAST)
	{
		n |= (1 << 10);
	}

	if (flags & PINCFG_OPENDRAIN)
	{
		n |= (1 << 11);
	}

	GpioPortEnable(aportnum);

	*icptr = n;

	unsigned bm = (1 << apinnum);

	// set GPIO direction

	if (flags & PINCFG_OUTPUT)
	{
	  // initial state
	  if (flags & PINCFG_GPIO_INIT_1)
	  {
	  	regs->SET[aportnum] = bm;
	  }
	  else
	  {
	  	regs->CLR[aportnum] = bm;
	  }

		regs->DIRSET[aportnum] = bm;
	}
	else
	{
		regs->DIRCLR[aportnum] = bm;
	}

  return true;
}

bool THwPinCtrl_lpc_v3::GpioPortEnable(int aportnum)
{
	if ((aportnum < 0) || (aportnum >= MAX_PORT_NUMBER))
	{
		return false;
	}

  SYSCON->AHBCLKCTRLSET[0] = (1 << (14 + aportnum));

  return true;
}

void THwPinCtrl_lpc_v3::GpioSet(int aportnum, int apinnum, int value)
{
	GPIO_Type * regs = GPIO;

	unsigned bm = (1 << apinnum);

  if (1 == value)
  {
  	regs->SET[aportnum] = bm;
  }
  else if (value & 2) // toggle
  {
  	regs->NOT[aportnum] = bm;
  }
  else
  {
  	regs->CLR[aportnum] = bm;
  }
}

// GPIO Port

void TGpioPort_lpc_v3::Assign(int aportnum)
{
	portnum = aportnum;
	portptr = (volatile unsigned *)(GPIO->PIN[aportnum]);
}

void TGpioPort_lpc_v3::Set(unsigned value)
{
	*portptr = value;
}

// GPIO Pin

void TGpioPin_lpc_v3::Assign(int aportnum, int apinnum, bool ainvert)
{
	regs = hwpinctrl.GetGpioRegs(aportnum);
	if (!regs)
	{
		return;
	}

  portnum = aportnum;
  pinnum = apinnum;
  inverted = ainvert;

	setbitvalue = (1 << pinnum);
	clrbitvalue = (1 << pinnum);
  getbitptr = (unsigned *)&(regs->PIN);
  getbitshift = apinnum;
  togglebitptr = (unsigned *)&(regs->NOT[portnum]);

  if (ainvert)
  {
    setbitptr = (unsigned *)&(regs->CLR[aportnum]);
    clrbitptr = (unsigned *)&(regs->SET[aportnum]);
  }
  else
  {
    setbitptr = (unsigned *)&(regs->SET[aportnum]);
    clrbitptr = (unsigned *)&(regs->CLR[aportnum]);
  }
}

void TGpioPin_lpc_v3::SwitchDirection(int adirection)
{
  if (adirection)
  {
  	regs->DIRSET[portnum] = setbitvalue;
  }
  else
  {
  	regs->DIRCLR[portnum] = setbitvalue;
  }
}
