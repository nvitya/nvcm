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
 *  file:     hwi2cslave_atsam.cpp
 *  brief:    ATSAM I2C / TWI Slave
 *  version:  1.00
 *  date:     2020-03-06
 *  authors:  nvitya
*/

#include "platform.h"
#include "hwpins.h"
#include "hwi2cslave.h"

#include "atsam_utils.h"

#include "traces.h"

#define I2C_EV_TXCOMP  (1 <<  0)
#define I2C_EV_RXRDY   (1 <<  1)
#define I2C_EV_TXRDY   (1 <<  2)
#define I2C_EV_SVACC   (1 <<  4)
#define I2C_EV_OVRE    (1 <<  6)
#define I2C_EV_NACK    (1 <<  8)
#define I2C_EV_EOSACC  (1 << 11)

#define I2C_SR_SVREAD  (1 <<  3)

bool THwI2cSlave_atsam::InitHw(int adevnum)
{
	unsigned tmp;
	unsigned perid;

	initialized = false;

	devnum = adevnum;

	regs = nullptr;

	// because of the peripheral IDs we need full multiple definitions
	if (false)
	{

	}
#ifdef TWI0
	else if (0 == devnum)
	{
		regs = (HW_I2CSL_REGS *)TWI0;
		perid = ID_TWI0;
	}
#endif
#ifdef TWIHS0
	else if (0 == devnum)
	{
		regs = (HW_I2CSL_REGS *)TWIHS0;
		perid = ID_TWIHS0;
	}
#endif
#ifdef TWI1
	else if (1 == devnum)
	{
		regs = (HW_I2CSL_REGS *)TWI1;
		perid = ID_TWI1;
	}
#endif
#ifdef TWIHS1
	else if (1 == devnum)
	{
		regs = (HW_I2CSL_REGS *)TWIHS1;
		perid = ID_TWIHS1;
	}
#endif
#ifdef TWI2
	else if (2 == devnum)
	{
		regs = (HW_I2CSL_REGS *)TWI2;
		perid = ID_TWI2;
	}
#endif
#ifdef TWIHS2
	else if (2 == devnum)
	{
		regs = (HW_I2CSL_REGS *)TWIHS2;
		perid = ID_TWIHS2;
	}
#endif
	if (!regs)
	{
		return false;
	}

	// Enable the peripheral
	if (perid < 32)
	{
		PMC->PMC_PCER0 = (1 << perid);
	}
	else
	{
		PMC->PMC_PCER1 = (1 << (perid-32));
	}

	regs->SMR = 0
		| (address << 16)
	;

	regs->CR = 0
		| (0  <<  0)  // START
		| (0  <<  1)  // STOP
		| (0  <<  2)  // MSEN
		| (1  <<  3)  // MSDIS
		| (1  <<  4)  // SVEN
		| (0  <<  5)  // SVDIS
		| (0  <<  6)  // QUICK
		| (0  <<  7)  // SWRST
	;

	// The enabled interrupts will be changed continously
	regs->IDR = 0xFFFF;
	regs->IER = I2C_EV_SVACC | I2C_EV_RXRDY;  // RX event will be always enabled

	runstate = 0;

	initialized = true;

	return true;
}

// runstate:
//   0: idle
//   1: receive data
//   5: transmit data


void THwI2cSlave_atsam::HandleIrq()
{
	uint32_t sr = regs->SR;

	//TRACE("[I2C IRQ %04X, %04X]\r\n", sr, regs->IMR);

	if (0 == runstate)  // wait for addressing
	{
		if (sr & I2C_EV_SVACC)  // Start of slave access
		{
			if (sr & I2C_SR_SVREAD)
			{
				istx = true;
				runstate = 5;  // go to transfer data
				regs->IER = I2C_EV_TXRDY;
			}
			else
			{
				istx = false;
				runstate = 1;  // go to receive data
			}

			OnAddressRw(address); // there is no other info on this chip, use own address

			regs->IDR = I2C_EV_SVACC;
			regs->IER = I2C_EV_EOSACC | I2C_EV_TXCOMP;
		}
		else
		{
			// unhandled IRQ event
		}
	}
	else
	{
		if (sr & (I2C_EV_EOSACC | I2C_EV_TXCOMP)) // EOSACC: End of slave access, TXCOMP: Transmission Completed
		{
			regs->IER = I2C_EV_SVACC;
			regs->IDR = I2C_EV_EOSACC | I2C_EV_TXCOMP | I2C_EV_TXRDY;
			runstate = 0;
		}
	}

	if (sr & I2C_EV_TXRDY) // TXRDY: Transmit Holding Register Ready
	{
		if (0 == (sr & I2C_EV_NACK))  // only when NACK is not set !
		{
			if (5 == runstate)
			{
				uint8_t d = OnTransmitRequest();
				regs->THR = d;
			}
			else
			{
				// do not send anything
				regs->IDR = I2C_EV_TXRDY;
			}
		}
		else
		{
			regs->IDR = I2C_EV_TXRDY;
		}
	}

	if (sr & I2C_EV_RXRDY) // RXRDY: Receive Holding Register Ready
	{
		uint8_t d = regs->RHR;
		if (1 == runstate)
		{
			OnByteReceived(d);
		}
		else
		{
			// unexpected byte
		}
	}
}

