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
#include "atsam_v2_utils.h"

static bool g_extint_initialized = false;

void atsam2_extint_init()
{
	if (g_extint_initialized)
	{
		return;
	}

	atsam2_set_periph_gclk(EIC_GCLK_ID, 0);  // setup EIC clock
	MCLK->APBAMASK.bit.EIC_ = 1;   // enable EIC clocks

	EIC->CTRLA.reg = 0; // disable
	EIC->CTRLA.bit.SWRST = 1;
	while (EIC->SYNCBUSY.bit.SWRST)  { } // wait for sync

	EIC->NMICTRL.reg = 0; // disable NMI
	EIC->EVCTRL.reg = 0;
	EIC->INTENCLR.reg = 0xFFFF; // disable all interrupts
	EIC->INTFLAG.reg = 0xFFFF;  // clear all pending IRQs

	g_extint_initialized = true;
}

bool atsam2_extint_enable(bool aenable)
{
	bool wasenable = (EIC->CTRLA.bit.ENABLE == 1);
	if (aenable)
	{
		EIC->CTRLA.bit.ENABLE = 1;
	}
	else
	{
		EIC->CTRLA.bit.ENABLE = 0;
	}
	while (EIC->SYNCBUSY.bit.ENABLE)  { } // wait for sync
	return wasenable;
}

bool THwExtIrq_atsam_v2::Init(int aextintnum, unsigned flags)
{
	atsam2_extint_init();

	atsam2_extint_enable(false);

	irq_mask = (1 << aextintnum);
	irqpend_reg = &EIC->INTFLAG.reg;
	irqack_reg  = &EIC->INTFLAG.reg;

	uint32_t intcfg = 0;
	if (flags & HWEXTIRQ_RISING)   intcfg |= 1;
	if (flags & HWEXTIRQ_FALLING)  intcfg |= 2;

	uint8_t regshift = ((aextintnum & 7) << 2);
	uint8_t regidx = ((aextintnum >> 3) & 1);
	uint32_t tmp;
	tmp = EIC->CONFIG[regidx].reg;
	tmp &= ~(15     << regshift);
	tmp |=  (intcfg << regshift);
	EIC->CONFIG[regidx].reg = tmp;

	IrqAck();
	Enable();

	atsam2_extint_enable(true);

  return true;
}

void THwExtIrq_atsam_v2::Enable()
{
	EIC->INTENSET.reg = irq_mask;
}

void THwExtIrq_atsam_v2::Disable()
{
	EIC->INTENCLR.reg = irq_mask;
}

