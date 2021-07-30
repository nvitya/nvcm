/* -----------------------------------------------------------------------------
 * This file is a part of the NVCM project: https://github.com/nvitya/nvcm
 * Copyright (c) 2019 Viktor Nagy, nvitya
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
 *  file:     hwintflash.cpp
 *  brief:    Interface for Internal Flash Memory Handling
 *  version:  1.00
 *  date:     2019-02-23
 *  authors:  nvitya
*/

#include "platform.h"
#include "hwintflash.h"

#include "traces.h"

THwIntFlash hwintflash;

bool THwIntFlash::Init()
{
	initialized = false;

	if (!HwInit())
	{
		return false;
	}

	pagemask = pagesize - 1;
	blockmask = erasesize - 1;
	completed = true;
	initialized = true;
	return true;
}

void THwIntFlash::TraceFlashInfo()
{
	TRACE("Internal Flash = %u k, erase = %u k, page = %u\r\n", bytesize >> 10, erasesize >> 10, pagesize);
}

void THwIntFlash::WaitForComplete()
{
	while (!completed)
	{
		Run();
	}
}

bool THwIntFlash::StartEraseMem(uint32_t aaddr, uint32_t alen)
{
	if (!initialized)
	{
		errorcode = INTFLASH_ERROR_NOTINIT;
		completed = true;
		return false;
	}

	// aaddr must be at block start
	if (aaddr & blockmask)
	{
		errorcode = INTFLASH_ERROR_ADDRESS;
		return false;
	}

	if (!completed)
	{
		errorcode = INTFLASH_ERROR_BUSY;  // this might be overwriten later
		return false;
	}

	address = aaddr; // already checked
	length = ((alen + blockmask) & ~blockmask);

	state = INTFLASH_STATE_ERASE;
	phase = 0;
	completed = false;

	Run();

	return true;
}

bool THwIntFlash::StartWriteMem(uint32_t aaddr, void * asrcptr, uint32_t alen)
{
	if (!initialized)
	{
		errorcode = INTFLASH_ERROR_NOTINIT;
		completed = true;
		return false;
	}

	if (!completed)
	{
		errorcode = INTFLASH_ERROR_BUSY;  // this might be overwriten later
		return false;
	}

	address = (aaddr & 0xFFFFFFFC); // ensure 4 byte boundary !
	length = ((alen + 3) & 0xFFFFFFFC); // ensure that it is rounded to the minimum
	srcaddr = (uint32_t *)asrcptr;
	dstaddr = (uint32_t *)address;

	state = INTFLASH_STATE_WRITEMEM;
	phase = 0;
	completed = false;

	Run();

	return true;
}

bool THwIntFlash::StartCopyMem(uint32_t aaddr, void * asrcptr, uint32_t alen)
{
	if (!initialized)
	{
		errorcode = INTFLASH_ERROR_NOTINIT;
		completed = true;
		return false;
	}

	// aaddr must be at block start
	if (aaddr & blockmask)
	{
		errorcode = INTFLASH_ERROR_ADDRESS;
		return false;
	}

	if (!completed)
	{
		errorcode = INTFLASH_ERROR_BUSY;  // this might be overwriten later
		return false;
	}

	address = aaddr; // alignment checked before
	length = ((alen + 3) & 0xFFFFFFFC); // ensure that it is rounded to the minimum
	srcaddr = (uint32_t *)asrcptr;
	dstaddr = (uint32_t *)address;

	state = INTFLASH_STATE_COPY;
	phase = 0;
	completed = false;

	Run();

	return true;
}

#ifndef HWINTFLASH_OWN_RUN

