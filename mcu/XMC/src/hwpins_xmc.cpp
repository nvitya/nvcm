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
 *  file:     hwpins_xmc.cpp
 *  brief:    XMC Pin/Pad and GPIO configuration
 *  version:  1.00
 *  date:     2018-02-10
 *  authors:  nvitya
*/

#include "platform.h"
#include "hwpins.h"

#if defined(PORT6_BASE)
  #define MAX_PORT_NUMBER  7
#elif defined(PORT5_BASE)
  #define MAX_PORT_NUMBER  6
#elif defined(PORT4_BASE)
  #define MAX_PORT_NUMBER  5
#elif defined(PORT3_BASE)
  #define MAX_PORT_NUMBER  4
#elif defined(PORT2_BASE)
  #define MAX_PORT_NUMBER  3
#else
  #define MAX_PORT_NUMBER  2
#endif

HW_GPIO_REGS * THwPinCtrl_xmc::GetGpioRegs(int aportnum)
{
	if ((aportnum < 0) || (aportnum >= MAX_PORT_NUMBER))
	{
		return nullptr;
	}
	else
	{
		return (HW_GPIO_REGS *)(PORT0_BASE + (PORT1_BASE-PORT0_BASE)*(aportnum));
	}
}

bool THwPinCtrl_xmc::PinSetup(int aportnum, int apinnum, unsigned flags)
{
	// Infineon version

	if ((apinnum < 0) || (apinnum > 15))
	{
		return false;
	}

	HW_GPIO_REGS * gpiox = GetGpioRegs(aportnum);
	if (!gpiox)
	{
		return false;
	}

  unsigned cfg; // 8 bit configuration

#if UC_SERIES == XMC14 // 6 bit IO config
  unsigned cfgshift = 2;
#else // 5 bit IO config
  unsigned cfgshift = 3;
#endif

  // set mode register
	if (flags & PINCFG_AF_MASK)
	{
		cfg = 0x80 | ((((flags & PINCFG_AF_MASK) >> PINCFG_AF_SHIFT) & 0x0F) << cfgshift);

		if (flags & PINCFG_OPENDRAIN)
		{
			cfg |= 0x40;
		}

		flags |= PINCFG_GPIO_INIT_1;  // some peripherals require this !
	}
	else if (flags & PINCFG_OUTPUT)
	{
		cfg = 0x80;  // GP output
		if (flags & PINCFG_OPENDRAIN)
		{
			cfg |= 0x40;
		}
	}
  else
	{
		// input
	  cfg = 0;
	  if (flags & PINCFG_PULLUP)
	  {
	  	cfg |= (2 << cfgshift);
	  }

	  if (flags & PINCFG_PULLDOWN)
	  {
	  	cfg |= (1 << cfgshift);
	  }

	  // inverted input not supported...
	}

	unsigned ridx = (apinnum >> 2);
	unsigned rsh = ((apinnum & 3) * 8);
	gpiox->IOCR[ridx] &= ~(0xFF << rsh);
	gpiox->IOCR[ridx] |= (cfg << rsh);

	if (flags & PINCFG_ANALOGUE)
	{
		cfg = 1; // disable the digital input path
	}
	else
	{
		cfg = 0; // enable the digital input path too
	}

	if (((gpiox->PDISC >> apinnum) & 1) != cfg)
	{
		if (cfg)
		{
			gpiox->PDISC |= (1 << apinnum);
		}
		else
		{
		  gpiox->PDISC &= ~(1 << apinnum);
		}
	}

	// HW control (Infineon speciality)

	if (flags & PINCFG_HWC_MASK)
	{
		cfg = ((1 + (flags >> PINCFG_HWC_SHIFT)) & 0x3);
	}
	else
	{
		cfg = 0;
	}
	rsh = (apinnum * 2);
	gpiox->HWSEL &= ~(3 << rsh);
	gpiox->HWSEL |= (cfg << rsh);

  // initial state
  if (flags & PINCFG_GPIO_INIT_1)
  {
  	gpiox->OMR = (1 << apinnum);
  }
  else
  {
  	gpiox->OMR = (1 << apinnum) << 16;
  }

  return true;
}

bool THwPinCtrl_xmc::GpioPortEnable(int aportnum)
{
	if ((aportnum < 0) || (aportnum >= MAX_PORT_NUMBER))
	{
		return false;
	}
  return true;
}

void THwPinCtrl_xmc::GpioSet(int aportnum, int apinnum, int value)
{
	XMC_GPIO_PORT_t * gpiox = (XMC_GPIO_PORT_t *)(PORT0_BASE + (PORT1_BASE-PORT0_BASE)*(aportnum));
	unsigned bm = (1 << apinnum);

  if (1 == value)
  {
  	gpiox->OMR = bm;
  }
  else if (value & 2) // toggle
  {
  	gpiox->OMR = bm | (bm << 16);
  }
  else
  {
  	gpiox->OMR = (bm << 16);
  }
}

void THwPinCtrl_xmc::GpioIrqSetup(int aportnum, int apinnum, int amode)
{

}

// GPIO Port

void TGpioPort_xmc::Assign(int aportnum)
{
	if ((aportnum < 0) || (aportnum >= MAX_PORT_NUMBER))
	{
		regs = nullptr;
		return;
	}

	regs = (HW_GPIO_REGS *)(PORT0_BASE + (PORT1_BASE-PORT0_BASE)*(aportnum));
}

void TGpioPort_xmc::Set(unsigned value)
{
	regs->OUT = value;
}

// GPIO Pin

void TGpioPin_xmc::Assign(int aportnum, int apinnum, bool ainvert)
{
	regs = hwpinctrl.GetGpioRegs(aportnum);
	if (!regs)
	{
		return;
	}

	portnum = aportnum;
  pinnum = apinnum;
  inverted = ainvert;

  setbitptr = (unsigned *)&(regs->OMR);
  clrbitptr = (unsigned *)&(regs->OMR);
  getbitptr = (unsigned *)&(regs->IN);
  getoutbitptr = (unsigned *)&(regs->OUT);
  getbitshift = apinnum;

  if (ainvert)
  {
  	setbitvalue = (1 << pinnum) << 16;
  	clrbitvalue = (1 << pinnum);
  }
  else
  {
  	setbitvalue = (1 << pinnum);
  	clrbitvalue = (1 << pinnum) << 16;
  }
}

void TGpioPin_xmc::Toggle()
{
  unsigned pinbit = (1 << pinnum);
  regs->OMR = pinbit | (pinbit << 16);
}

void TGpioPin_xmc::SwitchDirection(int adirection)
{
	unsigned n;
	if (adirection)
	{
		n = 0x10; // push-pull output
	}
	else
	{
		n = 0;    // normal input
	}

	unsigned ridx = (pinnum >> 2);
	unsigned rsh = ((pinnum & 3) * 8);
	regs->IOCR[ridx] &= ~(0xFF << rsh);
	regs->IOCR[ridx] |= ((n << 3) << rsh);
}

