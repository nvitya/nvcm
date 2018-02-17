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
 *  file:     hwclkctrl_lpc_v2.h
 *  brief:    LPC_V2 MCU Clock / speed setup
 *  version:  1.00
 *  date:     2018-02-10
 *  authors:  nvitya
*/

#ifndef HWCLKCTRL_LPC_V2_H_
#define HWCLKCTRL_LPC_V2_H_

#define HWCLKCTRL_PRE_ONLY
#include "hwclkctrl.h"

class THwClkCtrl_lpc_v2 : public THwClkCtrl_pre
{
public:
	void StartExtOsc();
	bool ExtOscReady();

	void StartIntHSOsc();
	bool IntHSOscReady();

	void PrepareHiSpeed(unsigned acpuspeed);  // increase Flash wait states etc. for reliable high speed operation

	bool SetupPlls(bool aextosc, unsigned abasespeed, unsigned acpuspeed);
};

#define HWCLKCTRL_IMPL THwClkCtrl_lpc_v2

#endif // def HWCLKCTRL_LPC_V2_H_
