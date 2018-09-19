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
 *  file:     d7s_595.cpp
 *  brief:    7 segment display module with 74HC595 (only GPIO required)
 *  version:  1.00
 *  date:     2018-09-19
 *  authors:  nvitya
 *  note:
 *    The digit multiplexing happens from the MCU side so
 *    when the update is stopped only one digit is shown.
*/

#include "platform.h"
#include "d7s_595.h"

bool Td7s_595::Init()
{
	state = 0;
	shiftstate = 0;
	regidx = 0;

	clock_sysclocks = SystemCoreClock / 1000000;  // 1 MHz clock
	digit_hold_clocks = digit_hold_time_us * (SystemCoreClock / 1000000);

	int i;
	for (i = 0; i < 8; ++i)
	{
		reg[i] = 0x00;
	}

	initialized = false;

	if (!pin_din.Assigned() || !pin_sclk.Assigned() || !pin_rclk.Assigned())
	{
		return false;
	}

	pin_din.Setup(PINCFG_OUTPUT | PINCFG_GPIO_INIT_0);
	pin_sclk.Setup(PINCFG_OUTPUT | PINCFG_GPIO_INIT_0);
	pin_rclk.Setup(PINCFG_OUTPUT | PINCFG_GPIO_INIT_0);

	initialized = true;

  return true;
}

/*      01
 *    20  02
 *      40
 *    10  04
 *      08
 *          80 (dot)
 */

static const unsigned char seg7_hexdigits[16] = {
//   0     1     2     3     4     5     6     7     8     9     A     B     C     D     E     F
  0x3F, 0x06, 0x5B, 0x4F, 0x66, 0x6D, 0x7D, 0x07, 0x7F, 0x6F, 0x77, 0x7C, 0x39, 0x5E, 0x79, 0x71
};

void Td7s_595::Set7Seg(uint8_t apos, uint8_t avalue)
{
	reg[apos & 7] = avalue;
}

void Td7s_595::SetHexDigit(uint8_t apos, uint8_t avalue)
{
	Set7Seg(apos, seg7_hexdigits[avalue & 0x0F]);
}

void Td7s_595::DisplayHexNum(uint32_t avalue)
{
  int i;
  unsigned v = avalue;
  for (i = 0; i < digit_count; ++i)
  {
  	SetHexDigit(i, v & 0x0F);
  	v = (v >> 4);
  }
}

void Td7s_595::DisplayDecNum(int avalue, bool zerofill)
{
  int v = (avalue < 0 ? -avalue : avalue);
  int m ;

  int i = 0;
  while (i < digit_count)
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

  if ((i < digit_count) && (avalue < 0))
  {
  	// negative sign
  	Set7Seg(i, 0x01);
  	++i;
  }

  while (i < digit_count) // blank the rest
  {
  	Set7Seg(i, 0x00);
  	++i;
  }
}


void Td7s_595::Run()
{
	int i;

	if (shiftstate > 0)
	{
		switch (shiftstate)
		{
			case 0:	// do nothing
				break;

			case 1:	// start shifting
				pin_din.Set0();
				pin_sclk.Set0();
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
				pin_din.SetTo(shiftdata >> (15 - shiftbit)); // MSB first
				//pin_din.SetTo(shiftdata >> shiftbit); // LSB first
				pin_sclk.Set0();
				sstarttime = CLOCKCNT;
				++shiftstate;
				break;

			case 4: // wait for clock rising edge time
				if (ELAPSEDCLOCKS(CLOCKCNT, sstarttime) >= clock_sysclocks / 2)
				{
					pin_sclk.Set1();
					++shiftstate;
				}
				break;

			case 5: // wait for clock cycle end time
				if (ELAPSEDCLOCKS(CLOCKCNT, sstarttime) >= clock_sysclocks)
				{
					++shiftbit;
					pin_sclk.Set0();

					if (shiftbit > 15)
					{
						sstarttime = CLOCKCNT;
						shiftstate = 10; // finished
					}
					else
					{
						shiftstate = 3; // next bit
					}
				}
				break;

			case 10:
				if (ELAPSEDCLOCKS(CLOCKCNT, sstarttime) >= clock_sysclocks / 2)
				{
					pin_rclk.Set1(); // latch the new value
					sstarttime = CLOCKCNT;
					shiftstate = 11;  // hold latch a little
				}
				break;

			case 11:
				if (ELAPSEDCLOCKS(CLOCKCNT, sstarttime) >= clock_sysclocks / 2)
				{
					pin_rclk.Set0();
					sstarttime = CLOCKCNT;
					shiftstate = 12;
				}
				break;

			case 12: // digit holding
				if (ELAPSEDCLOCKS(CLOCKCNT, sstarttime) >= digit_hold_clocks)
				{
					shiftstate = 0; // end digit
				}
				break;
		}
	}
	else
	{
		// continously updating all the digits

		++regidx;
		if (regidx >= digit_count)
		{
			regidx = 0;
			++scancount;
		}

		//shiftdata = (0x100 << (regidx & 7)) | 0xff; //reg[regidx];
		// lower part: positive logic digit selection
		// upper part: negative logic segments
		shiftdata = ((reg[regidx] << 8) ^ 0xFF00) | (1 << regidx);
		shiftstate = 1;
	}
}
