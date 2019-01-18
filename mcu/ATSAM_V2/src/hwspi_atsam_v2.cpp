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
 *  file:     hwspi_atsam_v2.cpp
 *  brief:    ATSAM_V2 SPI
 *  version:  1.00
 *  date:     2019-01-17
 *  authors:  nvitya
*/

#include <stdio.h>
#include <stdarg.h>

#include "hwspi_atsam_v2.h"

static const Sercom * sercom_inst_list[] = SERCOM_INSTS;

bool THwSpi_atsam_v2::Init(int adevnum)  // devnum: 0 - 7 = SERCOM ID
{
	uint32_t tmp;
	unsigned perid;

	devnum = adevnum;
	initialized = false;
	regs = nullptr;

	if (devnum < 0)
	{
		return false;
	}
	else if (devnum >= SERCOM_INST_NUM)
	{
		return false;
	}
#if defined(MCUSF_E5X)
	else if (devnum < 2)
	{
		MCLK->APBAMASK.reg |= (1 << (12 + devnum));  // Enable/unmask CPU interface (register access)
		perid = 7 + devnum;
	}
	else if (devnum < 4)
	{
		MCLK->APBBMASK.reg |= (1 << (9 + devnum - 2)); // Enable/unmask CPU interface (register access)
		perid = 23 + (devnum - 2);
	}
	else if (devnum < 8)
	{
		MCLK->APBDMASK.reg |= (1 << (devnum - 4)); // Enable/unmask CPU interface (register access)
		perid = 34 + (devnum - 4);
	}
	else
	{
		return false;
	}

	// setup peripheral clock
	GCLK->PCHCTRL[perid].reg = ((0 << 0) | (1 << 6));   // select main clock frequency (120 MHz) + enable

#elif defined(MCUSF_C2X)
	else if (devnum <= 5)
	{
		MCLK->APBCMASK.reg |= (1 << (1 + devnum));  // enable register interface
		perid = 19 + devnum;
		if (devnum == 5)
		{
			GCLK->PCHCTRL[25].reg = (1 << 6) | (0 << 0);  // Enable the peripheral and select GEN0 (main clock)
			GCLK->PCHCTRL[24].reg = (1 << 6) | (3 << 0);  // Select the SERCOM5 slow clock
		}
		else
		{
			GCLK->PCHCTRL[19 + devnum].reg = (1 << 6) | (0 << 0);  // Enable the peripheral and select GEN0 (main clock)
			GCLK->PCHCTRL[18].reg = (1 << 6) | (3 << 0);  // Select the SERCOM slow clock
		}
	}
	else
	{
		return false;
	}
#elif defined(MCUSF_D10)
	else
	{
		PM->APBCMASK.reg |= (1 << (2 + devnum));  // enable register interface
		GCLK->CLKCTRL.reg = 0x430E + devnum;      // Select GCLK3 for SERCOM
	}

#else
  #error "UART Unimplemented."
#endif


	regs = (HW_SPI_REGS *)sercom_inst_list[devnum];

	regs->CTRLA.bit.ENABLE = 0; // disable
	//regs->CTRLA.bit.SWRST = 1; // reset
	//while (regs->SYNCBUSY.bit.SWRST) { } // wait for reset
	//regs->CTRLA.bit.SWRST = 0; // reset

	while (regs->SYNCBUSY.bit.ENABLE) { } // wait for sync

	uint32_t periphclock = (SystemCoreClock >> 1);

	unsigned brdiv = (periphclock / speed);
	if (brdiv < 1)  brdiv = 1;

	regs->BAUD.reg = (brdiv - 1);

	while (regs->SYNCBUSY.bit.CTRLB) { } // wait for sync

	// CTRLB
	regs->CTRLB.reg = 0
		| (1 << 17)  // RXEN: RX Enable
		| (0 << 14)  // AMODE(2): Address mode
		| (0 << 13)  // MSSEN: 1 = HW CS(SS) control + inter character spacing
		| (0 <<  9)  // SSDE: 1 = Slave Select Low Detect Enable
		| (0 <<  6)  // PLOADEN: 1 = data preload enable
		| (0 <<  0)  // CHSIZE(3): 0 = 8 bit, 1 = 9 bit, more options are not supported
	;

	while (regs->SYNCBUSY.bit.CTRLB) { } // wait for sync

	// CTRLA
	tmp = 0
		| (1 << 30)  // DORD: 1 = LSB first
		| (0 << 29)  // CPOL: Clock Polarity, 1 = SCK is high on idle
		| (0 << 28)  // CPHA: 1 = late sample
		| (0 << 24)  // FORM(4): frame format, 0 = SPI, 2 = SPI with address
		| (3 << 20)  // DIPO(2): Data In Pinout, pad select for MISO
		| (0 << 20)  // DOPO(2): Data Out Pinout, 0 = P0:MOSI|P1:SCK|P2:SS
		| (0 <<  8)  // IBON: Immediate Buffer Overflow Notification
		| (0 <<  7)  // RUNSTDBY: Run In Standby, 1 = run in stdby
		| (3 <<  2)  // MODE(3): Mode, 3 = SPI Master
		| (0 <<  1)  // ENABLE
		| (0 <<  0)  // SWRST: Software Reset
	;

	if (idleclk_high)     tmp |= (1 << 29);
	if (datasample_late)  tmp |= (1 << 28);

	regs->CTRLA.reg = tmp;
	regs->DBGCTRL.reg = 0;

	while (regs->SYNCBUSY.bit.ENABLE) { } // wait for enable
	regs->CTRLA.reg = (tmp | (1 << 1)); // enable it
	while (regs->SYNCBUSY.bit.ENABLE) { } // wait for enable

	initialized = true;

	return true;
}

bool THwSpi_atsam_v2::TrySendData(unsigned short adata)
{
	if (regs->INTFLAG.bit.DRE)
	{
		regs->DATA.reg = adata;
		return true;
	}
	else
	{
		return false;
	}
}

bool THwSpi_atsam_v2::TryRecvData(unsigned short * dstptr)
{
	if (regs->INTFLAG.bit.RXC)
	{
		*dstptr = regs->DATA.reg;
		return true;
	}
	else
	{
		return false;
	}
}




