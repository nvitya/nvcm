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
 *  file:     tm1638.h
 *  brief:    TM1638 chip support (only GPIO required)
 *  version:  1.00
 *  date:     2018-02-10
 *  authors:  nvitya
*/

#ifndef SRC_TM1638_H_
#define SRC_TM1638_H_

#include "hwpins.h"
#include "clockcnt.h"

class Ttm1638
{
public:
	int  state;
	int  nextstate;

	unsigned      clock_sysclocks;

	unsigned char outregs[16];
	unsigned char inregs[4];

	unsigned      scancounter;

	unsigned char ctrl;  // bits:  0000abbb, a = activate display, bbb = brightness

	TGpioPin      dio_pin;
	TGpioPin      clk_pin;
	TGpioPin      stb_pin;

	bool          initialized = false;

	bool          Init();
	void          Run();

protected:
	clockcnt_t		 sstarttime;

	int            shiftstate;
	int            shiftbit;
	int            shiftbyte;
	unsigned char  shiftdata[32];
	unsigned char  readdata[4];
	int            shiftdatacnt;
	int            readdatacnt;

/*
	void set_data(unsigned char avalue);
	void set_clock(unsigned char avalue);
	void set_strobe(unsigned char avalue);
	unsigned char get_data();
	void datadir_output(bool aoutput);
*/
};

#endif /* SRC_TM1638_H_ */
