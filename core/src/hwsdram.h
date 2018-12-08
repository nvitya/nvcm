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
 *  file:     hwsdram.h
 *  brief:    SDRAM controller + SDRAM IC setup vendor-independent definitions
 *  version:  1.00
 *  date:     2018-11-24
 *  authors:  nvitya
*/

#ifndef _HWSDRAM_H_PRE_
#define _HWSDRAM_H_PRE_

#include "platform.h"
#include "hwpins.h"
#include "errors.h"

class THwSdram_pre
{
public:	// controller settings
	bool 					 initialized = false;
	uint32_t       address = 0x80000000;

public: // calculated values
	uint32_t       byte_size = 0; // will be calculated
	uint32_t       bank1_offset = 0;
	uint32_t       refresh_time_ns = 15625;

public: // settings
	uint8_t        row_bits = 13;
	uint8_t        column_bits = 9;
	uint8_t        bank_count = 4;
	uint8_t        cas_latency = 3;
	uint8_t        data_bus_width = 16;

	uint32_t       refresh_period_ns = 64000;

	uint8_t        row_precharge_delay = 3;       // TRP
	uint8_t        row_to_column_delay = 3;       // TRCD
	uint8_t        recovery_delay = 2;            // TWR
	uint8_t        row_cycle_delay = 9;           // TRC
	uint8_t        active_to_precharge_delay = 6; // TRAS
	uint8_t        exit_self_refresh_delay = 10;  // TXSR

public: // SDRAM IC settings
	uint8_t        burst_length = 1;     // valid: 1, 2, 3, 8,  128 = full page
	uint8_t        burst_type = 0;       // 0 = sequential, 1 = interleaved
	uint8_t        write_burst_mode = 0; // 0 = burst_length, 1 = single location

public:

	void           PrepareParams();
};

#endif // ndef _HWSDRAM_H_PRE_

#ifndef HWSDRAM_PRE_ONLY

//-----------------------------------------------------------------------------

#ifndef HWSDRAM_H_
#define HWSDRAM_H_

#include "mcu_impl.h"

#ifndef HWSDRAM_IMPL

| (5 <<  4)  // TXSR(4):
| (3 <<  8)  // TRAS(4):
| (5 << 12)  // TRC(4):
| (1 << 16)  // TWR(4):
| (1 << 20)  // TRP(4):
| (1 << 24)  // TRCD(4):


class THwSdram_noimpl : public THwSdram_pre
{
public: // mandatory
	bool InitHw() { return false; }

	void Cmd_ClockEnable() { }
	void Cmd_AllBankPrecharge() { }
	void Cmd_AutoRefresh(int acount) { }
	void Cmd_LoadModeRegister(uint16_t aregvalue) { }
	void Cmd_LoadExtModeRegister(uint16_t aregvalue) { }
	void SetNormalMode() { }
	void SetRefreshTime(uint32_t atime_ns) { }
};

#define HWSDRAM_IMPL   THwSdram_noimpl

#endif // ndef HWSDRAM_IMPL

//-----------------------------------------------------------------------------

class THwSdram : public HWSDRAM_IMPL
{
public:
	bool Init();
};

extern THwSdram hwsdram;

#endif // HWSDRAM_H_

#else
  #undef HWSDRAM_PRE_ONLY
#endif
