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
 *  file:     qspiflash.cpp
 *  brief:    QSPI Flash Memory implementation
 *  version:  1.00
 *  date:     2018-02-10
 *  authors:  nvitya
*/

#include "qspiflash.h"
#include "clockcnt.h"

bool TQspiFlash::InitInherited()
{
	qspi.Init();

	if (qspi.multi_line_count == 4)
	{
		// The Quad mode must be enabled (switch HOLD and WP pins to IO2 and IO3)
		// This is done usually with status register write but the bit number is manufacturer specific

		if (!ReadIdCode())
		{
			return false;
		}

		unsigned char mnfid = (idcode & 0xFF);

		if ((0x9D == mnfid) || (0xC2 == mnfid))
		{
			// 0x9D = ISSI
			// 0xC2 = MXIC / Macronix
			// Quad enable is status bit 6

			qspi.StartWriteData(0x06, 0, nullptr, 0);  // Enable write
			qspi.WaitFinish();

			unsigned data = 0x40;
			qspi.StartWriteData(0x01, 0, &data, 1);  // Write status register bit 6
			qspi.WaitFinish();

			do
			{
				StartReadStatus();
				qspi.WaitFinish();
			}
			while (statusreg & 1);

		}
		else if (0xEF == mnfid)
		{
			// Winbond
			// Quad enable is status register bit 9
			qspi.StartWriteData(0x06, 0, nullptr, 0);  // Enable write
			qspi.WaitFinish();

			unsigned data = 0x200;
			qspi.StartWriteData(0x01, 0, &data, 2);  // Write status register bit 9
			qspi.WaitFinish();

			do
			{
				StartReadStatus();
				qspi.WaitFinish();
			}
			while (statusreg & 1);
		}
		else
		{
			// unknown hardware
			qspi.multi_line_count = 2; // falling back to dual mode
			return false;
		}
	}

	return true;
}

bool TQspiFlash::ReadIdCode()
{
	rxbuf[0] = 0; rxbuf[1] = 0; rxbuf[2] = 0; rxbuf[3] = 0;

	qspi.StartReadData(0x9F, 0, &rxbuf[0], 4);
	qspi.WaitFinish();

	unsigned * up = (unsigned *)&(rxbuf[0]);
	idcode = (*up & 0x00FFFFFF);

	if ((idcode == 0) || (idcode == 0x00FFFFFF))
	{
		// SPI communication error
		return false;
	}

	return true;
}

