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
 *  file:     max7219.h
 *  brief:    MAX7219 chip support (only GPIO required)
 *  version:  1.00
 *  date:     2018-09-18
 *  authors:  nvitya
*/

#ifndef MAX7219_H_
#define MAX7219_H_

#include "stdint.h"
#include "hwpins.h"
#include "clockcnt.h"

class Tmax7219
{
public:
	TGpioPin      dio_pin;
	TGpioPin      clk_pin;
	TGpioPin      cs_pin;

public:

	unsigned      scancount = 0;

	bool          initialized = false;

	bool          Init();
	void          Run();

	void Set7Seg(uint8_t apos, uint8_t avalue);
	void SetHexDigit(uint8_t apos, uint8_t avalue);

	void DisplayHexNum(uint32_t avalue);
	void DisplayDecNum(int avalue, bool zerofill);

	void           SetIntensity(uint8_t avalue) { reg[10] = (avalue & 0x0F); }

public:
	unsigned       clock_sysclocks;

	int  					 state;
	uint8_t        regidx;
	unsigned			 sstarttime;

	int            shiftstate;
	int            shiftbit;
	uint16_t       shiftdata;

	uint8_t        reg[16];
};

#endif /* MAX7219_H_ */
