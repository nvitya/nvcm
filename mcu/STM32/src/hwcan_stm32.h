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
 *  file:     hwcan_stm32.h
 *  brief:    STM32 CAN
 *  version:  1.00
 *  date:     2019-01-12
 *  authors:  nvitya
*/

#ifndef HWCAN_STM32_H_
#define HWCAN_STM32_H_

#include "platform.h"

#if defined(CAN_BASE)

#define HWCAN_PRE_ONLY
#include "hwcan.h"

#define HW_CAN_REGS CAN_TypeDef

class THwCan_stm32 : public THwCan_pre
{
public: // mandatory
	bool HwInit(int adevnum);

	void Enable();
	bool Enabled();

	void HandleTx();
	void HandleRx();

	void AcceptListClear();
	void AcceptAdd(uint16_t cobid, uint16_t amask);

	bool IsBusOff();
	bool IsWarning();

public:
	HW_CAN_REGS *      regs = nullptr;

	uint8_t            filtercnt = 0;
};

#define HWCAN_IMPL THwCan_stm32

#endif

#endif /* HWCAN_STM32_H_ */
