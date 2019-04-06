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
 *  file:     hwintflash.h
 *  brief:    Internal Flash Handling for ATSAM V2
 *  version:  1.00
 *  date:     2019-02-23
 *  authors:  nvitya
*/

#include <stdio.h>
#include <stdarg.h>

#include "platform.h"

#include "hwintflash.h"

#include "traces.h"

bool THwIntFlash_atsam_v2::HwInit()
{
	regs = NVMCTRL;

	pagesize = (8 << ((NVMCTRL->PARAM.reg >> 16) & 7));
	pagecount = (NVMCTRL->PARAM.reg & 0xFFFF);
	bytesize = pagesize * pagecount;
	blocksize = pagesize * 16;  // usually 8192

	// fix parameters:
	smallest_write = 4;
	bank_count = 2;

	start_address = 0;

	return true;
}

void THwIntFlash_atsam_v2::CmdEraseBlock()
{
	regs->INTFLAG.reg = 1; // clear done
	regs->ADDR.reg = address;
	regs->CTRLB.reg = 0
		| (0xA5 << 8)   // CMDEX(8): command execution key, fix 0xA5
		| (0x01 << 0)   // CMD(7):
	;
}

void THwIntFlash_atsam_v2::CmdWritePage()
{
	regs->INTFLAG.reg = 1; // clear done
	regs->CTRLB.reg = 0
		| (0xA5 << 8)   // CMDEX(8): command execution key, fix 0xA5
		| (0x03 << 0)   // CMD(7):
	;
}

void THwIntFlash_atsam_v2::CmdClearPageBuffer()
{
	regs->INTFLAG.reg = 1; // clear done
	regs->CTRLB.reg = 0
		| (0xA5 << 8)   // CMDEX(8): command execution key, fix 0xA5
		| (0x15 << 0)   // CMD(7):
	;
}
