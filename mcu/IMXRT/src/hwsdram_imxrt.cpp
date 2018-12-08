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
 *  file:     hwsdram_imxrt.cpp
 *  brief:    IMXRT SDRAM controller
 *  version:  1.00
 *  date:     2018-11-24
 *  authors:  nvitya
*/

#include "platform.h"

#include "hwsdram_imxrt.h"
#include "imxrt_utils.h"
#include "clockcnt.h"

typedef enum _semc_ipcmd_sdram
{
	kSEMC_SDRAMCM_Read = 0x8U, /*!< SDRAM memory read. */
	kSEMC_SDRAMCM_Write,       /*!< SDRAM memory write. */
	kSEMC_SDRAMCM_Modeset,     /*!< SDRAM MODE SET. */
	kSEMC_SDRAMCM_Active,      /*!< SDRAM active. */
	kSEMC_SDRAMCM_AutoRefresh, /*!< SDRAM auto-refresh. */
	kSEMC_SDRAMCM_SelfRefresh, /*!< SDRAM self-refresh. */
	kSEMC_SDRAMCM_Precharge,   /*!< SDRAM precharge. */
	kSEMC_SDRAMCM_Prechargeall /*!< SDRAM precharge all. */
} semc_ipcmd_sdram_t;


bool THwSdram_imxrt::InitHw()
{
	uint32_t tmp;
	uint32_t sdramidx = 0;

	// enable the SDRAM controller clock

	imxrt_set_clock_gate(3,  4, 3);  // SEMC Clock
	imxrt_set_clock_gate(1, 18, 3);  // SEMC Ext Clock

	regs = SEMC;

  regs->MCR = 0
  	| SEMC_MCR_MDIS_MASK // disable now
  	| SEMC_MCR_BTO(31)   // bus timeout
  	| SEMC_MCR_CTO(0)    // command timeout
  	| SEMC_MCR_DQSMD(0)  // 0 = Dummy read strobe loopbacked internally
  ;

#if 1
  // Configure Queue 0/1 for AXI bus.
  regs->BMCR0 = 0
  	| (5  <<  0)  // WQOS(4): Weight of QoS
  	| (8  <<  4)  // WAGE(4): Weight of Aging
  	| (64 <<  8)  // WSH(8): Weight of Slave Hit (no read/write switch)
  	| (16 << 16)  // WRWS(8): Weight of Slave Hit (Read/Write switch)
  ;

  regs->BMCR1 = 0
  	| (5    <<  0)  // WQOS(4): Weight of QoS
  	| (8    <<  4)  // WAGE(4): Weight of Aging
  	| (0x60 <<  8)  // WPH(8): Weight of Page Hit
  	| (0x24 << 16)  // WRWS(8): Weight of Read/Write switch
  	| (0x40 << 24)  // WBR(8): Weight of Bank Rotation
  ;
#endif

  // Enable SEMC.
  regs->MCR &= ~SEMC_MCR_MDIS_MASK;

  // Setup IOCR - Pin mux register
  tmp = regs->IOCR;
  tmp &= ~SEMC_IOCR_MUX_A8_MASK; // set SEMC_A8 to Address 8
  tmp &= ~SEMC_IOCR_MUX_CSX0_MASK;
  tmp |= SEMC_IOCR_MUX_CSX0(sdramidx);  // select SEMC_CSX0 to SDRAM_CS0 (wrong in the reference manual)
  regs->IOCR = tmp;

  // Base register

  // calculate memory size code
  uint32_t sizecode = 0;
  uint32_t memsize = byte_size / (8 * 1024);
  while (memsize)
  {
		memsize >>= 1;
		sizecode++;
  }
  regs->BR[sdramidx] = (HWSDRAM_ADDRESS & SEMC_BR_BA_MASK) | SEMC_BR_MS(sizecode) | SEMC_BR_VLD_MASK;

  // SDRAMCR0
  regs->SDRAMCR0 = 0
  	| SEMC_SDRAMCR0_PS(data_bus_width >> 4)  // port size
  	| SEMC_SDRAMCR0_BL(burst_length - 1)     // burst length
  	| SEMC_SDRAMCR0_COL(12 - column_bits)    // column bits
  	| SEMC_SDRAMCR0_CL(cas_latency)          // CAS latency
  ;

  uint32_t sdramclk = SystemCoreClock / 4; // = 125 MHz

  // Timing settings
  regs->SDRAMCR1 = 0
  	| SEMC_SDRAMCR1_PRE2ACT(row_precharge_delay - 1) // Trp
    | SEMC_SDRAMCR1_ACT2RW(row_to_column_delay - 1) // Trcd
    | SEMC_SDRAMCR1_RFRC(21 - 1) // Refresh recovery time, cycle delay between REFRESH command to ACTIVE command
    | SEMC_SDRAMCR1_WRC(recovery_delay - 1) // write recovery clocks + 1
    | SEMC_SDRAMCR1_CKEOFF(active_to_precharge_delay - 1) // The minimum cycle of SDRAM CLK off state. CKE is off in self refresh at a minimum period tRAS.
    | SEMC_SDRAMCR1_ACT2PRE(active_to_precharge_delay - 1) // Tras
  ;

  regs->SDRAMCR2 = 0
    | SEMC_SDRAMCR2_SRRC(exit_self_refresh_delay - 1)
    | SEMC_SDRAMCR2_REF2REF(exit_self_refresh_delay)
    | SEMC_SDRAMCR2_ACT2ACT(10)
    | SEMC_SDRAMCR2_ITO(0x80) // idle timeout in prescaled (=x16) clocks
  ;

  SetRefreshTime(refresh_time_ns);

  // let the HwSdram class do the configuration of the SDRAM device

	return true;
}

