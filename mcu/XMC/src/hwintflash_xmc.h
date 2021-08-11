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
 *  file:     hwintflash_xmc.h
 *  brief:    Internal Flash Handling for XMC4000
 *  version:  1.00
 *  date:     2021-07-30
 *  authors:  nvitya
*/

#ifndef HWINTFLASH_XMC_H_
#define HWINTFLASH_XMC_H_

#define HWINTFLASH_PRE_ONLY
#include "hwintflash.h"

#ifdef MCUSF_4000

#if !defined(MCU_FLASH_SIZE)
  #define MCU_FLASH_SIZE    64
#endif

#define HWREGS_FLASH   FLASH0_GLOBAL_TypeDef

class THwIntFlash_xmc : public THwIntFlash_pre
{
public:
	bool             HwInit();

public:
	HWREGS_FLASH *   regs = nullptr;

	void             Run();

	void             CmdEraseBlock();   // at address
	void             CmdProgramPage();  // from the pagebuf
	bool             CmdFinished();

protected:

	uint32_t         uc_start_address = 0x0C000000;
	uint32_t         addr_mask        = 0x001FFFFF;

	uint16_t *       src16;
	uint16_t *       dst16;

	int              BlockIdFromAddress(uint32_t aaddress);
	uint32_t         BlockSizeFromAddress(uint32_t aaddress);

	uint8_t          pagebuf[256];

	void             ClearStatusCommand();
	void             CmdEnterPageMode();
	void             CmdWritePage();

	void             ProgramNextPage();

};

#define HWINTFLASH_IMPL     THwIntFlash_xmc

#define HWINTFLASH_OWN_RUN

#endif // MCUSF_4000

#endif // def HWINTFLASH_XMC_H_
