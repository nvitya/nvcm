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
 *  file:     hwsdcard.cpp
 *  brief:    Internal SDCARD Interface vendor-independent implementations
 *  version:  1.00
 *  date:     2018-06-07
 *  authors:  nvitya
*/

#include <stdio.h>
#include <stdarg.h>

#include "hwsdcard.h"
#include "traces.h"
#include "clockcnt.h"

bool THwSdcard::Init()
{
	initialized = false;

	if (!HwInit())
	{
		return false;
	}

	state = 0; // set the state machine to start position

	after_error_delay_clocks = (SystemCoreClock / 20); // = 50 ms

	initialized = true;

	return true;
}

void THwSdcard::Run()
{
	int i;

	if (!initialized)
	{
		return;
	}

	if (cmdrunning && !CmdFinished())
	{
		return;
	}

	cmdrunning = false;

	switch (state)
	{
	case 0: // send init clocks

		// required low settings for the initialization
		SetSpeed(400000); // initial speed = 400 kHz
		SetBusWidth(1);

		card_present = false;

		SendSpecialCmd(SD_SPECIAL_CMD_INIT); // never fails
		++state;
		break;

	case 1: // set SDCARD to init
		SendCmd(0, 0, SDCMD_RES_NO); // never fails
		++state;
		break;

	case 2: // process reset command
		// reset ok
		//TRACE("SDCARD reset ok.\r\n");
		++state;
		break;

	// Wait until the card is ready

	case 3:
		// prepare application specific command
		SendCmd(55, 0, SDCMD_RES_48BIT | SDCMD_OPENDRAIN);
		++state;
		break;

	case 4:
		if (cmderror)
		{
			// no card available
			card_present = false;
			state = 100;
		}
		else
		{
			// get operating status
			SendCmd(41, 0x001f8000 | 0x40000000, SDCMD_RES_48BIT | SDCMD_OPENDRAIN);
			++state;
		}
		break;

	case 5:
		if (cmderror)	 state = 100;
		else
		{
			reg_ocr = GetCmdResult32();
			if (reg_ocr & 0x80000000)
			{
				state = 10;
			}
			else
			{
				// repeat OCR register read
				state = 3;
			}
		}
		break;

  // read Card Identification Data (CID)
	case 10:
		SendCmd(2, 0, SDCMD_RES_136BIT | SDCMD_OPENDRAIN);
		++state;
		break;

	case 11:
		if (cmderror)  state = 100;
		else
		{
			GetCmdResult128(&reg_cid[0]);
			TRACE("SDCARD CID = ");
			for (i = 0; i < 16; ++i)
			{
				TRACE(" %02X", reg_cid[i]);
			}
			TRACE("\r\n");

			card_present = true;
			state = 20;
		}
		break;

	case 20:  // todo: continue...
		break;

	case 100: // delay after error
		if (CLOCKCNT - lastcmdtime > after_error_delay_clocks)
		{
			state = 0;
		}
		break;
	}
}
