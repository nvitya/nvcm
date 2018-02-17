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
 *  file:     hwpins_lpc_v2.cpp
 *  brief:    LPC_V2 Pin/Pad and GPIO configuration
 *  version:  1.00
 *  date:     2018-02-10
 *  authors:  nvitya
*/

#include "platform.h"
#include "hwpins.h"

static const unsigned char lpc82x_iocon_offsets[32] =
{
	0x044, // 0
	0x02C, // 1
	0x018, // 2
	0x014, // 3
	0x010, // 4
	0x00C, // 5
	0x040, // 6
	0x03C, // 7
	0x038, // 8
	0x034, // 9
	0x020, // 10
	0x01C, // 11
	0x008, // 12
	0x004, // 13
	0x048, // 14
	0x028, // 15
	0x024, // 16
	0x000, // 17
	0x078, // 18
	0x074, // 19
	0x070, // 20
	0x06C, // 21
	0x068, // 22
	0x064, // 23
	0x060, // 24
	0x05C, // 25
	0x058, // 26
	0x054, // 27
	0x050, // 28
// not available
	0xFF,  // 29
	0xFF,  // 30
	0xFF   // 31
};

#define MAX_PORT_NUMBER  2

bool THwPinCtrl_lpc_v2::PinFuncConnect(int afuncid, int aportnum, int apinnum)
{
	if (apinnum > 31)
	{
		return false;
	}
	if (aportnum >= MAX_PORT_NUMBER)
	{
		return false;
	}

	unsigned regindex = (afuncid >> 2);
	unsigned regshift = ((afuncid & 3) << 3);
	unsigned pinid = (((aportnum << 5) + apinnum) & 0xFF);
	if (apinnum < 0)  pinid = 0xFF;

	LPC_SYSCON->SYSAHBCLKCTRL |= (1 << 7); // enable SWM clock

	LPC_SWM->PINASSIGN[regindex] &= ~(0xFF << regshift);
	LPC_SWM->PINASSIGN[regindex] |=  (pinid << regshift);

	return true;
}

HW_GPIO_REGS * THwPinCtrl_lpc_v2::GetGpioRegs(int aportnum)
{
	if ((aportnum < 0) || (aportnum >= MAX_PORT_NUMBER))
	{
		return nullptr;
	}

	return (HW_GPIO_REGS *)LPC_GPIO_PORT_BASE;
}

bool THwPinCtrl_lpc_v2::PinSetup(int aportnum, int apinnum, unsigned flags)
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

	LPC_GPIO_PORT_TypeDef * regs = (LPC_GPIO_PORT_TypeDef *)LPC_GPIO_PORT_BASE;

	unsigned ioconoffs = lpc82x_iocon_offsets[apinnum];
	if (ioconoffs == 0xFF)
	{
		return false;
	}

	LPC_SYSCON->SYSAHBCLKCTRL |= (1 << 18); // enable IOCON clock

	icptr = (unsigned *)(LPC_IOCON_BASE + ioconoffs);

	if (flags & PINCFG_PULLDOWN)
	{
		n |= (1 << 3);
	}
	if (flags & PINCFG_PULLUP)
	{
		n |= (1 << 4);
	}

	if (flags & PINCFG_OPENDRAIN)
	{
		n |= (1 << 10);
	}

	GpioPortEnable(aportnum);

	*icptr = n;

	unsigned bm = (1 << apinnum);

	// set GPIO direction

	if (flags & PINCFG_OUTPUT)
	{
		regs->DIRSET[aportnum] = bm;
	}
	else
	{
		regs->DIRCLR[aportnum] = bm;
	}

  // initial state
  if (flags & PINCFG_GPIO_INIT_1)
  {
  	regs->SET[aportnum] = bm;
  }
  else
  {
  	regs->CLR[aportnum] = bm;
  }

  return true;
}

bool THwPinCtrl_lpc_v2::GpioPortEnable(int aportnum)
{
	if ((aportnum < 0) || (aportnum >= MAX_PORT_NUMBER))
	{
		return false;
	}

	LPC_SYSCON->SYSAHBCLKCTRL |= (1 << 6);

  return true;
}

void THwPinCtrl_lpc_v2::GpioSet(int aportnum, int apinnum, int value)
{
	LPC_GPIO_PORT_TypeDef * regs = (LPC_GPIO_PORT_TypeDef *)LPC_GPIO_PORT_BASE;

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

void TGpioPort_lpc_v2::Assign(int aportnum)
{
	portnum = aportnum;
	portptr = (volatile unsigned *)(LPC_GPIO_PORT->PIN[aportnum]);
}

void TGpioPort_lpc_v2::Set(unsigned value)
{
	*portptr = value;
}

// GPIO Pin

void TGpioPin_lpc_v2::Assign(int aportnum, int apinnum, bool ainvert)
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

void TGpioPin_lpc_v2::SwitchDirection(int adirection)
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
