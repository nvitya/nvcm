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
 *  file:     hwi2c_atsam_v2.cpp
 *  brief:    ATSAM V2 I2C
 *  version:  1.00
 *  date:     2019-02-16
 *  authors:  nvitya
*/

#include "platform.h"
#include "hwpins.h"
#include "hwi2c_atsam_v2.h"
#include "atsam_v2_utils.h"

bool THwI2c_atsam_v2::Init(int adevnum)
{
	unsigned tmp;
	unsigned perid;

	initialized = false;
	devnum = adevnum;
	regs = nullptr;

	uint8_t clksrc = 0;  // main clock by default
	uint32_t periphclock = SystemCoreClock;
	if (periphclock > 50000000)
	{
		clksrc = 2; // use the internal 48 MHz oscillator
		periphclock = 48000000;
	}

	if (!atsam2_sercom_enable(devnum, clksrc))
	{
		return false;
	}

	regs = (SercomI2cm *)sercom_inst_list[devnum];

	regs->CTRLA.bit.ENABLE = 0; // disable
	while (regs->SYNCBUSY.bit.ENABLE) { } // wait for disable

	// CTRLA
	regs->CTRLA.reg = 0
		| (0 << 30)  // LOWTOUT: 0 = disabled
		| (0 << 28)  // INACTOUT(2): 0 = disabled
		| (0 << 27)  // SCLSM: 1 = SCL stretch only after ACK bit
		| (0 << 24)  // SPEED(2): 0 = standard (max 400 kHz), 1 = fast mode plus (max 1 MHz), 2 = High-speed (max 3.4 MHz)
		| (0 << 23)  // SEXTTOEN:
		| (0 << 22)  // MEXTTOEN:
		| (0 << 20)  // SDAHOLD(2):
		| (0 << 16)  // PINOUT: 0 = 4 wire disabled
		| (0 <<  7)  // RUNSTDBY: 1 = enable in standby
		| (5 <<  2)  // MODE(3): 5 = I2C Master
		| (0 <<  1)  // ENABLE: (do not enable yet)
		| (0 <<  0)  // SWRST: 1 = software reset
	;


	unsigned clockdiv = (periphclock / speed);
	uint32_t halfclocks = ((clockdiv - 10) >> 1);
	if (halfclocks > 255)  halfclocks = 255;

	regs->BAUD.reg = 0
		| (halfclocks <<  0)  // BAUD(8):
		| (0          <<  8)  // BAUDLOW(8):  0 = use 2x BAUD
		| (0          << 16)  // HSBAUD(8):
		| (0          << 24)  // HSBAUDLOW(8):
	;

#ifdef MCUSF_E5X
	regs->CTRLC.reg = 0;  // disable 32 bit mode
#endif

	regs->CTRLB.reg = 0
		| (0 << 18)  // ACKACT: 0 = send ACK after a byte received, 0 = send NACK
		| (0 << 16)  // CMD(2): 0 = no action
		| (0 <<  9)  // QCEN: 0 = quick command is disabled
		| (1 <<  8)  // SMEN: 1 = send ACK automatically
	;

	regs->CTRLA.bit.ENABLE = 1; // enable
	while (regs->SYNCBUSY.bit.ENABLE) { } // wait for disable

	// force IDLE state, clear clearable flags

	regs->STATUS.reg = 0
		| (1 << 10)  // LENERR
		| (1 <<  9)  // SEXTTOUT
		| (1 <<  8)  // MEXTTOUT
		| (0 <<  7)  // CLKHOLD
		| (1 <<  6)  // LOWTOUT
		| (1 <<  4)  // BUSSTATE(2): 1 = force state to idle
		| (0 <<  2)  // RXNACK
		| (1 <<  1)  // ARBLOST
		| (1 <<  0)  // BUSERR
	;

	while (regs->SYNCBUSY.bit.SYSOP) { } // wait for complete

	initialized = true;

	return true;
}

