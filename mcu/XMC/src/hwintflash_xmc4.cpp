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
 *  file:     hwintflash_xmc4.cpp
 *  brief:    Internal Flash Handling for XMC4000
 *  version:  1.00
 *  date:     2021-07-30
 *  authors:  nvitya
 *  notes:
 *    WARNING: the flash erased to 0x00-s instead of 0xFF at this MCU family !
*/

#include "platform.h"

#ifdef MCUSF_4000

#include <stdio.h>
#include <stdarg.h>
#include "string.h"

#include "hwintflash_xmc.h"
#include "hwintflash.h"

#include "hwclkctrl.h"

#include "traces.h"

bool THwIntFlash_xmc::HwInit()
{
	hwclkctrl.StartIntHSOsc();  // required for Flash Writes

	regs = FLASH0;

	// it seems so that there is no way to detect the the actual Flash size on the XMC4 processors

	bytesize = MCU_FLASH_SIZE * 1024;

	pagesize = 256;

	bank_count = 1;

	// The XMC4000 has different block/sector sizes:
	//   - the first 8 sectors have the size of 16k
	//   - the 9th sector is 128k
	//   - from the 10th sector the size is 256k

  erasesize = 16 * 1024;  // the smallest erase size, used for check the erase alignment

	smallest_write = 8;  // to be more compatible

	start_address = 0x08000000;  // the cached start address

	return true;
}

int THwIntFlash_xmc::BlockIdFromAddress(uint32_t aaddress)
{
	uint32_t maskedaddr = (aaddress & addr_mask);

	if (maskedaddr < 128 * 1024)  // the first 128k is divided into 8 x 16 k blocks
	{
		return (maskedaddr >> 14);
	}
	else if (maskedaddr < 256 * 1024)  // then one 128k block
	{
		return 8;
	}
	else
	{
		return 8 + (maskedaddr >> 18);
	}
}

uint32_t THwIntFlash_xmc::BlockSizeFromAddress(uint32_t aaddress)
{
	uint32_t maskedaddr = (aaddress & addr_mask);

	if (maskedaddr < 128 * 1024)  // the first 128k is divided into 8 x 16 k blocks
	{
		return 16 * 1024;
	}
	else if (maskedaddr < 256 * 1024)
	{
		return 128 * 1024;  // then one 128k block
	}
	else
	{
		return 256 * 1024;  // the rest is 256k
	}
}

void THwIntFlash_xmc::Run()
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
				ebchunk = BlockSizeFromAddress(address);
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
			{
				ProgramNextPage(); // updates: chunksize, srcaddr, dstaddr

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
				errorcode = 0;
				remaining = length;
				++phase;
				// no break!

			case 1: // prepare / continue
			{
				ebchunk = BlockSizeFromAddress(address);
				if (remaining < ebchunk)  ebchunk = remaining;  // important for the uint remaining

				ebremaining = ebchunk;

				// compare contents for wrong 1 bits (XMC erases to 0)
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
						if ((sv | dv) != sv)  // if the flash content has ones elswhere as the source
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
				ProgramNextPage(); // updates: chunksize, srcaddr
				++phase;
				break;

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

bool THwIntFlash_xmc::CmdFinished()
{
  if (regs->FSR & FLASH_FSR_PBUSY_Msk)
	{
		return false;
	}
  else
  {
  	return true;
  }
}

/*
 * Command to clear FSR.PROG and FSR.ERASE and the error flags in FSR such as PFOPER, SQER, PROER, PFDBER, ORIER, VER
 */
void THwIntFlash_xmc::ClearStatusCommand()
{
  volatile uint32_t *  addr;

  addr = (uint32_t *)(uc_start_address + 0x5554);
  *addr = 0xF5;
}

void THwIntFlash_xmc::CmdEraseBlock()
{
	ClearStatusCommand();

  volatile uint32_t *  addr;

  addr = (uint32_t *)(uc_start_address + 0x5554);
  *addr = 0xAA;
  addr = (uint32_t *)(uc_start_address + 0xAAA8);
  *addr = 0x55;
  addr = (uint32_t *)(uc_start_address + 0x5554);
  *addr = 0x80;
  addr = (uint32_t *)(uc_start_address + 0x5554);
  *addr = 0xAA;
  addr = (uint32_t *)(uc_start_address + 0xAAA8);
  *addr = 0x55;
  addr = (uint32_t *)(uc_start_address + (address & addr_mask));
  *addr = 0x30;
}

void THwIntFlash_xmc::CmdEnterPageMode()
{
  volatile uint32_t *  addr;

  addr = (uint32_t *)(uc_start_address + 0x5554);
  *addr = 0x50;
}

void THwIntFlash_xmc::CmdWritePage()
{
  volatile uint32_t *  addr;

  addr = (uint32_t *)(uc_start_address + 0x5554);
  *addr = 0xAA;
  addr = (uint32_t *)(uc_start_address + 0xAAA8);
  *addr = 0x55;
  addr = (uint32_t *)(uc_start_address + 0x5554);
  *addr = 0xA0;
  addr = (uint32_t *)(uc_start_address + ((address & ~pagemask) & addr_mask));
  *addr = 0xAA;
}


void THwIntFlash_xmc::CmdProgramPage()
{
  volatile uint32_t * addr;
  uint32_t * pu32;

	ClearStatusCommand();
	CmdEnterPageMode();

	// load the page buffer into the internal assembly buffer (64 bit wide)

	pu32 = (uint32_t *)&pagebuf[0];

  for (int idx = 0; idx < 32; ++idx)
  {
  	addr = (uint32_t *)(uc_start_address + 0x55F0);
 	  *addr = *pu32;
 	  ++pu32;

  	addr = (uint32_t *)(uc_start_address + 0x55F4);
  	*addr = *pu32;
  	++pu32;
  }

  // write the assembly buffer into the flash

  CmdWritePage();
}

void THwIntFlash_xmc::ProgramNextPage()
{
	memset(pagebuf, 0, sizeof(pagebuf)); // clear the page buffer (0 is the erased state)
	                                     // for the possible partial writes

	chunksize = pagesize;
	chunksize -= (address & pagemask);
	if (remaining < chunksize)  chunksize = remaining;

	// fill the page buffer
	uint32_t * pu32 = (uint32_t *)&pagebuf[address & pagemask];
	uint32_t * endaddr = srcaddr + (chunksize >> 2);
	while (srcaddr < endaddr)
	{
		*pu32++ = *srcaddr++;
		++dstaddr; // for the copy function important to follow the dst address (for the compare)
	}

	CmdProgramPage();
}

#endif // MCUSF_4000
