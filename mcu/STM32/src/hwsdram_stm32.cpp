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
 *  file:     hwsdram_stm32.cpp
 *  brief:    STM32 SDRAM controller
 *  version:  1.00
 *  date:     2018-11-27
 *  authors:  nvitya
*/

#include "platform.h"

#if defined(FMC_SDCR1_CAS)

#include "hwsdram_stm32.h"
#include "clockcnt.h"

bool THwSdram_stm32::InitHw()
{
	uint32_t commonbits;
	uint32_t bankbits;

  // Enable the FMC interface clock
  RCC->AHB3ENR |= RCC_AHB3ENR_FMCEN;

  regs = FMC_Bank5_6;

  /*##-1- Configure the SDRAM device #########################################*/
  /* SDRAM device configuration */

  if (2 == bank)
  {
  	address = 0xD0000000;
  }
  else
  {
  	address = 0xC0000000;
  }

  int bankidx = bank - 1;

  commonbits = 0
		| ((hclk_div              & 3) << 10)  // SDCLK(2): 2 or 3 x HCLK
		| ((read_burst_enable ? 1 : 0) << 12)  // RBURST: Read burst enable
		| ((rpipe_delay           & 3) << 13)  // RPIPE(2): Read pipe delay
	;

  bankbits = 0
		| (((column_bits - 8)     & 3) <<  0)  // NC(2): 0 = 8 bits
		| (((row_bits - 11)       & 3) <<  2)  // NR(2): 0 = 11 bits
		| (((data_bus_width >> 4) & 3) <<  4)  // MWID(2): 1 = 16 bit memory width
		| (((bank_count >> 2)     & 1) <<  6)  // NB: 0 = 2 banks, 1 = 4 Banks
		| (((cas_latency)         & 3) <<  7)  // CAS(2): 2
		| ((0                     & 1) <<  9)  // WP: 0 = write access allowed
	;

  //st lib F429: 0x2c00 for bank 0

  if (2 == bank)
  {
  	regs->SDCR[0] = commonbits;
  	regs->SDCR[1] = bankbits;
  }
  else
  {
  	regs->SDCR[0] = (commonbits | bankbits);
  	regs->SDCR[1] = 0;
  }

  // SDRAM Timing

  commonbits = 0
		| ((4                         & 15) <<  0)  // TMRD(4): Load Mode Register to Active
		| ((exit_self_refresh_delay   & 15) <<  4)  // TXSR(4): Exit Self-refresh delay
		| ((active_to_precharge_delay & 15) <<  8)  // TRAS(4): Self refresh time
  	| ((row_cycle_delay           & 15) << 12)  // TRC(4): Row cycle delay
  	| ((row_to_column_delay       & 15) << 24)  // TRCD(4): Row to column delay
  ;

  bankbits = 0
  	| ((recovery_delay            & 15) << 16)  // TWR(4): Recovery delay
  	| ((row_precharge_delay       & 15) << 20)  // TRP(4): Row precharge delay
  ;

  if (2 == bank)
  {
  	regs->SDTR[0] = commonbits;
  	regs->SDTR[1] = bankbits;
  }
  else
  {
  	regs->SDTR[0] = (commonbits | bankbits);
  	regs->SDTR[1] = 0;
  }

  // let the HwSdram class do the configuration of the SDRAM device

	return true;
}

void THwSdram_stm32::Cmd_ClockEnable()
{
	SendCommand(1, 0, 0);
  delay_us(1000); // extra delay required
}

void THwSdram_stm32::Cmd_AllBankPrecharge()
{
	SendCommand(2, 0, 0);
}

void THwSdram_stm32::Cmd_AutoRefresh(int acount)
{
	SendCommand(3, 0, acount);
}

void THwSdram_stm32::Cmd_LoadModeRegister(uint16_t aregvalue)
{
	SendCommand(4, aregvalue, 0);
}

void THwSdram_stm32::Cmd_LoadExtModeRegister(uint16_t aregvalue)
{
  //
}

void THwSdram_stm32::SetNormalMode()
{
	//SendCommand(0, 0, 0);
}

void THwSdram_stm32::SetRefreshTime(uint32_t atime_ns)
{
	uint32_t sdramclock = SystemCoreClock / hclk_div;
	uint32_t clks = sdramclock / 1000;
	clks *= atime_ns;
	clks /= 1000000;
	regs->SDRTR = (clks << 1) | 1;  // bit0: clear refresh error
  delay_us(1000); // extra delay required ?
}

void THwSdram_stm32::SendCommand(uint16_t command, uint32_t mrdata, uint32_t refrcnt)
{
	uint32_t banksel = (bank == 2 ? 1 : 2);

	regs->SDCMR = 0
		| ((command & 7)        <<  0)  // MODE(3): 0 = normal mode
		| (banksel              <<  3)  // CBT2: 1 = Send the command to the Bank2, 2 = send bank-1
		| (((refrcnt - 1) & 15) <<  5)  // NRFS(4): Number of Auto-refresh, 0 = 1, 1 = 2, ... 15 = 16 cycles
		| ((mrdata & 0x1FFF)    <<  9)  // MRD(13): Mode register data
	;

	delay_us(1);

  while (regs->SDSR & 0x00000020)
  {
  	// wait until the command finished
  }
}

#endif
