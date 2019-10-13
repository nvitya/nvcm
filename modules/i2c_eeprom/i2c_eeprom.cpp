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
 *  file:     i2c_eeprom.cpp
 *  brief:    I2C EEPROM (24LC16B) Implementation
 *  version:  1.00
 *  date:     2019-03-24
 *  authors:  nvitya
*/

#include "i2c_eeprom.h"

bool TI2cEeprom::Init(THwI2c * ai2c, uint8_t aaddr, uint32_t abytesize)
{
	initialized = false;
	pi2c = ai2c;
	errorcode = ERROR_NOTINIT;

	if (!pi2c)
	{
		return false;
	}

	if (!pi2c->initialized)
	{
		return false;
	}

	devaddr = aaddr;
	bytesize = abytesize;

	initialized = true;

	// try to read the first 4 bytes

	uint32_t data = 0;
	errorcode = pi2c->StartReadData(devaddr + ((address >> 8) & 7), (address & 0xFF) | I2CEX_1, &data, 4);
	if (errorcode != ERROR_OK)
	{
		initialized = false;
		return false;
	}

	if (pi2c->WaitFinish() != ERROR_OK)
	{
		errorcode = pi2c->error;
		initialized = false;
		return false;
	}

	return true;
}

int TI2cEeprom::StartReadMem(unsigned aaddr, void * adstptr, unsigned alen)
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

	if (aaddr + alen > bytesize)
	{
		errorcode = ERROR_READ;
		return false;
	}

	dataptr = (uint8_t *)adstptr;
	datalen = alen;
	address = aaddr;

	state = I2C_STATE_READMEM; // read memory
	phase = 0;

	errorcode = 0;
	completed = false;

	Run();

	return (errorcode == 0);
}

int TI2cEeprom::StartWriteMem(unsigned aaddr, void * asrcptr, unsigned alen)
{
	if (!initialized)
	{
		errorcode = ERROR_NOTINIT;
		completed = true;
		return errorcode;
	}

	if (!completed)
	{
		errorcode = ERROR_BUSY;  // this might be overwriten later
		return errorcode;
	}

	if (aaddr + alen > bytesize)
	{
		errorcode = ERROR_WRITE;
		return errorcode;
	}

	dataptr = (uint8_t *)asrcptr;
	datalen = alen;
	address = aaddr;

	state = I2C_STATE_WRITEMEM; // write page
	phase = 0;

	errorcode = 0;
	completed = false;

	Run();

	return errorcode;
}

void TI2cEeprom::Run()
{
	if (0 == state)
	{
		// idle
		return;
	}

	if (I2C_STATE_READMEM == state)  // read memory
	{
		// the read can be carried out in a single transaction

		switch (phase)
		{
			case 0: // start
				pi2c->StartReadData(devaddr + ((address >> 8) & 7), (address & 0xFF) | I2CEX_1, dataptr, datalen);
				phase = 1;
				break;

			case 1:
				if (pi2c->Finished())
				{
					errorcode = pi2c->error;
					completed = true;
					state = 0; // back to idle
				}
				break;
		}
	}
	else if (I2C_STATE_WRITEMEM == state)  // write memory
	{
		// by the writes the 16 byte internal page structure must be taken care of

		switch (phase)
		{
			case 0: // initialization
				remaining = datalen;
				++phase;
				break;

			case 1: // write next chunk
				if (remaining == 0)
				{
					completed = true;
					state = 0;
					return;
				}

				chunksize = 16 - (address & 0xF);
				if (chunksize > remaining)  chunksize = remaining;

				pi2c->StartWriteData(devaddr + ((address >> 8) & 7), (address & 0xFF) | I2CEX_1, dataptr, chunksize);

				++phase;
				break;

			case 2: // wait i2c send complete
				if (pi2c->Finished())
				{
					errorcode = pi2c->error;
					if (errorcode)
					{
						completed = true;
						state = 0;
						return;
					}

					// the write operation started internally, until it finishes the device does not send an ACK
					++phase; Run();  // phase jump
					return;
				}
				break;

			case 3: // try reading until it answers ...
				pi2c->StartReadData(devaddr, 0, &rxbuf[0], 1);
				++phase;
				break;

			case 4: // wait completition
				if (pi2c->Finished())
				{
					if (pi2c->error == 0) // read successful, that means that the write is completed
					{
						dataptr += chunksize;
						address += chunksize;
						remaining -= chunksize;

						phase = 1;  Run();  return;
					}
					else
					{
						phase = 3; // continue polling
					}
				}
				break;
		}
	}
}

int TI2cEeprom::WaitComplete()
{
	while (!completed)
	{
		Run();
	}

	return errorcode;
}
