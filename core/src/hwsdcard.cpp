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

#include "errors.h"
#include "hwsdcard.h"
#include "traces.h"
#include "clockcnt.h"

bool THwSdcard::Init()
{
	initialized = false;

	if (!dma.initialized)
	{
		return false;
	}

	if (!HwInit())
	{
		return false;
	}

	completed = true;
	card_initialized = false;
	initstate = 0; // set the state machine to start position

	after_error_delay_clocks = (SystemCoreClock / 20); // = 50 ms

	initialized = true;

	return true;
}

void THwSdcard::Run()
{
	if (!initialized)
	{
		return;
	}

	if (!card_initialized)
	{
		RunInitialization();
		return;
	}

	RunTransfer(); // run transfer state machine
}

void THwSdcard::RunInitialization()
{
	int i;
	uint32_t carg;

	//TRACE("SD state = %i\r\n", state);

	if (cmdrunning && !CmdFinished())
	{
		return;
	}

	cmdrunning = false;

	switch (initstate)
	{
	case 0: // send init clocks

		// required low settings for the initialization
		SetSpeed(400000); // initial speed = 400 kHz
		SetBusWidth(1);

		card_present = false;
		high_capacity = false;
		card_v2 = false;

		SendSpecialCmd(SD_SPECIAL_CMD_INIT); // never fails
		++initstate;
		break;

	case 1: // set SDCARD to init
		SendCmd(0, 0, SDCMD_RES_NO); // never fails
		++initstate;
		break;

	case 2:
		SendCmd(8, 0x1AA, SDCMD_RES_48BIT | SDCMD_OPENDRAIN);  // test for V2
		++initstate;
		break;

	case 3:
		if (!cmderror && (0xFFFFFFFF != GetCmdResult32()))
		{
			card_v2 = true;
		}
		initstate = 5;
		break;

	// Wait until the card is ready

	case 5:
		// prepare application specific command
		SendCmd(55, 0, SDCMD_RES_48BIT | SDCMD_OPENDRAIN);
		++initstate;
		break;

	case 6:
		if (cmderror)
		{
			// no card available
			card_present = false;
			initstate = 100;
		}
		else
		{
			// get operating status
			carg = 0x001f8000;
			if (card_v2)  carg |= (1u << 30);
			SendCmd(41, carg, SDCMD_RES_48BIT | SDCMD_OPENDRAIN);
			++initstate;
		}
		break;

	case 7:
		if (cmderror)	 initstate = 100;
		else
		{
			reg_ocr = GetCmdResult32();
			if (reg_ocr & 0x80000000)
			{
				TRACE("SDCARD OCR = %08X\r\n", reg_ocr);
				high_capacity = (reg_ocr & (1u << 30) ? true : false);
				initstate = 10;
			}
			else
			{
				// repeat OCR register read
				initstate = 5;
			}
		}
		break;

  // read Card Identification Data (CID)
	case 10:
		SendCmd(2, 0, SDCMD_RES_136BIT | SDCMD_OPENDRAIN);
		++initstate;
		break;

	case 11:
		if (cmderror)  initstate = 100;
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
			initstate = 20;
		}
		break;

	// Ask for a Relative Card Address (RCA)
	case 20:
		SendCmd(3, 0, SDCMD_RES_48BIT | SDCMD_OPENDRAIN);
		++initstate;
		break;

	case 21:
		if (cmderror)  initstate = 100;
		else
		{
			rca = (GetCmdResult32() & 0xFFFF0000);  // keep it high-shifted to spare later shiftings
			TRACE("SDCARD RCA = %08X\r\n", rca);
			++initstate;
		}
		break;

	// get the Card Specific Data (CSD)
	case 22:
		SendCmd(9, rca, SDCMD_RES_136BIT | SDCMD_OPENDRAIN);
		++initstate;
		break;

	case 23:
		if (cmderror)  initstate = 100;
		else
		{
			GetCmdResult128(&reg_csd[0]);

			TRACE("SDCARD CSD = ");
			for (i = 0; i < 16; ++i)
			{
				TRACE(" %02X", reg_csd[i]);
			}
			TRACE("\r\n");
			++initstate;

			ProcessCsd();
		}
		break;

	// Select the card and put into transfer mode
	case 24:
		SendCmd(7, rca, SDCMD_RES_R1B);
		++initstate;
		break;

	case 25:
		if (cmderror)
		{
			TRACE("Error selecting card!\r\n");
			initstate = 100;
		}
		else
		{
			++initstate;
		}
		break;

  // get the SCR register
	case 26:
		// prepare application specific command
		SendCmd(55, rca, SDCMD_RES_48BIT);
		++initstate;
		break;

	case 27:
		StartDataReadCmd(51, 0, SDCMD_RES_48BIT, &reg_scr[0], sizeof(reg_scr));
		++initstate;
		break;
	case 28:
		if (cmderror)
		{
			TRACE("SCR register read error!\r\n");
			initstate = 100;
		}
		else if (!dma.Active())
		{
			TRACE("SDCARD SCR = ");
			for (i = 0; i < 8; ++i)
			{
				TRACE(" %02X", reg_scr[i]);
			}
			TRACE("\r\n");

			++initstate;
		}
		break;

  // set bus width to 4 bit
	case 29:
		// prepare application specific command
		SendCmd(55, rca, SDCMD_RES_48BIT);
		++initstate;
		break;

	case 30:
		SendCmd(6, 2, SDCMD_RES_48BIT);  // Switch command, 2 = 4 bit bus
		++initstate;
		break;

	case 31:
		if (cmderror)		initstate = 100;
		else
		{
			SetSpeed(clockspeed);
			SetBusWidth(4); // always 4 bit bus
			++initstate;
		}
		break;

	case 32:
		SendCmd(16, 512, SDCMD_RES_48BIT); // set block size
		++initstate;
		break;

	case 33:
		if (cmderror)		initstate = 100;
		else
		{
			TRACE("SDCARD initialized, ready to accept transfer commands.\r\n");
			initstate = 50;
		}
		break;

	case 50:
		// ready to accept transfer commands, go to normal state
		card_initialized = true;
		completed = true;
		initstate = 0;
		break;

	case 100: // delay after error
		if (CLOCKCNT - lastcmdtime > after_error_delay_clocks)
		{
			initstate = 0;
		}
		break;
	}
}

