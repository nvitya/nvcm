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
 *  file:     tm1637.h
 *  brief:    TM1637 chip support (only GPIO required)
 *  version:  1.00
 *  date:     2018-02-10
 *  authors:  nvitya
 *  note:
 *    Data read not supported (yet)
*/

#ifndef SRC_TM1637_H_
#define SRC_TM1637_H_

#include "hwpins.h"
#include "clockcnt.h"

class Ttm1637
{
public:
	int  state;
	int  nextstate;

	unsigned      clock_sysclocks;

	unsigned char outregs[6] = {0,0,0,0,0,0};

	unsigned      scancounter = 0;

	unsigned char ctrl = 0x0F;  // bits:  0000abbb, a = activate display, bbb = brightness

	TGpioPin      dio_pin;
	TGpioPin      clk_pin;

	bool          initialized = false;

	bool          Init();
	void          Run();

protected:
	clockcnt_t		 sstarttime;

	int            shiftstate;
	int            shiftbit;
	int            shiftbyte;
	unsigned char  shiftdata[16];
	unsigned char  readdata[4];
	int            shiftdatacnt;
	int            readdatacnt;
};

#endif /* SRC_TM1637_H_ */
