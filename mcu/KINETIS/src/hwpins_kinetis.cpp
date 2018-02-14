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
 *  file:     hwpins_kinetis.cpp
 *  brief:    KINETIS Pin/Pad and GPIO configuration
 *  version:  1.00
 *  date:     2018-02-10
 *  authors:  nvitya
*/

#include "platform.h"
#include "hwpins.h"

#if defined(PORTH)
  #define MAX_PORT_NUMBER 8
#elif defined(PORTG)
  #define MAX_PORT_NUMBER 7
#elif defined(PORTF)
  #define MAX_PORT_NUMBER 6
#elif defined(PORTE)
  #define MAX_PORT_NUMBER 5
#elif defined(PORTD)
  #define MAX_PORT_NUMBER 4
#elif defined(PORTC)
  #define MAX_PORT_NUMBER 3
#else
  #define MAX_PORT_NUMBER 2
#endif

HW_GPIO_REGS * THwPinCtrl_kinetis::GetGpioRegs(int aportnum)
{
	if ((aportnum < 0) || (aportnum >= MAX_PORT_NUMBER))
	{
		return nullptr;
	}

	return (HW_GPIO_REGS *)(GPIOA_BASE + aportnum*(GPIOB_BASE - GPIOA_BASE));
}

bool THwPinCtrl_kinetis::PinSetup(int aportnum, int apinnum, unsigned flags)
{
	HW_GPIO_REGS * gpio = GetGpioRegs(aportnum);
	if (!gpio)
	{
		return false;
	}

	if ((apinnum < 0) || (apinnum > 15))
	{
		return false;
	}

	GpioPortEnable(aportnum);

	unsigned cfg = 0;

	if (flags & PINCFG_AF_MASK)
	{
		cfg |= (((flags >> PINCFG_AF_SHIFT) & 7) << 8);
	}
	else
	{
		// GPIO
		cfg |= (1 << 8);
	}

	if (flags & PINCFG_DRIVE_STRONG)
	{
		cfg |= (1 << 6);
	}

	if (flags & PINCFG_OPENDRAIN)
	{
		cfg |= (1 << 5);
	}

	if ((flags & PINCFG_SPEED_MASK) == PINCFG_SPEED_SLOW)
	{
		cfg |= (1 << 4); // enable passive filter
		cfg |= (1 << 3); // slow slew rate
	}

	if (flags & PINCFG_PULLUP)
	{
		cfg |= (3 << 0);
	}
	else if (flags & PINCFG_PULLDOWN)
	{
		cfg |= (2 << 0);
	}

	PORT_Type * port = (PORT_Type *)(PORTA_BASE + aportnum*(PORTB_BASE - PORTA_BASE));
	port->PCR[apinnum] = cfg;

  // initial state
  if (flags & PINCFG_GPIO_INIT_1)
  {
  	gpio->PSOR = (1 << apinnum);
  }
  else
  {
  	gpio->PCOR = (1 << apinnum);
  }

  if (flags & PINCFG_OUTPUT)
  {
  	gpio->PDDR |= (1 << apinnum);
  }
  else
  {
  	gpio->PDDR &= ~(1 << apinnum);
  }

  return true;
}

bool THwPinCtrl_kinetis::GpioPortEnable(int aportnum)
{
	if ((aportnum < 0) || (aportnum >= MAX_PORT_NUMBER))
	{
		return false;
	}

	SIM->SCGC5 |= (1 << (9 + aportnum));
  return true;
}

void THwPinCtrl_kinetis::GpioSet(int aportnum, int apinnum, int value)
{
	HW_GPIO_REGS * gpio = GetGpioRegs(aportnum);
	if (!gpio)
	{
		return;
	}

  if (1 == value)
  {
  	gpio->PSOR = (1 << apinnum);
  }
  else if (value & 2) // toggle
  {
  	gpio->PTOR = (1 << apinnum);
  }
  else
  {
  	gpio->PCOR = (1 << apinnum);
  }
}

// GPIO Port

void TGpioPort_kinetis::Assign(int aportnum)
{
	portnum = aportnum;
	HW_GPIO_REGS * regs = hwpinctrl.GetGpioRegs(portnum);
	portptr = (volatile unsigned *)&regs->PDOR;
}

void TGpioPort_kinetis::Set(unsigned value)
{
	*portptr = value;
}

// GPIO Pin

void TGpioPin_kinetis::Assign(int aportnum, int apinnum, bool ainvert)
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
  getbitptr = (unsigned *)&(regs->PDIR);
  togglebitptr = (unsigned *)&(regs->PTOR);
  getbitshift = apinnum;

  if (ainvert)
  {
    setbitptr = (unsigned *)&(regs->PCOR);
    clrbitptr = (unsigned *)&(regs->PSOR);
  }
  else
  {
    setbitptr = (unsigned *)&(regs->PSOR);
    clrbitptr = (unsigned *)&(regs->PCOR);
  }
}

void TGpioPin_kinetis::SwitchDirection(int adirection)
{
  if (adirection)
  {
  	regs->PDDR |= (1 << pinnum);
  }
  else
  {
  	regs->PDDR &= ~(1 << pinnum);
  }
}