void THwIntFlash::Run()
{
	if (0 == state)
	{
		return;  // idle
	}

	if (INTFLASH_STATE_ERASE == state)
	{
		switch (phase)
		{
			case 0: // start
				errorcode = 0;
				remaining = length;
				++phase;
				// no break!

			case 1: // prepare / continue
				ebchunk = erasesize;
				if (remaining < ebchunk)  ebchunk = remaining;  // important for the uint remaining

				CmdEraseBlock();
				++phase;
				break;

			case 2: // check finished
				if (CmdFinished())
				{
					address += ebchunk;
					remaining -= ebchunk;
					if (remaining > 0)
					{
						phase = 1; Run(); return; // continue with the next page
					}
					else
					{
						completed = true;
						state = 0;
					}
				}
				break;
		}
	}
	else if (INTFLASH_STATE_WRITEMEM == state)
	{
		switch (phase)
		{
			case 0: // start
				errorcode = 0;
				remaining = length;
				++phase;
				// no break!

			case 1: // prepare / continue
				chunksize = pagesize;
				chunksize -= (address & pagemask);
				if (remaining < chunksize)  chunksize = remaining;

				if (chunksize != pagesize)
				{
					CmdClearPageBuffer(); // Clear Page Buffer
				}
				++phase;
				break;

			case 2:
				if (CmdFinished())
				{
					// Clear Page Buffer is ready

					// TODO: provide DMA version too

					// fill the page buffer
					uint32_t * endaddr = srcaddr + (chunksize >> 2);
					while (srcaddr < endaddr)
					{
						*dstaddr++ = *srcaddr++;
					}

					CmdWritePage(); // start write the page (using the last access address)
					++phase;
				}
				break;

			case 3:
				if (CmdFinished()) // wait the write to be ready
				{
					address += chunksize;
					remaining -= chunksize;
					if (remaining > 0)
					{
						phase = 1; Run(); return; // continue with the next page
					}
					else
					{
						completed = true;
						state = 0;
					}
				}
				break;
		}
	}
	else if (INTFLASH_STATE_COPY == state)  // with automatic erase, block alignment is mandatory
	{
		switch (phase)
		{
			case 0: // start
				errorcode = 0;
				remaining = length;
				++phase;
				// no break!

			case 1: // prepare / continue
			{
				ebchunk = erasesize;
				if (remaining < ebchunk)  ebchunk = remaining;  // important for the uint remaining

				ebremaining = ebchunk;

				// compare contents for wrong 0 bits
				uint32_t * sptr = srcaddr;
				uint32_t * dptr = dstaddr;
				uint32_t * endaddr = srcaddr + (ebchunk >> 2);
				bool erase_required = false;
				bool match = true;
				while (sptr < endaddr)
				{
					uint32_t sv = *sptr++;
					uint32_t dv = *dptr++;

					if (sv != dv)  
					{
						match = false;
						if ((sv & dv) != sv)  // if the flash content has elsewhere zeroes as the source
						{
							erase_required = true;
							break;
						}
					}
				}

				if (match)
				{
					//TRACE("Flash content match at %08X\r\n", address);
					srcaddr += (ebchunk >> 2); // adjust addresses here, which normally incremented during write
					dstaddr += (ebchunk >> 2);
					address += ebchunk;
					phase = 10; // block finished
				}
				else if (erase_required)
				{
					//TRACE("Flash content erase required at %08X\r\n", address);
					CmdEraseBlock();
					phase = 4;
				}
				else
				{
					//TRACE("Flash content overwrite at %08X\r\n", address);
					phase = 6; Run(); return; // jump to write
				}
				break;
			}

			case 4: // wait the erase to finish
				if (CmdFinished())
				{
					phase = 6; Run(); return; // jump to write
				}
				break;

			case 6: // start / continue write
				chunksize = pagesize;
				chunksize -= (address & pagemask); // should not happen here
				if (ebremaining < chunksize)  chunksize = ebremaining;

				if (chunksize != pagesize)
				{
					CmdClearPageBuffer(); // Clear Page Buffer
				}
				++phase;
				break;

			case 7:
				if ((chunksize == pagesize) || CmdFinished())
				{
					// Clear Page Buffer is ready

					// fill the page buffer
					uint32_t * endaddr = srcaddr + (chunksize >> 2);
					while (srcaddr < endaddr)
					{
						*dstaddr++ = *srcaddr++;
					}

					CmdWritePage(); // start write the page (using the last access address)
					++phase;
				}
				break;

			case 8:
				if (CmdFinished()) // wait the write to be ready
				{
					address += chunksize;
					ebremaining -= chunksize;
					if (ebremaining > 0)
					{
						phase = 6; Run(); return; // continue with the next page
					}
					else
					{
						phase = 10; Run(); return;
					}
				}
				break;

			case 10: // block finished
				remaining -= ebchunk;
				if (remaining > 0)
				{
					phase = 1; Run(); return; // continue with the block
				}
				else
				{
					completed = true;
					state = 0;
				}
				break;
		}
	}
	else // invalid state
	{
		completed = true;
		errorcode = 0;
		state = 0;
	}
}

#endif
