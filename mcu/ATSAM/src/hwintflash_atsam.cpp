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
 *  file:     hwintflash_atsam.h
 *  brief:    Internal Flash Handling for ATSAM
 *  version:  1.00
 *  date:     2019-04-07
 *  authors:  nvitya
*/

#include <stdio.h>
#include <stdarg.h>

#include "platform.h"

#include "hwintflash.h"

#include "traces.h"

bool THwIntFlash_atsam::HwInit()
{
	int i;

#ifdef EFC
	regs = EFC;
#else
	regs = EFC0;
#endif

	// first get the Flash Descriptor

	regs->EEFC_FCR = 0
		| (0    <<  0)  // FCMD(8): 0 = get flash descriptor
		| (0    <<  8)  // FARG(16): no arguments required
		| (0x5A << 24)  // FKEY(8): must be 5A
	;

	while (!CmdFinished())
	{
		// wait
	}

	uint32_t fdesc[16];

	for (i = 0; i < 16; ++i)
	{
		fdesc[i] = regs->EEFC_FRR;
	}

	bytesize  = fdesc[1];
	pagesize  = fdesc[2];
	erasesize = pagesize * 8;  // this is the smallest erase unit, that can be used everywhere

	// fix parameters:
	smallest_write = 4;
	start_address = 0x00400000;

	return true;
}

void THwIntFlash_atsam::CmdEraseBlock()
{
	uint32_t ebnum = (address - start_address) / erasesize;

	regs->EEFC_FCR = 0
		| (7     <<  0)  // FCMD(8): 7 = Erase Pages
		| (1     <<  8)  // FARG(0..1): 1 = 8x PAGES
		| (0     << 10)  // FARG(2): not used
		| (ebnum << 11)  // FARG(3..15): page number
		| (0x5A  << 24)  // FKEY(8): must be 5A
	;
}

void THwIntFlash_atsam::CmdWritePage()
{
	uint32_t page = (address - start_address) / pagesize;

	regs->EEFC_FCR = 0
		| (1     <<  0)  // FCMD(8): 1 = Write Page
		| (page  <<  8)  // FARG(16): page number
		| (0x5A  << 24)  // FKEY(8): must be 5A
	;
}

void THwIntFlash_atsam::CmdClearPageBuffer()
{
	// there is no such command on this MCU, so it has to be done manually

	uint32_t baseaddr = (address & ~pagemask);

	uint32_t * dptr = (uint32_t *)baseaddr;
	uint32_t * endptr = dptr + (pagesize >> 2);

	while (dptr < endptr)
	{
		*dptr++ = 0xFFFFFFFF;
	}
}
