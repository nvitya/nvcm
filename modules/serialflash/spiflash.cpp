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
 *  file:     spiflash.cpp
 *  brief:    SPI Flash Memory Implementation
 *  version:  1.00
 *  date:     2018-02-10
 *  authors:  nvitya
*/

#include "spiflash.h"

bool TSpiFlash::InitInherited()
{
	if (!pin_cs.Assigned())
	{
		return false;
	}

	if (!txdma.initialized || !rxdma.initialized)
	{
		return false;
	}

	spi.DmaAssign(true, &txdma);
	spi.DmaAssign(false, &rxdma);

	return true;
}

bool TSpiFlash::ReadIdCode()
{
	txbuf[0] = 0x9F;
	txbuf[1] = 0x00;
	txbuf[2] = 0x00;
	txbuf[3] = 0x00;

	rxbuf[0] = 0; rxbuf[1] = 0; rxbuf[2] = 0; rxbuf[3] = 0;
	//rxbuf[0] = 0x55; rxbuf[1] = 0x56; rxbuf[2] = 0x57; rxbuf[3] = 0x58;

	ExecCmd(4);

	unsigned * up = (unsigned *)&(rxbuf[0]);
	idcode = (*up >> 8);

	if ((idcode == 0) || (idcode == 0x00FFFFFF))
	{
		// SPI communication error
		return false;
	}

	return true;
}

void TSpiFlash::StartCmd(unsigned acmdlen)
{
	pin_cs.Set1();

	curcmdlen = acmdlen;

	txfer.srcaddr = &txbuf[0];
	txfer.bytewidth = 1;
	txfer.count = curcmdlen;
	txfer.addrinc = true;

	rxfer.dstaddr = &rxbuf[0];
	rxfer.bytewidth = 1;
	rxfer.addrinc = true;
	rxfer.count = txfer.count;

	pin_cs.Set0();  // activate CS

	spi.DmaStartRecv(&rxfer);
	spi.DmaStartSend(&txfer);
}

void TSpiFlash::ExecCmd(unsigned acmdlen)
{
	StartCmd(acmdlen);

	while (!spi.DmaRecvCompleted())
	{
		// wait
	}

	pin_cs.Set1();
}

