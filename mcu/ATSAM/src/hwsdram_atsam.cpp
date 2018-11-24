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
 *  file:     hwsdram_atsam.cpp
 *  brief:    ATSAM SDRAM controller
 *  version:  1.00
 *  date:     2018-11-24
 *  authors:  nvitya
*/

#include "platform.h"

#if defined(SDRAMC)

#include "hwsdram_atsam.h"
#include "clockcnt.h"

bool THwSdram_atsam::InitHw()
{
	// enable the SDRAM controller
	MATRIX->CCFG_SMCNFCS = CCFG_SMCNFCS_SDRAMEN;
	PMC->PMC_PCER1 = (1 << 30);

	regs = SDRAMC;

	// configure the SDRAM Controller

	// CR register
	regs->SDRAMC_CR = 0
	  | (((column_bits - 8)   & 3)   <<  0)   // NC(2): number of columns, 0 = 8 bits
	  | (((row_bits - 11)     & 3)   <<  2)   // NR(2): number of rows, 0 = 11 bits
	  | (((bank_count >> 2)   & 1)   <<  4)   // NB(1): number of banks, 1 = 4 banks, 0 = 2 banks
	  | (((cas_latency)       & 3)   <<  5)   // CAS(2): CAS latency, 3 = 3 cycles
	  | (((data_bus_width >> 4) & 1) <<  7)   // DBW: data bus width, 1 = 16 bits
// timing
	  | ((recovery_delay            & 15) <<  8)   // TWR(4): Write Recovery Delay
	  | ((row_cycle_delay           & 15) << 12)   // TRC_TRFC(4): Row Cycle Delay and Row Refresh Cycle
	  | ((row_precharge_delay       & 15) << 16)   // TRP(4): Row Precharge Delay
	  | ((row_to_column_delay       & 15) << 20)   // TRCD(4): Row to Column Delay
	  | ((active_to_precharge_delay & 15) << 24)   // TRAS(4): Active to Precharge Delay
	  | ((exit_self_refresh_delay   & 15) << 28)   // TXSR(4): Exit Self-Refresh to Active Delay
	;

	/* For low-power SDRAM, Temperature-Compensated Self Refresh (TCSR),
	   Drive Strength (DS) and Partial Array Self Refresh (PASR) must be set
	   in the Low-power Register. */
	SDRAMC->SDRAMC_LPR = 0;

	SDRAMC->SDRAMC_CFR1 |= SDRAMC_CFR1_UNAL;  // enable unaligned support

	delay_us(300);

	/* Program the memory device type into the Memory Device Register */
	SDRAMC->SDRAMC_MDR = SDRAMC_MDR_MD_SDRAM;

  // let the HwSdram class do the configuration of the SDRAM device

	return true;
}

void THwSdram_atsam::Cmd_Nop()
{
	SDRAMC->SDRAMC_MR = SDRAMC_MR_MODE_NOP;
	*(volatile uint16_t *)(HWSDRAM_ADDRESS) = 0x0;
}

void THwSdram_atsam::Cmd_AllBankPrecharge()
{
	SDRAMC->SDRAMC_MR = SDRAMC_MR_MODE_ALLBANKS_PRECHARGE;
	*(volatile uint16_t *)(HWSDRAM_ADDRESS) = 0x0;
}

	void THwSdram_atsam::Cmd_AutoRefresh(int acount)
{
	for (uint16_t n = 0; n < acount; ++n)
	{
		SDRAMC->SDRAMC_MR = SDRAMC_MR_MODE_AUTO_REFRESH;
		*(volatile uint16_t *)(HWSDRAM_ADDRESS) = (n + 1);
	}
}

void THwSdram_atsam::Cmd_LoadModeRegister(uint16_t aregvalue)
{
	SDRAMC->SDRAMC_MR = SDRAMC_MR_MODE_LOAD_MODEREG;
	*(volatile uint16_t *)(HWSDRAM_ADDRESS + aregvalue) = 0xCAFE;
}

void THwSdram_atsam::Cmd_LoadExtModeRegister(uint16_t aregvalue)
{
	SDRAMC->SDRAMC_MR = SDRAMC_MR_MODE_EXT_LOAD_MODEREG;
	*(volatile uint16_t *)(HWSDRAM_ADDRESS + bank1_offset) = aregvalue;
}

void THwSdram_atsam::SetNormalMode()
{
	SDRAMC->SDRAMC_MR = SDRAMC_MR_MODE_NORMAL;
	*(volatile uint16_t *)(HWSDRAM_ADDRESS) = 0;
}

void THwSdram_atsam::SetRefreshTime(uint32_t atime_ns)
{
	uint32_t periphclock = SystemCoreClock / 2;
	uint32_t clks = periphclock / 1000;
	clks *= atime_ns;
	clks /= 1000000;
	SDRAMC->SDRAMC_TR = SDRAMC_TR_COUNT(clks);
}

#endif
