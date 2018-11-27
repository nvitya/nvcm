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
 *  file:     hwsdram.cpp
 *  brief:    SDRAM controller + SDRAM setup vendor-independent definitions
 *  version:  1.00
 *  date:     2018-11-24
 *  authors:  nvitya
*/

#include "platform.h"
#include "hwsdram.h"
#include "clockcnt.h"

THwSdram hwsdram;

void THwSdram_pre::PrepareParams()
{
	byte_size = (1 << (row_bits + column_bits)) * bank_count * (data_bus_width >> 3);
	bank1_offset = (1 << (row_bits + column_bits + (data_bus_width >> 3)));

	refresh_time_ns = refresh_period_ns / (1 << row_bits);
}

bool THwSdram::Init()
{
	initialized = false;

	PrepareParams();

	if (!InitHw())
	{
		return false;
	}

	// A minimum pause of 200 µs is provided to precede any signal toggle. (6 core cycles per iteration)
	delay_us(200);

	// program the SDRAM device

	// A NOP/ClkEnable command is issued to the SDR-SDRAM. Now the clock which drives SDR-SDRAM device is enabled.
	Cmd_ClockEnable();

	// An all banks precharge command is issued to the SDR-SDRAM.
	Cmd_AllBankPrecharge();

	// Add some delays after precharge
	delay_us(200);

	// Eight auto-refresh (CBR) cycles are provided
	Cmd_AutoRefresh(8);

	// A Mode Register Set (MRS) cycle is issued to program the parameters
	// of the SDRAM device, in particular CAS latency and burst length

	uint16_t blcode;
	if       (burst_length <= 1)    blcode = 0;
	else if  (burst_length <= 2)    blcode = 1;
	else if  (burst_length <= 4)    blcode = 2;
	else if  (burst_length <= 8)    blcode = 3;
	else if  (burst_length >= 128)  blcode = 7;
	else                            blcode = 0;

	uint16_t modereg = 0
		| (blcode            <<  0)  // BURST_LENGTH(3)
		| ((burst_type & 1)  <<  3)  // BURST_TYPE: 0 = sequential, 1 = interleaved
		| ((cas_latency & 3) <<  4)  // CAS_LATENCY(3)
		| (0                 <<  7)  // OPERATION MODE
		| (write_burst_mode  <<  9)  // Write burst mode 0 = programmed burst length, 1 = single location
	;
	Cmd_LoadModeRegister(modereg);

	delay_us(300);

#if 0
	// For low-power SDR-SDRAM initialization, an Extended Mode Register Set
  // (EMRS) cycle is issued to program the SDR-SDRAM parameters (TCSR, PASR, DS).
	// The write address must be chosen so that BA[1] is set to 1 and BA[0] is set to 0.
	Cmd_LoadExtModeRegister(0);
#endif

	SetNormalMode();

	SetRefreshTime(refresh_time_ns);

	initialized = true;
	return true;
}

