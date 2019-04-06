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
 *  file:     hwintflash_stm32.cpp
 *  brief:    Internal Flash Handling for STM32
 *  version:  1.00
 *  date:     2019-03-31
 *  authors:  nvitya
*/

#include <stdio.h>
#include <stdarg.h>

#include "platform.h"

#include "hwintflash_stm32.h"
#include "hwintflash.h"

#include "hwclkctrl.h"

#include "traces.h"

bool THwIntFlash_stm32::HwInit()
{
	hwclkctrl.StartIntHSOsc();  // required for Flash Writes

	regs = FLASH;

	bytesize = *(uint16_t *)(FLASHSIZE_BASE) * 1024;

	if (bytesize <= 64 * 1024)
	{
		erasesize = 1024;
	}
	else
	{
		erasesize = 2048;
	}

	pagesize = 256; // used here as burst length

	smallest_write = 4;  // to be more compatible
	bank_count = 1;

	start_address = 0;

	return true;
}

bool THwIntFlash_stm32::CmdFinished()
{
	if (regs->SR & FLASH_SR_BSY)
	{
		return false;
	}

	if (regs->SR & FLASH_SR_EOP)
	{
		regs->SR = FLASH_SR_EOP; // clear EOP flag
	}

	regs->CR = 0; // clear all CMD flags

	return true;
}

void THwIntFlash_stm32::CmdEraseBlock()
{
	regs->CR = FLASH_CR_PER; // prepare page erase
	regs->AR = address;
	regs->CR = (FLASH_CR_STRT | FLASH_CR_PER); // start page erase
}

void THwIntFlash_stm32::Run()
{
	uint32_t n;

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

				Unlock();

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

				Unlock();

				errorcode = 0;
				remaining = length;
				++phase;
				// no break!

			case 1: // prepare / continue
			{
				chunksize = pagesize;
				if (remaining < chunksize)  chunksize = remaining;

				uint32_t * endaddr = srcaddr + (chunksize >> 2);
				while (srcaddr < endaddr)
				{
					Write32(dstaddr, *srcaddr);
					++dstaddr;
					++srcaddr;
				}

				++phase;
				break;
			}

			case 2:
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

				Unlock();

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

					if (sv != dv)  match = false;
					if (sv != 0)  // zeroes can be programmed any time
					{
						erase_required = true;
						break;
					}
				}

				if (match)
				{
					//TRACE("Flash content match at %08X\r\n", address);
					srcaddr += (ebchunk >> 2); // adjust addresses here, which normally incremented during write
					dstaddr += (ebchunk >> 2);
					address += (ebchunk >> 2);
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
			{
				chunksize = pagesize;
				if (ebremaining < chunksize)  chunksize = ebremaining;

				uint32_t * endaddr = srcaddr + (chunksize >> 2);
				while (srcaddr < endaddr)
				{
					Write32(dstaddr, *srcaddr);
					++dstaddr;
					++srcaddr;
				}

				++phase;
				break;
			}

			case 7:
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

void THwIntFlash_stm32::Unlock()
{
	if (regs->CR & FLASH_CR_LOCK_Msk)
	{
		regs->KEYR = 0x45670123;
		regs->KEYR = 0xCDEF89AB;

		if (regs->CR & FLASH_CR_LOCK_Msk)
		{
			TRACE("Flash Unlock Failed !\r\n");
		}
	}
}

void THwIntFlash_stm32::Write32(uint32_t * adst, uint32_t avalue)
{
	while (!CmdFinished())
	{
		// wait
	}

	regs->CR = FLASH_CR_PG;

	uint16_t * dst16 = (uint16_t *)adst;

	*dst16 = (avalue & 0xFFFF);
	++dst16;

	while (!CmdFinished())
	{
		// wait
	}

	regs->CR = FLASH_CR_PG;

	*dst16 = (avalue >> 16);
}