void TQspiFlash::Run()
{
	if (0 == state)
	{
		// idle
		return;
	}

	if (SERIALFLASH_STATE_READMEM == state)  // read memory
	{
		switch (phase)
		{
			case 0: // start
				remaining = datalen;
				phase = 2;
				break;

			case 2: // start read next data chunk
				if (remaining == 0)
				{
					// finished.
					phase = 10; Run();  // phase jump
					return;
				}

				// start data phase

				chunksize = maxchunksize;
				if (chunksize > remaining)  chunksize = remaining;

				if (qspi.multi_line_count == 4)
				{
					qspi.StartReadData(0xEB | QSPICM_SMM | QSPICM_ADDR | QSPICM_DUMMY, address, dataptr, chunksize);
				}
				else if (qspi.multi_line_count == 2)
				{
					qspi.StartReadData(0xBB | QSPICM_SMM | QSPICM_ADDR | QSPICM_DUMMY, address, dataptr, chunksize);
				}
				else
				{
					qspi.StartReadData(0x0B | QSPICM_SSS | QSPICM_ADDR | QSPICM_DUMMY, address, dataptr, chunksize);
				}

				phase = 3;
				break;

			case 3:  // wait for completion
				if (qspi.Finished())
				{
					// read next chunk
					address   += chunksize;
					dataptr   += chunksize;
					remaining -= chunksize;
					phase = 2; Run();  // phase jump
					return;
				}

				// TODO: timeout handling

				break;

			case 10: // finished
				completed = true;
				state = 0; // go to idle
				break;
		}
	}
	else if (SERIALFLASH_STATE_WRITEMEM == state)  // write memory
	{
		switch (phase)
		{
			case 0: // initialization
				remaining = datalen;

				++phase;
				break;

			// write next chunk
			case 1: // set write enable
				if (remaining == 0)
				{
					// finished.
					phase = 20; Run();  // phase jump
					return;
				}

				StartWriteEnable();
				++phase;
				break;

			case 2: // wait Write Enable completition
				if (qspi.Finished())
				{
					++phase; Run();  // phase jump
					return;
				}
				break;

			case 3: // write data
				chunksize = 256 - (address & 0xFF); // for the writes this is the maximal size

				if (chunksize > remaining)  chunksize = remaining;

				if ((qspi.multi_line_count == 4) && ((idcode & 0xFF) != 0xC2))
				{
					qspi.StartWriteData(0x32 | QSPICM_SMM | QSPICM_ADDR, address, dataptr, chunksize);
				}
				else
				{
					qspi.StartWriteData(0x02 | QSPICM_SSS | QSPICM_ADDR, address, dataptr, chunksize);
				}

				++phase;
				break;

			case 4: // wait completition
				if (qspi.Finished())
				{
					++phase; Run();  // phase jump
					return;
				}
				break;

			case 5: // read status register, repeat until BUSY flag is set
				StartReadStatus();
				++phase;
				break;

			case 6:
				if (qspi.Finished())
				{
					// check the result
					if (statusreg & 1) // busy
					{
						// repeat status register read
						phase = 5; Run();
						return;
					}
					// Write finished.
					address += chunksize;
					dataptr += chunksize;
					remaining -= chunksize;
					phase = 1; Run();  // phase jump
					return;
				}
				// TODO: timeout
				break;

			case 20: // finished
				completed = true;
				state = 0; // go to idle
				break;
		}
	}
	else if (SERIALFLASH_STATE_ERASE == state)  // erase memory
	{
		switch (phase)
		{
			case 0: // initialization

				// correct start address if necessary
				if (address & erasemask)
				{
					// correct the length too
					datalen += (address & erasemask);
					address &= ~erasemask;
				}

				// round up to 4k the data length
				datalen = ((datalen + erasemask) & ~erasemask);

				remaining = datalen;
				++phase;
				break;

			// erase next sector

			case 1: // set write enable
				if (remaining == 0)
				{
					// finished.
					phase = 20; Run();  // phase jump
					return;
				}

				StartWriteEnable();
				++phase;
				break;

			case 2: // wait for Write Enable completition
				if (qspi.Finished())
				{
					++phase; Run();  // phase jump
					return;
				}
				break;

			case 3: // erase sector / block

				txbuf[0] = ((address >> 16) & 0xFF);
				txbuf[1] = ((address >>  8) & 0xFF);
				txbuf[2] = ((address >>  0) & 0xFF);
				txbuf[3] = 0;

				if (has4kerase && ((address & 0xFFFF) || (remaining < 0x10000)))
				{
					// 4k sector erase
					chunksize = 0x01000;
					qspi.StartWriteData(0x20, 0, &txbuf[0], 3);
				}
				else
				{
					// 64k block erase
					chunksize = 0x10000;
					qspi.StartWriteData(0xD8, 0, &txbuf[0], 3);
				}

				++phase;
				break;

			case 4: // wait for command phase completition
				if (qspi.Finished())
				{
					phase = 7; Run();  // phase jump
					return;
				}
				break;

			case 7: // read status register, repeat until BUSY flag is set
				StartReadStatus();
				++phase;
				break;

			case 8:
				if (qspi.Finished())
				{
					// check the result
					if (statusreg & 1) // busy
					{
						// repeat status register read
						phase = 7; Run();
						return;
					}
					// Write finished.
					address   += chunksize;
					remaining -= chunksize;
					phase = 1; Run();  // phase jump
					return;
				}
				// TODO: timeout
				break;

			case 20: // finished
				completed = true;
				state = 0; // go to idle
				break;
		}
	}
	else if (SERIALFLASH_STATE_ERASEALL == state)  // erase all / chip
	{
		switch (phase)
		{
			case 0: // initialization
				++phase;
				break;

			case 1: // set write enable
				StartWriteEnable();
				++phase;
				break;

			case 2: // wait for completition
				if (qspi.Finished())
				{
					++phase; Run();  // phase jump
					return;
				}
				break;

			case 3: // issue bulk erase
				qspi.StartWriteData(0xC7, 0, nullptr, 0);
				++phase;
				break;

			case 4: // wait for command phase completition
				if (qspi.Finished())
				{
					phase = 7; Run();  // phase jump
					return;
				}
				break;

			case 7: // read status register, repeat until BUSY flag is set
				StartReadStatus();
				++phase;
				break;

			case 8:
				if (qspi.Finished())
				{
					// check the result
					if (statusreg & 1) // busy
					{
						// repeat status register read
						phase = 7; Run();
						return;
					}
					// finished.
					phase = 20; Run();  // phase jump
					return;
				}
				// TODO: timeout
				break;

			case 20: // finished
				completed = true;
				state = 0; // go to idle
				break;
		}
	}
}

void TQspiFlash::StartReadStatus()
{
	statusreg = 0xFFFFFFFF;
	qspi.StartReadData(0x05, 0, &statusreg, 4);  // some QSPI drivers support only 32 bit access
}

void TQspiFlash::StartWriteEnable()
{
	qspi.StartWriteData(0x06, 0, nullptr, 0);
}
