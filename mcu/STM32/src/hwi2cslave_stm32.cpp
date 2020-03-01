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
 *  file:     hwi2cslave_stm32.cpp
 *  brief:    STM32 I2C / TWI Slave
 *  version:  1.00
 *  date:     2019-10-13
 *  authors:  nvitya
*/

#include "platform.h"
#include "hwpins.h"
#include "hwi2cslave.h"

#include "traces.h"

#if I2C_HW_VER == 1

bool THwI2cSlave_stm32::InitHw(int adevnum)
{
	unsigned tmp;
	unsigned clockdiv = 1;

	initialized = false;

	devnum = adevnum;

	regs = nullptr;

	if (false)
	{
	}
#ifdef I2C1
	else if (1 == devnum)
	{
		regs = I2C1;
		RCC->APB1ENR |= RCC_APB1ENR_I2C1EN;
		clockdiv = 2;
	}
#endif
#ifdef I2C2
	else if (2 == devnum)
	{
		regs = I2C2;
		RCC->APB1ENR |= RCC_APB1ENR_I2C2EN;
		clockdiv = 2;
	}
#endif
	if (!regs)
	{
		return false;
	}

	// disable for setup
	regs->CR1 &= ~I2C_CR1_PE;

	regs->CR1 |= I2C_CR1_SWRST;
	regs->CR1 &= ~I2C_CR1_SWRST;

	unsigned cr1 = 0;  // use the default settings, keep disabled

	regs->CR1 = cr1;

#ifdef MCUSF_F3
	clockdiv = (clockdiv << 1);
#else
	if (SystemCoreClock <= 48000000)
	{
		clockdiv = 1;
	}
	else if (SystemCoreClock > 72000000)
	{
		clockdiv = (clockdiv << 1);
	}
#endif

	// setup address
	// this device does not support address mask

	regs->OAR1 = ((address & 0x7F) << 1);

	unsigned periphclock = SystemCoreClock / clockdiv;

	// CR2
	tmp = 0
		| (0 << 12)  // LAST: 1 = Last transfer on DMA EOT
		| (0 << 11)  // DMAEN: 1 = enable DMA
		| (1 << 10)  // ITBUFEN: 1 = enable data interrupt
		| (1 <<  9)  // ITEVTEN: 1 = enable event interrupt
		| (1 <<  8)  // ITERREN: 1 = enable error interrupt
		| ((periphclock / 1000000) << 0)  // set clock speed in MHz
	;
	regs->CR2 = tmp;

	cr1 |= 0
		| I2C_CR1_ACK   // ACKnowledge must be enabled, otherwise even the own address won't be handled
	  | I2C_CR1_PE    // Enable
	;

	regs->CR1 = cr1;

	initialized = true;

	return true;
}

// runstate:
//   0: idle
//   1: receive data
//   5: transmit data

void THwI2cSlave_stm32::HandleIrq()
{
	uint32_t sr1 = regs->SR1;
	uint32_t sr2 = regs->SR2;

	// warning, the sequence above clears some of the status bits

	//TRACE("[I2C IRQ %04X %04X]\r\n", sr1, sr2);

	// check errors
	if (sr1 & 0xFF00)
	{
		if (sr1 & I2C_SR1_AF)  // ACK Failure ?
		{
			// this is normal
			regs->SR1 = ~I2C_SR1_AF; // clear AF error
		}
		else
		{
			//TRACE("I2C errors: %04X\r\n", sr1);
			regs->SR1 = ~(sr1 & 0xFF00); // clear errors
		}
	}

	// check events

	if (sr1 & I2C_SR1_ADDR) // address matched, after start / restart
  {
		if (sr2 & I2C_SR2_TRA)
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
	}

	if (sr1 & I2C_SR1_RXNE)
	{
		uint8_t d = regs->DR;
		if (1 == runstate)
		{
			OnByteReceived(d);
		}
		else
		{
			// unexpected byte
		}
	}

	if (sr1 & I2C_SR1_TXE)
	{
		if (5 == runstate)
		{
			uint8_t d = OnTransmitRequest();
			regs->DR = d;
		}
		else
		{
			// force stop, releases the data lines
			regs->CR1 |= I2C_CR1_STOP;  // this must be done here for the proper STOP
		}
	}

	// check stop
	if (sr1 & I2C_SR1_STOPF)
	{
		//TRACE(" STOP DETECTED.\r\n");

		// the CR1 must be written in order to clear this flag
		runstate = 0;
		uint32_t cr1 = regs->CR1;
		cr1 &= ~I2C_CR1_STOP;
		regs->CR1 = cr1;
	}

	if (sr2)  // to keep sr2 if unused
	{

	}
}

#endif
