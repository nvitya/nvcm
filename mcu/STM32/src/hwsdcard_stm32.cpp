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
 *  file:     hwsdcard_stm32.cpp
 *  brief:    STM32 SDCARD driver
 *  version:  1.00
 *  date:     2020-12-26
 *  authors:  nvitya
*/

#include "platform.h"
#include "hwsdcard.h"
#include "stm32_utils.h"
#include "clockcnt.h"

#include "traces.h"

#if defined(SDMMC) || defined(SDMMC1)

bool THwSdcard_stm32::HwInit()
{
	unsigned code;
	unsigned perid;
	//unsigned periphclock = atsam_peripheral_clock();

	regs = SDMMC1;

#if defined(RCC_AHB3ENR_SDMMC1EN) // H7

	RCC->AHB3ENR |= RCC_AHB3ENR_SDMMC1EN;
	if (RCC->AHB3ENR) { } // some syncing

	// reset
	RCC->AHB3RSTR |= RCC_AHB3RSTR_SDMMC1RST;
	if (RCC->AHB3RSTR) { } // some syncing
	RCC->AHB3RSTR &= ~RCC_AHB3RSTR_SDMMC1RST;
	if (RCC->AHB3RSTR) { } // some syncing

#else
	RCC->APB2ENR |= RCC_APB2ENR_SDMMC1EN;
	if (RCC->APB2ENR) { } // some syncing

	// reset
	RCC->APB2RSTR |= RCC_APB2RSTR_SDMMC1RST;
	if (RCC->APB2RSTR) { } // some syncing
	RCC->APB2RSTR &= ~RCC_APB2RSTR_SDMMC1RST;
	if (RCC->APB2RSTR) { } // some syncing
#endif


#if defined(SDMMC_IDMA_IDMAEN)
	// use the IDMA
#else
	dma.Init(2, dma_stream, 4); // channel 4
#endif

	regs->POWER = (0
		| (1  <<  4)  // DIRPOL
		| (0  <<  3)  // VSWITCHEN
		| (0  <<  2)  // VSWITCH
		| (0  <<  0)  // PWRCTRL(2): 3 = power on, 0 = disabled, 2 = power cycle
	);

	if (regs->POWER) { }

#ifdef MCUSF_H7
	regs->CLKCR = (0
		| (0  << 20)  // SELCLKRX(2): 0 = internal
		| (0  << 19)  // BUSSPEED: 0 = 25 MHz, 1 = 50 MHz
		| (0  << 18)  // DDR: 0 = single data rate
		| (0  << 17)  // HWFC_EN: 1 = hardware flow control enable
		| (0  << 16)  // NEGEDGE
		| (0  << 14)  // WIDBUS(2): 0 = 1 bit, 1 = 4 bit, 2 = 8 bit
		| (0  << 12)  // PWRSAV
		| (0  <<  0)  // CLKDIV(10)
	);
#else
	regs->CLKCR = (0
		| (0  << 14)  // HWFC_EN: 1 = hardware flow control enable
		| (0  << 16)  // NEGEDGE
		| (0  << 11)  // WIDBUS(2): 0 = 1 bit, 1 = 4 bit, 2 = 8 bit
		| (0  <<  9)  // PWRSAV
		| (0  <<  8)  // CLKEN
		| (0  <<  0)  // CLKDIV(8)
	);
#endif

	SetSpeed(initial_speed);
	SetBusWidth(1);

	// power on card
	regs->POWER |= 3; // power on

	// clear the data fifo
	regs->DCTRL = (0
		| (1  << 13)  // FIFORST
		| (0  << 12)  // BOOTACKEN
		| (0  << 11)  // SDIOEN
		| (0  << 10)  // RWMOD
		| (0  <<  9)  // RWSTOP
		| (0  <<  8)  // RWSTART
		| (0  <<  4)  // DBLOCKSIZE(4): 9 = 512
		| (0  <<  2)  // DTMODE(2)
		| (0  <<  1)  // DTDIR: 0 = host to card, 1 = card to host
		| (0  <<  0)  // DTEN: 1 = start data without CPSM transfer command
	);

	// wait 74 SD clycles

	delay_us(200);

	return true;
}