uint32_t THwSdcard::GetRegBits(void * adata, uint32_t startpos, uint8_t bitlen)
{

	uint32_t * d32p = (uint32_t *)adata;
	uint32_t widx = (startpos >> 5);
	d32p += widx;
	uint8_t bitidx = (startpos & 31);

	uint32_t result = (*d32p >> bitidx);

	if (bitidx + bitlen > 32)
	{
		++d32p;
		result |= (*d32p << (32 - bitidx));
	}

	if (bitlen < 32)
	{
		result &= ((1 << bitlen) - 1);
	}

	return result;
}

//! SD/MMC transfer rate unit codes (10K) list
static const uint32_t sd_trans_units[8] = { 10, 100, 1000, 10000, 0, 0, 0, 0 };
//! SD transfer multiplier factor codes (1/10) list
static const uint32_t sd_trans_values[16] = {	0, 10, 12, 13, 15, 20, 25, 30, 35, 40, 45, 50, 55, 60, 70, 80 };

void THwSdcard::ProcessCsd()
{
/* CSD bits:

     126(2): Version
     122(4): Spec. version
      96(8): Tran speed
V1:
      47(3): V1 C size mult.
     62(12): V1 C size
      80(4): V1 Read BL Len
v2:
     48(22): V2 C. Size
*/
	csd_ver = GetRegBits(&reg_csd[0], 126, 2);

	uint32_t transunit = sd_trans_units[GetRegBits(&reg_csd[0], 96, 3)];
	uint32_t transval  = sd_trans_values[GetRegBits(&reg_csd[0], 99, 4)];
	csd_max_speed = 1000 * transunit * transval;

	// decode card size

	/*
	 * Get card capacity.
	 * ----------------------------------------------------
	 * For normal SD/MMC card:
	 * memory capacity = BLOCKNR * BLOCK_LEN
	 * Where
	 * BLOCKNR = (C_SIZE+1) * MULT
	 * MULT = 2 ^ (C_SIZE_MULT+2)       (C_SIZE_MULT < 8)
	 * BLOCK_LEN = 2 ^ READ_BL_LEN      (READ_BL_LEN < 12)
	 * ----------------------------------------------------
	 * For high capacity SD card:
	 * memory capacity = (C_SIZE+1) * 512K byte
	 */

	if (csd_ver >= 1)
	{
		card_megabytes = ((GetRegBits(&reg_csd[0], 48, 22) + 1) >> 1);
	}
	else
	{
		uint32_t c_size = GetRegBits(&reg_csd[0], 62, 12);
		uint32_t c_size_mult = GetRegBits(&reg_csd[0], 47, 3);
		uint32_t block_len = (1 << GetRegBits(&reg_csd[0], 80, 4));

		uint32_t blocknr = (c_size + 1) * (1 << (c_size_mult + 2));

		card_megabytes = ((blocknr * block_len) >> 20);
	}

	TRACE("SD card max speed = %u MHz, size = %u MBytes\r\n", csd_max_speed / 1000000, card_megabytes);
}

bool THwSdcard::StartReadBlocks(uint32_t astartblock, void * adataptr, uint32_t ablockcount)
{
	if (!initialized)
	{
		errorcode = ERROR_NOTINIT;
		completed = true;
		return false;
	}

	if (!completed)
	{
		errorcode = ERROR_BUSY;  // this might be overwriten later
		return false;
	}

	dataptr = (uint8_t *)adataptr;
	blockcount = ablockcount;
	remainingblocks = ablockcount;
	startblock = astartblock;
	curblock = astartblock;

	errorcode = 0;
	completed = false;

	trstate = 1; // read blocks

	Run();

	return (errorcode == 0);
}

void THwSdcard::WaitForComplete()
{
	while (!completed)
	{
		Run();
	}
}