int THwI2c_atsam_v2::StartReadData(uint8_t adaddr, unsigned aextra, void * dstptr, unsigned len)
{
	if (busy)
	{
		return ERROR_BUSY;
	}

	istx = false;
	error = 0;
	devaddr = (adaddr & 0x7F);
	dataptr = (uint8_t *)dstptr;
	datalen = len;
	remainingbytes = datalen;
	waitreload = false;

	extracnt = ((aextra >> 24) & 3);
	if (extracnt)
	{
		// reverse byte order
		uint32_t edr = __REV(aextra);
		if (1 == extracnt)
		{
			extradata[0] = (aextra & 0xFF);
		}
		else if (2 == extracnt)
		{
			extradata[0] = ((aextra >> 8) & 0xFF);
			extradata[1] = ((aextra >> 0) & 0xFF);
		}
		else if (3 == extracnt)
		{
			extradata[0] = ((aextra >> 16) & 0xFF);
			extradata[1] = ((aextra >>  8) & 0xFF);
			extradata[2] = ((aextra >>  0) & 0xFF);
		}
	}
	extraremaining = extracnt;

	//extraremaining = 0;

	dmaused = rxdma.initialized;
	if (dmaused)
	{
		rxdma.Prepare(false, (void *)&(regs->DATA.reg), 0);
	}

	// clear all interrupt flags
	regs->INTFLAG.reg = 0
		| (1 << 7)  // ERROR
		| (1 << 1)  // SB
		| (1 << 0)  // MB
	;

	runstate = 0;
	busy = true;  // start the state machine

	Run();

	return ERROR_OK;
}

int THwI2c_atsam_v2::StartWriteData(uint8_t adaddr, unsigned aextra, void * srcptr, unsigned len)
{
	if (busy)
	{
		return ERROR_BUSY;
	}

	return ERROR_NOTIMPL;
}