void THwSdcard_stm32::SetSpeed(uint32_t speed)
{
#ifdef MCUSF_H7
	uint32_t periphclock = stm32_bus_speed(0); // get AHB bus speed
#else
	uint32_t periphclock = stm32_bus_speed(2); // get APB2 bus speed
#endif

	uint32_t clkdiv = 0;
	while ((periphclock >> clkdiv) > speed)
	{
		++clkdiv;
	}

	uint32_t tmp = regs->CLKCR;

#ifdef MCUSF_H7
	uint32_t hsbus = 0;
	if (speed > 25000000)
	{
		hsbus = 1;
	}
	tmp &= ~(0x3FF | (1 << 19));
	tmp |= ((hsbus << 19) | (clkdiv << 0));
#else
	tmp &= ~0xFF;
	tmp |= (clkdiv << 0);
	tmp |= SDMMC_CLKCR_CLKEN;
#endif

	regs->CLKCR = tmp;
}

void THwSdcard_stm32::SetBusWidth(uint8_t abuswidth)
{
	uint32_t tmp = regs->CLKCR;
	tmp &= ~(3 << SDMMC_CLKCR_WIDBUS_Pos);

	if (4 == abuswidth)
	{
		tmp |= (1 << SDMMC_CLKCR_WIDBUS_Pos);
	}
	else
	{
		// already zero
	}

	regs->CLKCR = tmp;
}

bool THwSdcard_stm32::CmdFinished()
{
	uint32_t sr = regs->STA;

	if (sr & (SDMMC_STA_CTIMEOUT | SDMMC_STA_CCRCFAIL | SDMMC_STA_DCRCFAIL | SDMMC_STA_CTIMEOUT | SDMMC_STA_DTIMEOUT))
	{
		if (sr & SDMMC_STA_CCRCFAIL)
		{
			return true;
		}

	  cmderror = true;
		return true;
	}

	if (SDCMD_RES_NO == (curcmdflags & SDCMD_RES_MASK))
	{
		if (sr & SDMMC_STA_CMDSENT)
		{
			return true;
		}
		else
		{
			return false;
		}
	}

#ifdef MCUSF_H7
	if (sr & (SDMMC_STA_CMDREND | SDMMC_STA_BUSYD0END))
	{
		return true;
	}
#else
	if (sr & (SDMMC_STA_CMDREND))
	{
		return true;
	}
#endif

	return false;
}

void THwSdcard_stm32::GetCmdResult128(void * adataptr)
{
	uint32_t * dst = (uint32_t *)adataptr;
	*dst++ = regs->RESP4;
	*dst++ = regs->RESP3;
	*dst++ = regs->RESP2;
	*dst++ = regs->RESP1;
}

uint32_t THwSdcard_stm32::GetCmdResult32()
{
	return regs->RESP1;
}

void THwSdcard_stm32::SendCmd(uint8_t acmd, uint32_t cmdarg, uint32_t cmdflags)  // send command without data transfer
{
	curcmd = acmd;
	curcmdarg = cmdarg;
	curcmdflags = cmdflags;

	uint32_t waitresp = 0;
	uint8_t restype = (cmdflags & SDCMD_RES_MASK);
	if (restype)
	{
		// response present
		if (restype == SDCMD_RES_48BIT)        waitresp = 1; // short response
		else if (restype == SDCMD_RES_136BIT)  waitresp = 3; // long response
		else if (restype == SDCMD_RES_R1B)     waitresp = 1; // short response
	}

	uint32_t cmdr = (0
		|	SDMMC_CMD_CPSMEN
		| (waitresp << SDMMC_CMD_WAITRESP_Pos)
		| (acmd <<  SDMMC_CMD_CMDINDEX_Pos)
	);


	regs->ICR = 0xFFFFFFFF; // clear all flags
	regs->DLEN = 0;

	regs->ARG = cmdarg;
	regs->CMD = cmdr; // start the execution

	cmderror = false;
	cmdrunning = true;
	lastcmdtime = CLOCKCNT;
}


