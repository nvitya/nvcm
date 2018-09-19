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

#include "platform.h"
#include "max7219.h"

bool Tmax7219::Init()
{
	state = 0;
	shiftstate = 0;
	regidx = 0;

	clock_sysclocks = SystemCoreClock / 1000000;  // 1 MHz clock

	int i;
	for (i = 0; i < 16; ++i)
	{
		reg[i] = 0x00;
	}

	reg[ 9] = 0; // Decode mode: 0 = no decode
	reg[10] = 8; // Intensity: 0..15, 8 = 50%
	reg[11] = 7; // Scan limit: 7 = all the digits
	reg[12] = 1; // Shutdown reg: 1 = normal operation
	reg[15] = 0; // Display test: 0 = normal operation, 1 = display test

	initialized = false;

	if (!dio_pin.Assigned() || !clk_pin.Assigned() || !cs_pin.Assigned())
	{
		return false;
	}

	cs_pin.Setup(PINCFG_OUTPUT | PINCFG_GPIO_INIT_1);
	dio_pin.Setup(PINCFG_OUTPUT | PINCFG_GPIO_INIT_0);
	clk_pin.Setup(PINCFG_OUTPUT | PINCFG_GPIO_INIT_0);

	initialized = true;

  return true;
}

/*      40
 *    02  20
 *      01
 *    04  10
 *      08
 *          80 (dot)
 */

static const unsigned char seg7_hexdigits[16] = {
//   0     1     2     3     4     5     6     7     8     9     A     B     C     D     E     F
  0x7E, 0x30, 0x6D, 0x79, 0x33, 0x5B, 0x5F, 0x70, 0x7F, 0x7B, 0x77, 0x1F, 0x0D, 0x3D, 0x4F, 0x47
};

void Tmax7219::Set7Seg(uint8_t apos, uint8_t avalue)
{
	reg[(apos & 7) + 1] = avalue;
}

void Tmax7219::SetHexDigit(uint8_t apos, uint8_t avalue)
{
	Set7Seg(apos, seg7_hexdigits[avalue & 0x0F]);
}

void Tmax7219::DisplayHexNum(uint32_t avalue)
{
  int i;
  unsigned v = avalue;
  for (i = 0; i < 8; ++i)
  {
  	SetHexDigit(i, v & 0x0F);
  	v = (v >> 4);
  }
}

void Tmax7219::DisplayDecNum(int avalue, bool zerofill)
{
  int v = (avalue < 0 ? -avalue : avalue);
  int m ;

  int i = 0;
  while (i < 8)
  {
  	m = v % 10;
  	SetHexDigit(i, m);
		++i;
  	v = v  / 10;
  	if (!zerofill && v == 0)
  	{
  		// stop here
  		break;
  	}
  }

  if ((i < 8) && (avalue < 0))
  {
  	// negative sign
  	Set7Seg(i, 0x01);
  	++i;
  }

  while (i < 8) // blank the rest
  {
  	Set7Seg(i, 0x00);
  	++i;
  }
}


void Tmax7219::Run()
{
	int i;

	if (shiftstate > 0)
	{
		switch (shiftstate)
		{
			case 0:	// do nothing
				break;

			case 1:	// start shifting
				cs_pin.Set0();
				dio_pin.Set0();
				clk_pin.Set0();
				shiftbit = 0;
				sstarttime = CLOCKCNT;
				++shiftstate;
				break;

			case 2: // wait a bit at start
				if (ELAPSEDCLOCKS(CLOCKCNT, sstarttime) >= clock_sysclocks)
				{
					++shiftstate;
				}
				break;

			case 3: // start data bit, setup clock
				dio_pin.SetTo(shiftdata >> (15 - shiftbit)); // MSB first
				clk_pin.Set0();
				sstarttime = CLOCKCNT;
				++shiftstate;
				break;

			case 4: // wait for clock rising edge time
				if (ELAPSEDCLOCKS(CLOCKCNT, sstarttime) >= clock_sysclocks / 2)
				{
					clk_pin.Set1();
					++shiftstate;
				}
				break;

			case 5: // wait for clock cycle end time
				if (ELAPSEDCLOCKS(CLOCKCNT, sstarttime) >= clock_sysclocks)
				{
					++shiftbit;

					if (shiftbit > 15)
					{
						clk_pin.Set0();
						sstarttime = CLOCKCNT;
						shiftstate = 10; // finished
					}
					else
					{
						shiftstate = 3; // next bit
					}
				}
				break;

			case 10: // hfinish transaction
				if (ELAPSEDCLOCKS(CLOCKCNT, sstarttime) >= clock_sysclocks / 2)
				{
					cs_pin.Set1();
					sstarttime = CLOCKCNT;
					shiftstate = 11;  // hold cs up a little
				}
				break;

			case 11: // finish transaction
				if (ELAPSEDCLOCKS(CLOCKCNT, sstarttime) >= clock_sysclocks / 2)
				{
					shiftstate = 0;  // stop shifting
				}
				break;
		}
	}
	else
	{
		// continously updating all the registers for the case of disconnect / reconnect

		// go to the next register

		++regidx;
		if (regidx == 13)  regidx = 15;  // skip non-existing registers
		if (regidx >= 16)
		{
			regidx = 1;
			++scancount;
		}

		shiftdata = (regidx << 8) | reg[regidx];
		shiftstate = 1;
	}
}