void THwI2c_atsam_v2::Run()
{
	if (!busy)
	{
		return;
	}

#if 1
	return;
#else


	uint8_t  firstbyte;
	unsigned cr2;
	unsigned isr = regs->STATUS.reg;
	unsigned nbytes;
	uint8_t  busstate = ((isr >> 4) & 3);
	uint8_t  intflags = regs->INTFLAG.reg;
	uint8_t  isread = 0;

	// check error flags
	if (!error)
	{
		if (isr & SERCOM_I2CM_STATUS_RXNACK)
		{
			error = ERR_I2C_ACK;
		}
		else if (isr & SERCOM_I2CM_STATUS_ARBLOST)
		{
			error = ERR_I2C_ARBLOST;
		}
		else if (isr & SERCOM_I2CM_STATUS_BUSERR)
		{
			error = ERR_I2C_BUS;
		}
		else if (isr & SERCOM_I2CM_STATUS_LENERR)
		{
			error = ERR_I2C_ACK;
		}
		else if (intflags & 0x80)
		{
			error = ERR_I2C_BUS; // any error
		}

		if (error)
		{
			// jump to error handling
			runstate = 90;
		}
	}

	switch (runstate)
	{
	case 0:  // wait until ready to start
		if (busstate == 3) 	// wait while busy
		{
			return;
		}

		isread = (istx ? 0 : 1);

		if (extraremaining > 0)
		{
			// start with sending the extra data, no autoend, no reload
			waitreload = true;
			runstate = 5;
			isread = 0;
		}
		else if (istx)
		{
			nbytes = remainingbytes;
			if (nbytes > 255)
			{
				nbytes = 255;
			}
			runstate = 10;
		}
		else // rx
		{
			nbytes = remainingbytes;
			if (nbytes > 255)
			{
				nbytes = 255;
			}
			runstate = 20;
		}

		// writing the address register starts the transaction

		regs->ADDR.reg = 0
			| (0 << 16)       // LEN(8):
			| (0 << 15)       // TENBITEN:
			| (0 << 14)       // HS:
			| (0 << 13)       // LENEN
			| (0 <<  8)       // ADDREXT(3)
			| (devaddr << 1)  // ADDR(7)
			| (isread  <<  0)       // RD_WRN
		;

		break;

	case 5: // send extra bytes
		if (intflags & 1)  // ready to send?
		{
			regs->DATA.reg = extradata[extracnt - extraremaining];
			--extraremaining;
			if (extraremaining == 0)
			{
				if (istx)
				{
					runstate = 10;  Run();  return;
				}
				else
				{
					runstate = 6;  Run();  return;
				}
			}
		}
		break;

	case 6:  // send re-start
		if (intflags & 1)  // ready to send?
		{
			nbytes = remainingbytes;
			if (nbytes > 255)  nbytes = 255;

			regs->ADDR.reg = 0
				| (nbytes << 16)  // LEN(8):
				| (0 << 15)       // TENBITEN:
				| (0 << 14)       // HS:
				| (1 << 13)       // LENEN
				| (0 <<  8)       // ADDREXT(3)
				| (devaddr << 1)  // ADDR(7)
				| (isread  << 0)  // RD_WRN
			;

			runstate = 20;
		}
		break;

	// todo: implement send
	// case 10: ....

	case 20: //	receiving bytes
		if (isr & I2C_ISR_TCR) // reload required?
		{
			cr2 = regs->CR2 & 0x3FF; // keep the slave address
			nbytes = remainingbytes;
			if (nbytes > 255)
			{
				nbytes = 255;
				cr2 |= I2C_CR2_RELOAD;
			}
			cr2 |= I2C_CR2_AUTOEND;
			cr2 |= (nbytes << 16); // I2C_CR2_START;
			regs->CR2 = cr2;
			waitreload = false;
		}

		if (dmaused)
		{
			if (rxdma.Active())
			{
				return; // wait until DMA finishes
			}

			if (remainingbytes > 0)
			{
				if (waitreload)
				{
					return;
				}

				// (re-)start DMA

				xfer.dstaddr = dataptr;
				xfer.bytewidth = 1;
				cr2 = regs->CR2;
				xfer.count = ((cr2 >> 16) & 0xFF); // todo: check count zero
				xfer.flags = 0;
				dataptr += xfer.count;
				remainingbytes -= xfer.count;

				rxdma.StartTransfer(&xfer);

				waitreload = (remainingbytes > 0);
			}
			else
			{
				runstate = 29; // finish
			}
			return;
		}

		// working without DMA

		if ((isr & I2C_ISR_RXNE) == 0)  // RX Ready?
		{
			return;
		}

		*dataptr++ = regs->RXDR;
		--remainingbytes;

		if (remainingbytes > 0)
		{
			return;
		}
		runstate = 29; // terminate
		break;

	case 29: // wait last transfer to finish and send stop
		if (isr & I2C_ISR_STOPF)
		{
			runstate = 30;
			return;
		}

		if ((isr & I2C_ISR_TC) == 0)
		{
			return;
		}

		if ((regs->CR2 & I2C_CR2_AUTOEND) == 0)
		{
			regs->CR2 |= I2C_CR2_STOP;  // send stop
		}
		runstate = 30; // closing
		break;

	case 30: // closing
		if ((isr & I2C_ISR_STOPF) == 0)
		{
			return;
		}

		regs->ICR = I2C_ICR_STOPCF;

		busy = false; // finished.
		runstate = 50;
	  break;

	case 50: // finished
		break;

	case 90: // handling errors
		regs->CR2 |= I2C_CR2_STOP;  // send stop condition
		runstate = 91;
		break;

	case 91:
		// todo: reset on timeout
		if (isr & I2C_ISR_BUSY)   // wait until end of the transfer
		{
			return;
		}
		busy = false; // finished.
		runstate = 50;
		break;

	} // case

#endif
}
