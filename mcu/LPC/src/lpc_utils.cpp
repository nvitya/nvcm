/*
 * lpc_utils.cpp
 *
 *  Created on: Dec 13, 2017
 *      Author: nagy
 */

#include "lpc_utils.h"

void lpc_enable_clock(unsigned index, unsigned value)
{
	if (index >= 322)
	{
		LPC_CCU2->CLKCCU[index - 322].CFG |= value;
	}
	else
	{
		LPC_CCU1->CLKCCU[index].CFG |= value;
	}
}

void lpc_disable_clock(unsigned index, unsigned value)
{
	if (index >= 322)
	{
		LPC_CCU2->CLKCCU[index - 322].CFG &= ~1;
	}
	else
	{
		LPC_CCU1->CLKCCU[index].CFG &= ~1;
	}
}

void lpc_set_clock_divider(CHIP_CGU_IDIV_T Divider, CHIP_CGU_CLKIN_T Input, uint32_t Divisor)
{
	uint32_t reg = LPC_CGU->IDIV_CTRL[Divider];

	Divisor--;

	if (Input != CLKINPUT_PD)
	{
		reg &= ~((0x1F << 24) | 1 | (CHIP_CGU_IDIV_MASK(Divider) << 2));

		// Enable autoblocking, clear PD, and set clock source & divisor
		LPC_CGU->IDIV_CTRL[Divider] = reg | (1 << 11) | (Input << 24) | (Divisor << 2);
	}
	else
	{
		LPC_CGU->IDIV_CTRL[Divider] = reg | 1;	// Power down this divider
	}
}

void lpc_set_base_clock(CHIP_CGU_BASE_CLK_T BaseClock, CHIP_CGU_CLKIN_T Input, bool autoblocken, bool powerdn)
{
	uint32_t reg = LPC_CGU->BASE_CLK[BaseClock];

	if (BaseClock < CLK_BASE_NONE)
	{
		if (Input != CLKINPUT_PD)
		{
			// Mask off fields we plan to update
			reg &= ~((0x1F << 24) | 1 | (1 << 11));

			if (autoblocken)
			{
				reg |= (1 << 11);
			}

			if (powerdn)
			{
				reg |= (1 << 0);
			}

			reg |= (Input << 24);

			LPC_CGU->BASE_CLK[BaseClock] = reg;
		}
	}
	else
	{
		LPC_CGU->BASE_CLK[BaseClock] = reg | 1;	// Power down this base clock
	}
}

