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
 *  file:     hwextirq_atsam.h
 *  brief:    ATSAM Extenal Pin Interrupt
 *  version:  1.00
 *  date:     2020-04-01
 *  authors:  nvitya
*/

#ifndef _HWEXTIRQ_ATSAM_H
#define _HWEXTIRQ_ATSAM_H

#include "hwpins.h"

#define HWEXTIRQ_PRE_ONLY
#include "hwextirq.h"

class THwExtIrq_atsam : public THwExtIrq_pre
{
public:
	HW_GPIO_REGS *        regs = nullptr;

	uint32_t *            pisr_shadow = nullptr;
	uint32_t              pin_mask = 0;

	// platform specific
	bool Init(int aportnum, int apinnum, unsigned flags);

	void Enable();
	void Disable();

	ALWAYS_INLINE void IrqBegin()   { *pisr_shadow = regs->PIO_ISR; } // this clears the ISR flags, so it must be buffered

	ALWAYS_INLINE bool IrqPending() { return (*pisr_shadow & pin_mask); }
	ALWAYS_INLINE void IrqAck()     {  } // no IRQ ack required !
};

#define HWEXTIRQ_IMPL   THwExtIrq_atsam

#endif /* HWEXTIRQ_ATSAM_H_ */