void TSpiFlash::Run()
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
				txbuf[0] = 0x03; // read command
				txbuf[1] = ((address >> 16) & 0xFF);
				txbuf[2] = ((address >>  8) & 0xFF);
				txbuf[3] = ((address >>  0) & 0xFF);
				StartCmd(4);  // activates the CS too
				phase = 1;
				break;
			case 1: // wait for address phase completition
				if (spi.DmaRecvCompleted())
				{
					phase = 2; Run();  // phase jump
					return;
				}
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

				txbuf[0] = 0;
				txfer.srcaddr = &txbuf[0];
				txfer.bytewidth = 1;
				txfer.count = chunksize;
				txfer.addrinc = false;    // sending the same zero character

				rxfer.dstaddr = dataptr;
				rxfer.bytewidth = 1;
				rxfer.addrinc = true;
				rxfer.count = chunksize;

				spi.DmaStartRecv(&rxfer);
				spi.DmaStartSend(&txfer);

				phase = 3;
				break;

			case 3:  // wait for completion
				if (spi.DmaRecvCompleted())
				{
					// read next chunk
					dataptr   += chunksize;
					remaining -= chunksize;
					phase = 2; Run();  // phase jump
					return;
				}

				// TODO: timeout handling

				break;

			case 10: // finished
				pin_cs.Set1();
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

			  // set write enable
				txbuf[0] = 0x06;
				StartCmd(1);  // activates the CS too
				++phase;
				break;

			case 2: // wait for command phase completition
				if (spi.DmaRecvCompleted())
				{
					pin_cs.Set1(); // pull back the CS
					++phase; Run();  // phase jump
					return;
				}
				break;

			case 3: // write data
				txbuf[0] = 0x02; // page program command
				txbuf[1] = ((address >> 16) & 0xFF);
				txbuf[2] = ((address >>  8) & 0xFF);
				txbuf[3] = ((address >>  0) & 0xFF);
				StartCmd(4);  // activates the CS too
				++phase;
				break;

			case 4: // wait for command phase completition
				if (spi.DmaRecvCompleted())
				{
					++phase; Run();  // phase jump
					return;
				}
				break;

			case 5: // data phase
				chunksize = 256; // page size
				if (chunksize > remaining)  chunksize = remaining;

				txfer.srcaddr = dataptr;
				txfer.bytewidth = 1;
				txfer.count = chunksize;
				txfer.addrinc = true;

				rxfer.dstaddr = &rxbuf[0];
				rxfer.bytewidth = 1;
				rxfer.addrinc = false;  // ignore the received data
				rxfer.count = chunksize;

				spi.DmaStartRecv(&rxfer);
				spi.DmaStartSend(&txfer);

				++phase;
				break;

			case 6:  // wait for completion
				if (spi.DmaRecvCompleted())
				{
					pin_cs.Set1(); // pull back the CS
					++phase; Run();
					return;
				}
				// TODO: timeout handling
				break;

			case 7: // read status register, repeat until BUSY flag is set
				StartReadStatus();
				++phase;
				break;

			case 8:
				if (spi.DmaRecvCompleted())
				{
					pin_cs.Set1(); // pull back the CS
					// check the result
					if (rxbuf[1] & 1) // busy
					{
						// repeat status register read
						phase = 7; Run();
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

			  // set write enable
				txbuf[0] = 0x06;
				StartCmd(1);  // activates the CS too
				++phase;
				break;

			case 2: // wait for command phase completition
				if (spi.DmaRecvCompleted())
				{
					pin_cs.Set1(); // pull back the CS
					++phase; Run();  // phase jump
					return;
				}
				break;

			case 3: // erase sector / block

				if (has4kerase && ((address & 0xFFFF) || (datalen < 0x10000)))
				{
					// 4k sector erase
					chunksize = 0x01000;
					txbuf[0] = 0x20;
				}
				else
				{
					// 64k block erase
					chunksize = 0x10000;
					txbuf[0] = 0xD8;
				}
				txbuf[1] = ((address >> 16) & 0xFF);
				txbuf[2] = ((address >>  8) & 0xFF);
				txbuf[3] = ((address >>  0) & 0xFF);
				StartCmd(4);  // activates the CS too
				++phase;
				break;

			case 4: // wait for command phase completition
				if (spi.DmaRecvCompleted())
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
				if (spi.DmaRecvCompleted())
				{
					pin_cs.Set1(); // pull back the CS
					// check the result
					if (rxbuf[1] & 1) // busy
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
			  // set write enable
				txbuf[0] = 0x06;
				StartCmd(1);  // activates the CS too
				++phase;
				break;

			case 2: // wait for command phase completition
				if (spi.DmaRecvCompleted())
				{
					pin_cs.Set1(); // pull back the CS
					++phase; Run();  // phase jump
					return;
				}
				break;

			case 3: // issue bulk erase
				txbuf[0] = 0xC7;
				StartCmd(1);  // activates the CS too
				++phase;
				break;

			case 4: // wait for command phase completition
				if (spi.DmaRecvCompleted())
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
				if (spi.DmaRecvCompleted())
				{
					pin_cs.Set1(); // pull back the CS
					// check the result
					if (rxbuf[1] & 1) // busy
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


bool TSpiFlash::StartReadStatus()
{
	txbuf[0] = 0x05;
	txbuf[1] = 0x00;
	rxbuf[1] = 0x7F;
	StartCmd(2);
	return true;
}


void TSpiFlash::ResetChip()
{
	txbuf[0] = 0x66;
	ExecCmd(1);

	txbuf[0] = 0x99;
	ExecCmd(1);
}
