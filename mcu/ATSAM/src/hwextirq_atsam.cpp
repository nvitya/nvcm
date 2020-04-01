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
 *  file:     hwextirq_atsam_v2.cpp
 *  brief:    ATSAM V2 Extenal Pin Interrupt
 *  version:  1.00
 *  date:     2020-04-01
 *  authors:  nvitya
*/

#include "platform.h"
#include "hwextirq.h"
#include "atsam_utils.h"

static uint32_t pio_isr_shadow[8];

bool THwExtIrq_atsam::Init(int aportnum, int apinnum, unsigned flags)
{
	regs = hwpinctrl.GetGpioRegs(aportnum);
	if (!regs)
	{
		return false;
	}

	pin_mask = (1 << apinnum);
	pisr_shadow = &pio_isr_shadow[aportnum];

	// by default both rising and falling edge causes an interrupt
	// the both edge case must be handled specially

	if ((flags & HWEXTIRQ_RISING) && (flags & HWEXTIRQ_FALLING))
	{
		regs->PIO_AIMDR = pin_mask; // disable additional mode = both edge will be detected
	}
	else
	{
		regs->PIO_AIMER = pin_mask; // enable additional mode
		regs->PIO_ESR = pin_mask;   // select edge mode

		if (flags & HWEXTIRQ_FALLING)
		{
			regs->PIO_FELLSR = pin_mask;  // select falling edge
		}
		else  // rising edge
		{
			regs->PIO_REHLSR = pin_mask;  // select rising edge
		}
	}

	IrqBegin();  // reads ISR register, that clears the pending IRQs
	Enable();

  return true;
}

void THwExtIrq_atsam::Enable()
{
	regs->PIO_IER = pin_mask;
}

void THwExtIrq_atsam::Disable()
{
	regs->PIO_IDR = pin_mask;
}

