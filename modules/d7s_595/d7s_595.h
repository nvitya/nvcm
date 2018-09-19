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
 *  file:     d7s_595.h
 *  brief:    7 segment display module with 74HC595 (only GPIO required)
 *  version:  1.00
 *  date:     2018-09-19
 *  authors:  nvitya
*/

#ifndef D7S_595_H_
#define D7S_595_H_

#include "stdint.h"
#include "hwpins.h"
#include "clockcnt.h"

class Td7s_595
{
public:
	TGpioPin      pin_sclk;
	TGpioPin      pin_rclk;
	TGpioPin      pin_din;

public:

	uint8_t        digit_count = 4;

	unsigned       digit_hold_time_us = 1000;  // 1 ms digit hold time

	bool           initialized = false;

	bool           Init();
	void           Run();

	void Set7Seg(uint8_t apos, uint8_t avalue);
	void SetHexDigit(uint8_t apos, uint8_t avalue);

	void DisplayHexNum(uint32_t avalue);
	void DisplayDecNum(int avalue, bool zerofill);

public:
	unsigned       clock_sysclocks;
	unsigned       digit_hold_clocks;
	unsigned       scancount = 0;

	int  					 state;
	uint8_t        regidx;
	unsigned			 sstarttime;

	int            shiftstate;
	int            shiftbit;
	uint32_t       shiftdata;

	uint8_t        reg[8];
};

#endif /* D7S_595_H_ */
