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

#include "string.h"
#include "keymatrix.h"
#include "clockcnt.h"

bool TKeyMatrix::Init(uint8_t arows, uint8_t acolumns)
{
	int i;
	initialized = false;
	rows = arows;
	columns = acolumns;
	if (rows > KEYMATRIX_MAX_ROWS)  rows = KEYMATRIX_MAX_ROWS;
	if (columns > KEYMATRIX_MAX_COLS)  columns = KEYMATRIX_MAX_COLS;
	if (rows < 1)  rows = 1;
	if (columns < 1)  columns = 1;

	for (i = 0; i < rows; ++i)
	{
		if (!rowpin[i].Assigned())  return false;
	}
	for (i = 0; i < columns; ++i)
	{
		if (!colpin[i].Assigned())  return false;
	}

	for (i = 0; i < rows; ++i)
	{
		rowpin[i].Setup(PINCFG_INPUT | PINCFG_PULLUP);
	}
	for (i = 0; i < columns; ++i)
	{
		colpin[i].Setup(PINCFG_INPUT | PINCFG_PULLUP);
	}

	memset(&fcnt[0], 0, sizeof(fcnt));

	scan_clocks = scan_speed_us * (SystemCoreClock / 1000000);

	initialized = true;

	return true;
}

void TKeyMatrix::Run()
{
	int i;

	if (!initialized)
	{
		return;
	}

	switch (state)
	{
	case 0: // start row scan
		for (i = 0; i < rows; ++i)
		{
			if (i == scanrow)
			{
				rowpin[i].Set0();
				rowpin[i].SwitchDirection(1);
			}
			else
			{
				rowpin[i].Set1();
				rowpin[i].SwitchDirection(0);
			}
		}
		for (i = 0; i < columns; ++i)  fcnt[i] = 0;
		stime = CLOCKCNT;
		sample_cnt = 0;
		state = 1;
		break;

	case 1: // sample columns
		if (CLOCKCNT - stime >= scan_clocks)
		{
			++sample_cnt;
			col_keys = 0;
			for (i = 0; i < columns; ++i)
			{
				uint32_t col_bit = (1 << i);
				if (colpin[i].Value() == 0)  // is it pressed ?
				{
					col_keys |= col_bit;
				}

				++fcnt[i];

				if (sample_cnt > 1)
				{
					if ((col_keys_prev ^ col_keys) & col_bit)
					{
						fcnt[i] = 0;
					}
				}

				if (sample_cnt >= filter_count) // last sample
				{
					if (fcnt[i] == filter_count)
					{
						// this is stable, update it
						uint64_t key_bit = (uint64_t(1) << (i + scanrow * columns));
						uint64_t tmp = keys64;
						if (col_keys & col_bit)
						{
							tmp |= key_bit;
						}
						else
						{
							tmp &= ~key_bit;
						}
						keys64 = tmp;
					}
				}
			}

			col_keys_prev = col_keys;
			stime = CLOCKCNT;

			if (sample_cnt >= filter_count) // end of row scan
			{
				++scanrow;
				if (scanrow >= rows)  scanrow = 0;
				state = 0;
			}
		}
		break;
	}
}

bool TKeyMatrix::AnyKeyPressed(unsigned test_us)
{
	state = 0; // reset the state if it is used parallel with the run

	int i;
	for (i = 0; i < rows; ++i)
	{
		rowpin[i].Set0();
		rowpin[i].SwitchDirection(1);
	}

	delay_us(test_us);

	for (i = 0; i < columns; ++i)
	{
		if (colpin[i].Value() == 0)
		{
			return true;
		}
	}

	return false;
}
