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
 *  file:     serialflash.cpp
 *  brief:    Serial flash memory base class
 *  version:  1.00
 *  date:     2018-02-10
 *  authors:  nvitya
*/

#include <serialflash.h>

bool TSerialFlash::Init()
{
	initialized = false;

	// call virtual functions
	if (!InitInterface())
	{
		return false;
	}

	if (!InitInherited())
	{
		return false;
	}

	// read ID value
	if (!ReadIdCode())
	{
		return false;
	}

	// process ID code

	if ((idcode == 0) || (idcode == 0x00FFFFFF))
	{
		// SPI communication error
		return false;
	}

	bytesize = (1 << (idcode >> 16));
	erasemask = (has4kerase ? 0x0FFF : 0xFFFF);

	initialized = true;

	return true;
}

bool TSerialFlash::InitInterface()
{
	return true;
}

bool TSerialFlash::InitInherited() // must be overridden
{
	return true;
}


void TSerialFlash::Run()  // must be overridden
{
	errorcode = ERROR_NOTIMPL;
	completed = true;
}

bool TSerialFlash::ReadIdCode()
{
	idcode = 0;
	return false;
}

bool TSerialFlash::StartReadMem(unsigned aaddr, void * adstptr, unsigned alen)
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

	dataptr = (uint8_t *)adstptr;
	datalen = alen;
	address = aaddr;

	state = SERIALFLASH_STATE_READMEM; // read memory
	phase = 0;

	errorcode = 0;
	completed = false;

	Run();

	return (errorcode == 0);
}

bool TSerialFlash::StartWriteMem(unsigned aaddr, void* asrcptr, unsigned alen)
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

	dataptr = (uint8_t *)asrcptr;
	datalen = alen;
	address = aaddr;

	state = SERIALFLASH_STATE_WRITEMEM; // write page
	phase = 0;

	errorcode = 0;
	completed = false;

	Run();

	return (errorcode == 0);
}

bool TSerialFlash::StartEraseMem(unsigned aaddr, unsigned alen)
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

	dataptr = nullptr;
	datalen = alen;
	address = aaddr;

	state = SERIALFLASH_STATE_ERASE; // erase memory
	phase = 0;

	errorcode = 0;
	completed = false;

	Run();

	return (errorcode == 0);
}

bool TSerialFlash::StartEraseAll()
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

	state = SERIALFLASH_STATE_ERASEALL; // erase chip
	phase = 0;

	errorcode = 0;
	completed = false;

	Run();

	return (errorcode == 0);
}

void TSerialFlash::WaitForComplete()
{
	while (!completed)
	{
		Run();
	}
}

