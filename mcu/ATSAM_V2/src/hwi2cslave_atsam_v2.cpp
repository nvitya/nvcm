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
 *  file:     hwi2cslave_atsam_v2.cpp
 *  brief:    ATSAM v2 I2C / TWI Slave
 *  version:  1.00
 *  date:     2020-03-06
 *  authors:  nvitya
*/

#include "platform.h"
#include "hwpins.h"
#include "hwi2cslave.h"

#include "atsam_v2_utils.h"

#include "traces.h"

#define I2C_EV_PREC    (1 <<  0) // STOP received
#define I2C_EV_AMATCH  (1 <<  1)
#define I2C_EV_DRDY    (1 <<  2)
#define I2C_EV_ERROR   (1 <<  7)

bool THwI2cSlave_atsam_v2::InitHw(int adevnum)
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

	regs = (SercomI2cs *)sercom_inst_list[devnum];

	regs->CTRLA.bit.ENABLE = 0; // disable
	while (regs->SYNCBUSY.bit.ENABLE) { } // wait for disable

	// CTRLA
	regs->CTRLA.reg = 0
		| (0 << 30)  // LOWTOUT: 0 = disabled
		| (0 << 27)  // SCLSM: 1 = SCL stretch only after ACK bit
		| (0 << 24)  // SPEED(2): 0 = standard (max 400 kHz), 1 = fast mode plus (max 1 MHz), 2 = High-speed (max 3.4 MHz)
		| (0 << 23)  // SEXTTOEN:
		| (0 << 20)  // SDAHOLD(2):
		| (0 << 16)  // PINOUT: 0 = 4 wire disabled
		| (1 <<  7)  // RUNSTDBY: 1 = enable in standby
		| (4 <<  2)  // MODE(3): 4 = I2C Slave
		| (0 <<  1)  // ENABLE: (do not enable yet)
		| (0 <<  0)  // SWRST: 1 = software reset
	;

#ifdef MCUSF_E5X
	regs->CTRLC.reg = 0;  // disable 32 bit mode
#endif

	regs->CTRLB.reg = 0
		| (0 << 18)  // ACKACT: 0 = send ACK after a byte received, 1 = send NACK
		| (0 << 16)  // CMD(2): 0 = no action
		| (0 << 14)  // AMODE(2): Address Mode, 0 = mask
		| (0 << 10)  // AACKEN: 1 = Enable Automatic Acknowledge (of the Address)
		| (0 <<  9)  // GCMD:
		| (1 <<  8)  // SMEN: 1 = send ACK automatically when data is read
	;

	regs->ADDR.reg = 0
		| (0       << 17)  // ADDRMASK(10): 0 = full address match
		| (0       << 15)  // TENBITEN: 0 = 7 bit address mode
		| (address <<  1)  // ADDR(10)
		| (0       <<  0)  // GENCEN
	;

	regs->LENGTH.reg = 0;

	// force IDLE state, clear clearable flags

	regs->STATUS.reg = 0
		| (1 << 10)  // HS
		| (1 <<  9)  // SEXTTOUT
		| (1 <<  6)  // LOWTOUT
		| (0 <<  2)  // RXNACK
		| (1 <<  1)  // COLL
		| (1 <<  0)  // BUSERR
	;

	regs->INTENSET.reg = 0
		| (1 <<  0)  // PREC
		| (1 <<  1)  // AMATCH
		| (1 <<  2)  // DRDY
		| (1 <<  7)  // ERROR
	;

	regs->CTRLA.bit.ENABLE = 1; // enable
	while (regs->SYNCBUSY.bit.ENABLE) { } // wait for enable

	initialized = true;

	return true;
}

// runstate:
//   0: idle
//   1: receive data
//   5: transmit data

void THwI2cSlave_atsam_v2::HandleIrq()
{
	uint8_t intflag = regs->INTFLAG.reg;

	//TRACE("[I2C IRQ %02X]\r\n", intflag);

	if (intflag & I2C_EV_AMATCH)
	{
		if (regs->STATUS.bit.DIR)
		{
			istx = true;
			runstate = 5;  // go to transfer data
		}
		else
		{
			istx = false;
			runstate = 1;  // go to receive data
		}

		OnAddressRw(address); // there is no other info on this chip, use own address

		//regs->INTFLAG.reg = I2C_EV_AMATCH;
		regs->CTRLB.bit.CMD = 3;
	}

	if (intflag & I2C_EV_DRDY)
	{
		if (1 == runstate)
		{
			uint8_t d = *(uint8_t *)&regs->DATA.reg;
			OnByteReceived(d);
		}
		else if (5 == runstate)
		{
			uint8_t d = OnTransmitRequest();
			*(uint8_t *)&regs->DATA.reg = d;
		}

		//regs->INTFLAG.reg = I2C_EV_DRDY;
		regs->CTRLB.bit.CMD = 3;
	}

	if (intflag & I2C_EV_PREC)
	{
		runstate = 0;
		regs->INTFLAG.reg = I2C_EV_PREC;
	}

	if (intflag & I2C_EV_ERROR)
	{
		regs->INTFLAG.reg = I2C_EV_ERROR;
	}
}