void THwSdcard_stm32::StartDataReadCmd(uint8_t acmd, uint32_t cmdarg, uint32_t cmdflags, void * dataptr, uint32_t datalen)
{
	regs->DTIMER = 0xFFFFFF; // todo: adjust data timeout

  // setup data control register
	uint32_t dcr = (0
		| (0  << 13)  // FIFORST
		| (0  << 12)  // BOOTACKEN
		| (0  << 11)  // SDIOEN
		| (0  << 10)  // RWMOD
		| (0  <<  9)  // RWSTOP
		| (0  <<  8)  // RWSTART
		| (0  <<  4)  // DBLOCKSIZE(4): 0 = 1 byte, 9 = 512
		| (0  <<  2)  // DTMODE(2): 0 = single block, 3 = multiple blocks
		| (1  <<  1)  // DTDIR: 0 = host to card, 1 = card to host
		| (0  <<  0)  // DTEN: 1 = start data without CPSM transfer command
	);

	if (datalen <= 512)
	{
		// single block mode, the data length must be power of two !

		uint32_t lz = 31-__CLZ(datalen);
		dcr |= (0
			| (lz  <<  4) // DBLOCKSIZE(4): 0 = 1 byte, 9 = 512
	  );
	}
	else
	{
		// multi block mode, the data length must be mutiple of 512 !
		dcr |= (0
			| (1  << 13)  // FIFORST
			| (9  <<  4)  // DBLOCKSIZE(4): 0 = 1 byte, 9 = 512
	  );
	}

#ifdef MCUSF_H7
	// the DMA must be started before DCTRL (DTEN)
	regs->IDMABASE0 = (uint32_t)dataptr;
	regs->IDMABSIZE = datalen; // not really necessary for single buffer mode ?
	regs->IDMACTRL = 1; // enable the internal DMA
#else

#endif

	regs->DLEN = datalen;
	regs->DCTRL = dcr;

	// CMD

	curcmd = acmd;
	curcmdarg = cmdarg;
	curcmdflags = cmdflags;

	uint32_t cmdr = (0
		| (0  << 16)  // CMDSUSPEND
		| (0  << 15)  // BOOTEN
		| (0  << 14)  // BOOTMODE
		| (0  << 13)  // DTHOLD
		| (1  << 12)  // CPSMEN: 1 = starts the command state machinde
		| (0  << 11)  // WAITPEND
		| (0  << 10)  // WAITINT
		| (0  <<  8)  // WAITRESP(2): 0 = no response, 1 = short, 2 = short + no CRC, 3 = long
		| (0  <<  7)  // CMDSTOP
		| (1  <<  6)  // CMDTRANS: ???
		| (acmd <<  0)  // CMDINDEX(6)
	);

	uint8_t restype = (cmdflags & SDCMD_RES_MASK);
	if (restype)
	{
		// response present
		if (restype == SDCMD_RES_48BIT)        cmdr |= (1 << 8); // short response
		else if (restype == SDCMD_RES_136BIT)  cmdr |= (3 << 8); // long response
		else if (restype == SDCMD_RES_R1B)     cmdr |= (1 << 8); // short response
	}

	regs->ICR = 0xFFFFFFFF; // clear all flags

	regs->ARG = cmdarg;
	regs->CMD = cmdr; // start the execution

	cmderror = false;
	cmdrunning = true;
	lastcmdtime = CLOCKCNT;
}

void THwSdcard_stm32::RunTransfer()
{
	if (cmdrunning && !CmdFinished())
	{
		return;
	}

	uint8_t  cmd;
	uint32_t cmdarg;

	cmdrunning = false;

	switch (trstate)
	{
	case 0: // idle
		break;

	case 1: // start read blocks
		cmdarg = startblock;
		if (!high_capacity)  cmdarg <<= 9; // byte addressing for low capacity cards
		cmd = (blockcount > 1 ? 18 : 17);

		StartDataReadCmd(cmd, cmdarg, SDCMD_RES_48BIT, dataptr, blockcount * 512);

		trstate = 101;
		break;

	case 101: // wait until the block transfer finishes

		if (cmderror)
		{
			// transfer error
			errorcode = 1;
			completed = true;
			trstate = 0; // transfer finished.
		}
		else
		{
			if (0 == (regs->STA & SDMMC_STA_DATAEND))
			{
				return;
			}

			if (blockcount > 1)
			{
				// send the stop transmission command
				SendCmd(12, 0, SDCMD_RES_R1B);
				trstate = 106;  // wait until the stop command finishes
			}
			else
			{
				completed = true;
				trstate = 0; // finished
			}
		}
		break;

	case 106: // wait until transfer done flag set

		if (cmderror)
		{
			// transfer error
			errorcode = 2;
		}
		completed = true;
		trstate = 0; // transfer finished.
		break;
	}

}

#endif
