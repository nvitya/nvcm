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
 *  file:     hwclkctrl.cpp
 *  brief:    MCU Clock / speed setup vendor-independent definitions
 *  version:  1.00
 *  date:     2018-02-10
 *  authors:  nvitya
 *
 *  notes:
 *    The MCU usually starts with a slower internal oscillator. The THwClkCtrl class
 *    allows to set up the maximal speed. The vendor dependent implementations usually restrict
 *    the selectable clock speeds
*/

#include "platform.h"

#include "hwclkctrl.h"

THwClkCtrl  hwclkctrl;

unsigned SystemCoreClock = 0;

bool THwClkCtrl::InitCpuClock(unsigned abasespeed, unsigned acpuspeed)
{
  StartExtOsc();

  unsigned startupcounter = 0;

  while (!ExtOscReady() && (startupcounter < 50000))
  {
    startupcounter++;
  }

  if (!ExtOscReady())
  {
  	return false;
  }

  PrepareHiSpeed(acpuspeed); // adjust flash acceleration!

  if (!SetupPlls(true, abasespeed, acpuspeed))
  {
  	return false;
  }

  SetClockInfo(acpuspeed); // early initialization might require the clock speed but this will cleared at cpp init
  return true;
}

bool THwClkCtrl::InitCpuClockIntRC(unsigned abasespeed, unsigned acpuspeed)
{
  StartIntHSOsc();

  unsigned startupcounter = 0;

  while (!IntHSOscReady() && (startupcounter < 5000))
  {
    startupcounter++;
  }

  if (!IntHSOscReady())
  {
  	return false;
  }

  PrepareHiSpeed(acpuspeed); // adjust flash acceleration!

  if (!SetupPlls(false, abasespeed, acpuspeed))
  {
  	return false;
  }

  SetClockInfo(acpuspeed); // early initialization might require the clock speed but this will cleared at cpp init
  return true;
}

void THwClkCtrl::SetClockInfo(unsigned acpuspeed)
{
	SystemCoreClock = acpuspeed;  // update the standard CMSIS variable as well
}