void THwSdram_imxrt::Cmd_ClockEnable()
{
	// not required for IMXRT ?
}

void THwSdram_imxrt::Cmd_AllBankPrecharge()
{
	SendIpCommand(HWSDRAM_ADDRESS, kSEMC_SDRAMCM_Prechargeall, 0);
}

	void THwSdram_imxrt::Cmd_AutoRefresh(int acount)
{
	for (uint16_t n = 0; n < acount; ++n)
	{
		SendIpCommand(HWSDRAM_ADDRESS, kSEMC_SDRAMCM_AutoRefresh, (n + 1));
	}
}

void THwSdram_imxrt::Cmd_LoadModeRegister(uint16_t aregvalue)
{
	SendIpCommand(HWSDRAM_ADDRESS, kSEMC_SDRAMCM_Modeset, aregvalue);
}

void THwSdram_imxrt::Cmd_LoadExtModeRegister(uint16_t aregvalue)
{
	// empty
}

void THwSdram_imxrt::SetNormalMode()
{
  /* Enables refresh */
  regs->SDRAMCR3 |= SEMC_SDRAMCR3_REN_MASK;
}

void THwSdram_imxrt::SetRefreshTime(uint32_t atime_ns)
{
	// fix 16 clock prescaler is used

	// for a 65 ms full refresh: 65000000/8192 = 7934 ns  row refresh time = 992 clocks (125 MHz)
	// with 16 prescaler it fits nicely into 8 bit.

	uint32_t periphclock = SystemCoreClock / 4;
	uint32_t clks = periphclock / 1000;
	clks *= atime_ns;
	clks /= 1000000;
	clks >>= 4; // divide by 16

  regs->SDRAMCR3 = 0
  	| SEMC_SDRAMCR3_REBL(0)       // refresh burst length
    | SEMC_SDRAMCR3_PRESCALE(1)   // prescale = 16 clocks
    | SEMC_SDRAMCR3_RT(clks)
    | SEMC_SDRAMCR3_UT(clks)
  ;
}

void THwSdram_imxrt::SendIpCommand(uint32_t address, uint16_t command, uint32_t data)
{
  regs->IPCR1 = 0x2;  // set data size to 2
  regs->IPCR2 = 0;    // clear byte masking

  regs->INTR |= SEMC_INTR_IPCMDDONE_MASK;  // Clear status bit
  regs->IPCR0 = address;
  regs->IPTXDAT = data;
  regs->IPCMD = command | SEMC_IPCMD_KEY(0xA55A);
  while (!(regs->INTR & SEMC_INTR_IPCMDDONE_MASK))
  {
  };

  regs->INTR |= SEMC_INTR_IPCMDDONE_MASK;
}

