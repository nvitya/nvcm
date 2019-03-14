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

#ifndef HWINTFLASH_ATSAM_V2_H_
#define HWINTFLASH_ATSAM_V2_H_

#define HWINTFLASH_PRE_ONLY
#include "hwintflash.h"

class THwIntFlash_atsam_v2 : public THwIntFlash_pre
{
public:
	bool           HwInit();

public:
	Nvmctrl *      regs = nullptr;

	bool           StartFlashCmd(uint8_t acmd);

	void           CmdEraseBlock(); // at address
	void           CmdWritePage();
	void           CmdClearPageBuffer();

	inline bool    CmdFinished() { return (regs->INTFLAG.reg & 1); }
};

#define HWINTFLASH_IMPL THwIntFlash_atsam_v2

#endif // def HWINTFLASH_ATSAM_V2_H_
