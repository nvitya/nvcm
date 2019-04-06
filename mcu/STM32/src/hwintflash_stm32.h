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
 *  file:     hwintflash_stm32.h
 *  brief:    Internal Flash Handling for STM32
 *  version:  1.00
 *  date:     2019-03-31
 *  authors:  nvitya
*/

#ifndef HWINTFLASH_STM32_H_
#define HWINTFLASH_STM32_H_

#define HWINTFLASH_PRE_ONLY
#include "hwintflash.h"

#if defined(MCUSF_F4) || defined(MCUSF_F7)
  #define HWINTFLASH_BIGBLOCKS  1
#else
  #define HWINTFLASH_BIGBLOCKS  0
#endif

class THwIntFlash_stm32 : public THwIntFlash_pre
{
public:
	bool             HwInit();

public:
	FLASH_TypeDef *  regs = nullptr;

	//bool           StartFlashCmd(uint8_t acmd);

	void             CmdEraseBlock(); // at address
	void             CmdWritePage();

	bool             CmdFinished();

	void             Run();

public:

	void             Unlock();

protected:

	uint16_t *       src16;
	uint16_t *       dst16;

	void             Write32(uint32_t * adst, uint32_t avalue);

#if HWINTFLASH_BIGBLOCKS

	uint32_t         cr_reg_base;

	int              BlockIdFromAddress(uint32_t aaddress);
#endif

};

#define HWINTFLASH_IMPL     THwIntFlash_stm32

#define HWINTFLASH_OWN_RUN

#endif // def HWINTFLASH_STM32_H_
