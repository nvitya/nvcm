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
 *  file:     keymatrix.cpp
 *  brief:    Simple keyboard matrix handler class
 *  version:  1.00
 *  date:     2018-09-30
 *  authors:  nvitya
*/

#ifndef KEYMATRIX_H_
#define KEYMATRIX_H_

#include "hwpins.h"

#define KEYMATRIX_MAX_COLS  8
#define KEYMATRIX_MAX_ROWS  8

class TKeyMatrix
{
public:

	TGpioPin    rowpin[KEYMATRIX_MAX_ROWS]; // must be assigned before calling init
	TGpioPin    colpin[KEYMATRIX_MAX_COLS]; // must be assigned before calling init

	uint8_t     rows = 1;
	uint8_t     columns = 1;

	uint32_t    scan_speed_us = 1000;
	uint8_t     filter_count = 3; // so much consecutive levels are taken into account only

	bool 			  initialized = false;

	// bitmap of the pressed keys
	union
	{
	  uint64_t    keys64 = 0;
		uint8_t     keys8[8];
		uint32_t    keys32[2];
	};

	bool Init(uint8_t arows, uint8_t acolumns);

	void Run();  // state maschine for the scanning

public:
	int         state = 0;
	int         scanrow = 0;
	uint8_t     sample_cnt = 0;
	uint32_t    col_keys_prev = 0;
	uint32_t    col_keys = 0;

	uint32_t    stime = 0;
	uint32_t    scan_clocks = 0;

	uint8_t     fcnt[KEYMATRIX_MAX_COLS];
};


#endif /* SRC_KEYMATRIX_H_ */
