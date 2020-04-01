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
 *  file:     hwextirq_atsam_v2.h
 *  brief:    ATSAM V2 Extenal Pin Interrupt
 *  version:  1.00
 *  date:     2020-04-01
 *  authors:  nvitya
*/

#ifndef _HWEXTIRQ_ATSAM_V2_H
#define _HWEXTIRQ_ATSAM_V2_H

#define HWEXTIRQ_PRE_ONLY
#include "hwextirq.h"

class THwExtIrq_atsam_v2 : public THwExtIrq_pre
{
public:
	volatile uint32_t *   irqpend_reg = nullptr;
	volatile uint32_t *   irqack_reg = nullptr;
	uint32_t              irq_mask = 0;

	// platform specific
	bool Init(int aextintnum, unsigned flags);

	void Enable();
	void Disable();

	ALWAYS_INLINE void IrqBegin()   {  } // not reqired for this MCU
	ALWAYS_INLINE bool IrqPending() { return (*irqpend_reg & irq_mask); }
	ALWAYS_INLINE void IrqAck()     { *irqack_reg = irq_mask; }
};

#define HWEXTIRQ_IMPL   THwExtIrq_atsam_v2

#endif /* HWEXTIRQ_ATSAM_V2_H_ */
