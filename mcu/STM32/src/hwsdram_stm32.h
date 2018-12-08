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
 *  file:     hwsdram_stm32.h
 *  brief:    STM32 SDRAM controller
 *  version:  1.00
 *  date:     2018-11-27
 *  authors:  nvitya
*/

#ifndef HWSDRAM_STM32_H_
#define HWSDRAM_STM32_H_

#define HWSDRAM_PRE_ONLY
#include "hwsdram.h"

#define HW_SDRAM_REGS    FMC_Bank5_6_TypeDef

class THwSdram_stm32 : public THwSdram_pre
{
public:  // special ST settings
	uint8_t          bank = 1;
	uint32_t         hclk_div = 2;
	uint32_t         rpipe_delay = 0;
	bool             read_burst_enable = true;

public:
	HW_SDRAM_REGS *  regs = nullptr;

	bool InitHw();

	void Cmd_ClockEnable();
	void Cmd_AllBankPrecharge();
	void Cmd_AutoRefresh(int acount);
	void Cmd_LoadModeRegister(uint16_t aregvalue);
	void Cmd_LoadExtModeRegister(uint16_t aregvalue);
	void SetNormalMode();
	void SetRefreshTime(uint32_t atime_ns);

public: // helper function
	void SendCommand(uint16_t command, uint32_t mrdata, uint32_t refrcnt);

};

#define HWSDRAM_IMPL THwSdram_stm32

#endif /* HWSDRAM_STM32_H_ */
