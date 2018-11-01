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
 *  file:     tm1637.cpp
 *  brief:    TM1637 chip support (only GPIO required)
 *  version:  1.00
 *  date:     2018-02-10
 *  authors:  nvitya
 *  note:
 *    Data read not supported (yet)
*/

#include "platform.h"
#include "tm1637.h"

bool Ttm1637::Init()
{
	state = 0;
	nextstate = 0;
	shiftstate = 0;

	scancounter = 0;

	clock_sysclocks = (SystemCoreClock / 20000);  // ~10 kHz clock

	initialized = false;

	if (!dio_pin.Assigned() || !clk_pin.Assigned())
	{
		return false;
	}

	dio_pin.Setup(PINCFG_OUTPUT | PINCFG_GPIO_INIT_0 | PINCFG_PULLUP);
	clk_pin.Setup(PINCFG_OUTPUT | PINCFG_GPIO_INIT_0 | PINCFG_PULLUP);

	// let the external resistor pull up the lines
  dio_pin.SwitchDirection(0);
  clk_pin.SwitchDirection(0);

	initialized = true;

  return true;
}

void Ttm1637::Run()
{
	int i;

	if (shiftstate > 0)
	{
		switch (shiftstate)
		{
			case 0:	// do nothing
				break;

			case 1:	// make start condition (DIO=0, CLK=1)
				dio_pin.SwitchDirection(1); // pull down the line
				shiftbit = 0;
				shiftbyte = 0;
				sstarttime = CLOCKCNT;
				++shiftstate;
				break;

			case 2: // hold start condition
				if (CLOCKCNT - sstarttime >= clock_sysclocks)
				{
					++shiftstate;
				}
				break;

			case 3: // CLOCK low
				clk_pin.SwitchDirection(1); // pull down the CLK line
				sstarttime = CLOCKCNT;
				++shiftstate;
				break;

			case 4: // hold clock low
				if (CLOCKCNT - sstarttime >= clock_sysclocks / 2)
				{
					++shiftstate;
				}
				break;

			case 5: // set data bit
				if ((shiftdata[shiftbyte] >> shiftbit) & 1)
				{
					dio_pin.SwitchDirection(0);
				}
				else
				{
					dio_pin.SwitchDirection(1);
				}
				sstarttime = CLOCKCNT;
				++shiftstate;
				break;

			case 6: // wait half clock
				if (CLOCKCNT - sstarttime >= clock_sysclocks / 2)
				{
					++shiftstate;
				}
				break;

			case 7: // CLOCK high
				clk_pin.SwitchDirection(0);
				sstarttime = CLOCKCNT;
				++shiftstate;
				break;

			case 8: // hold clock high
				if (CLOCKCNT - sstarttime >= clock_sysclocks)
				{
					if (shiftbit >= 7)
					{
						++shiftstate; // go to byte ack
					}
					else
					{
						++shiftbit;
						shiftstate = 3; // next bit
					}
				}
				break;

			case 9: // byte ack
				clk_pin.SwitchDirection(1); // pull down the CLK line
				dio_pin.SwitchDirection(0); // DIO=1 / input
				sstarttime = CLOCKCNT;
				++shiftstate;
				break;

			case 10: // hold byte ack
				if (CLOCKCNT - sstarttime >= clock_sysclocks * 2)
				{
					++shiftstate;
				}
				break;

			case 11: // CLK high
				clk_pin.SwitchDirection(0);
				sstarttime = CLOCKCNT;
				++shiftstate;
				break;

			case 12: // check ack
				if (CLOCKCNT - sstarttime >= clock_sysclocks)
				{
					// TODO: check ack
					//   x = dio_pin.Value();
					dio_pin.SwitchDirection(1); // DIO=0
					++shiftstate;
				}
				break;

			case 13: // CLK low
				clk_pin.SwitchDirection(1);
				sstarttime = CLOCKCNT;
				++shiftstate;
				break;

			case 14: // byte finished
				if (CLOCKCNT - sstarttime >= clock_sysclocks)
				{
					// byte finished.
					++shiftbyte;
					if (shiftbyte >= shiftdatacnt)
					{
						shiftstate = 32; // write finished
					}
					else
					{
						shiftbit = 0;
						shiftstate = 5;
					}
				}
				break;

			case 32: // send stop condition
				dio_pin.SwitchDirection(1);  // DIO=0
				sstarttime = CLOCKCNT;
				++shiftstate;
				break;

			case 33:
				if (CLOCKCNT - sstarttime >= clock_sysclocks)
				{
					++shiftstate;
				}
				break;

			case 34: // CLK=1
				clk_pin.SwitchDirection(0);
				sstarttime = CLOCKCNT;
				++shiftstate;
				break;

			case 35:
				if (CLOCKCNT - sstarttime >= clock_sysclocks)
				{
					++shiftstate;
				}
				break;

			case 36: // DIO=1
				dio_pin.SwitchDirection(0);
				sstarttime = CLOCKCNT;
				++shiftstate;
				break;

			case 37:
				if (CLOCKCNT - sstarttime >= clock_sysclocks)
				{
					shiftstate = 0; // go to idle
					state = nextstate; // set main state
				}
				break;
		}
	}
	else
	{
		// handle main state

		nextstate = state + 1;  // step forward by default

		switch (state)
		{
			case 0:	// prepare update output registers
				shiftdata[0] = 0x40;  // write data to display with autoincrement address
				shiftdatacnt = 1;
				readdatacnt = 0;
				shiftstate = 1;
				break;

			case 1:	// update output registers
				shiftdata[0] = 0xC0;  // set address + data

				for (i = 0; i < 6; ++i)
				{
					shiftdata[i+1] = outregs[i];
				}

				// start shifting
				shiftdatacnt = 7;
				readdatacnt = 0;
				shiftstate = 1;
				break;

			case 2:  // set control register
				shiftdata[0] = 0x80 | (ctrl & 0x0F);
				shiftdatacnt = 1;
				readdatacnt = 0;
				shiftstate = 1;
				break;

			case 3: // end of scanning
				++scancounter; // increment the scan counter to help processing the buttons
				//state = 0;
				++state;
				break;

			case 4: // some pause for simpler scope analysis
				sstarttime = CLOCKCNT;
				++state;
				break;

			case 5:
				if (CLOCKCNT - sstarttime > 10 * clock_sysclocks)
				{
					state = 0;
				}
				break;

		}
	}
}
