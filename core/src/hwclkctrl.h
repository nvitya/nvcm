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
 *  file:     hwclkctrl.h
 *  brief:    MCU Clock / speed setup vendor-independent definitions
 *  version:  1.00
 *  date:     2018-02-10
 *  authors:  nvitya
*/

#ifndef HWCLKCTRL_H_PRE_
#define HWCLKCTRL_H_PRE_

class THwClkCtrl_pre
{
};

#endif // ndef HWCLKCTRL_H_PRE_

#ifndef HWCLKCTRL_PRE_ONLY

//-----------------------------------------------------------------------------

#ifndef HWCLKCTRL_H_
#define HWCLKCTRL_H_

#include "mcu_impl.h"

#ifndef HWCLKCTRL_IMPL

#warning "HWCLKCTRL is not implemented!"

class THwClkCtrl_noimpl : public THwClkCtrl_pre
{
public:
	void StartExtOsc()    { }
	bool ExtOscReady()    { return false; }

	void StartIntHSOsc()  { }
	bool IntHSOscReady()  { return false; }

	// increase Flash wait states etc. for reliable high speed operation
	void PrepareHiSpeed(unsigned acpuspeed)  { }

	bool SetupPlls(bool aextosc, unsigned abasespeed, unsigned acpuspeed)  { return false; }
};

#define HWCLKCTRL_IMPL  THwClkCtrl_noimpl

#endif // ndef HWCLKCTRL_IMPL

//-----------------------------------------------------------------------------

class THwClkCtrl : public HWCLKCTRL_IMPL
{
public:
	// Starts the external crystal oscillator and sets up the CPU (PLL) to the given speed
	bool InitCpuClock(unsigned abasespeed, unsigned acpuspeed);

	// Initializes the MCU clock using the internal oscillators
	bool InitCpuClockIntRC(unsigned abasespeed, unsigned acpuspeed);

   // Required to call when the oscillator setup was done before the cppinit
	void SetClockInfo(unsigned acpuspeed);
};

extern THwClkCtrl  hwclkctrl;

#endif // ndef HWCLKCTRL_H_

#else
  #undef HWCLKCTRL_PRE_ONLY
#endif

