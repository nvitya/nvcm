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
 *  file:     hwi2cslave_stm32_v2.cpp
 *  brief:    STM32 I2C / TWI Slave for F0, F3, F7 etc
 *  version:  1.00
 *  date:     2020-06-07
 *  authors:  nvitya
*/

#include "platform.h"
#include "hwpins.h"
#include "hwi2cslave.h"

#include "stm32_utils.h"

#include "traces.h"

#if I2C_HW_VER != 1

#ifdef RCC_APB1ENR1_I2C1EN  // G4
  #define RCC_APB1ENR_I2C1EN     RCC_APB1ENR1_I2C1EN
  #define RCC_APB1ENR_I2C2EN     RCC_APB1ENR1_I2C2EN
  #define RCC_APB1ENR_I2C3EN     RCC_APB1ENR1_I2C3EN
#elif defined(RCC_APB1LENR_I2C1EN) // H7
  #define RCC_APB1ENR_I2C1EN     RCC_APB1LENR_I2C1EN
  #define RCC_APB1ENR_I2C2EN     RCC_APB1LENR_I2C2EN
  #define RCC_APB1ENR_I2C3EN     RCC_APB1LENR_I2C3EN
#endif

// v2: F0, F3, F7 etc.

bool THwI2cSlave_stm32::InitHw(int adevnum)
{
	unsigned tmp;
	uint8_t busid = STM32_BUSID_APB1;

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
		APB1ENR_REGISTER |= RCC_APB1ENR_I2C1EN;

		#ifdef RCC_CFGR3_I2C1SW
			RCC->CFGR3 |= RCC_CFGR3_I2C1SW; // select system clock for the source instead of the HSI
		#endif
	}
#endif
#ifdef I2C2
	else if (2 == devnum)
	{
		regs = I2C2;
		APB1ENR_REGISTER |= RCC_APB1ENR_I2C2EN;
	}
#endif
#ifdef I2C3
	else if (3 == devnum)
	{
		regs = I2C3;
		APB1ENR_REGISTER |= RCC_APB1ENR_I2C3EN;
	}
#endif
#ifdef I2C4
	else if (4 == devnum)
	{
		regs = I2C4;
		#ifdef RCC_APB1ENR2_I2C4EN
			RCC->APB1ENR2 |= RCC_APB1ENR2_I2C4EN;
		#elif defined RCC_APB4ENR_I2C4EN
			RCC->APB4ENR |= RCC_APB4ENR_I2C4EN;
		#else
			RCC->APB1ENR |= RCC_APB1ENR_I2C4EN;
		#endif
	}
#endif
	if (!regs)
	{
		return false;
	}

	// disable for setup
	regs->CR1 &= ~I2C_CR1_PE;

	regs->CR1 = 0
		| (0  << 23)  // PECEN
		| (0  << 22)  // ALERTEN
		| (0  << 21)  // SMBDEN
		| (0  << 20)  // SMBHEN
		| (0  << 19)  // GCEN: 0 = disable general call address
		| (0  << 17)  // NOSTRECH: 0 = clock stretching enabled
		| (1  << 16)  // SBC: 1 = slave byte control
		| (0  << 15)  // RXDMAEN
		| (0  << 14)  // TXDMAEN
		| (0  << 12)  // ANFOFF: 0 = analogue noise filter on
		| (0  <<  8)  // DNF(4): 0 = digital noise filter off
		| (1  <<  7)  // ERRIE
		| (1  <<  6)  // TCIE
		| (1  <<  5)  // STOPIE
		| (1  <<  4)  // NACKIE
		| (1  <<  3)  // ADDRIE
		| (1  <<  2)  // RXIE
		| (1  <<  1)  // TXIE
		| (1  <<  0)  // PE
	;

	// setup the address

	regs->OAR1 = 0; // disable the OAR1, only the OAR2 will be used
	regs->OAR2 = 0; // disable first

	regs->CR2 = 0
    | (1  << 24)  // RELOAD: 1 = reload event after each byte
		| (1  << 16)  // NBYTES(8): 1 = reload event instead auto stop
	;

	regs->TIMEOUTR = 0; // disable timeouts
	regs->TIMINGR = 0x00B00000;

	uint8_t masklist[8] = {0x00, 0x40, 0x60, 0x70, 0x78, 0x7C, 0x7E, 0x7F};
	uint8_t maskid = 0;
	addressmask = (addressmask & 0x7F);
	while ((maskid < 8) && (addressmask != masklist[maskid]))  ++maskid;
	if (maskid > 7)  maskid = 0;

	regs->OAR2 = 0
		| ((address & 0x7F) << 1)
		| (maskid << 8)
	;
	regs->OAR2 |= (1 << 15);

	initialized = true;

	return true;
}

void THwI2cSlave_stm32::HandleIrq()
{
	uint32_t isr = regs->ISR;

	// warning, the sequence above clears some of the status bits

	//TRACE("[I2C IRQ %08X]\r\n", isr);

	// check errors
	if (isr & I2C_ISR_NACKF)
	{
		// this is normal
		regs->ICR = I2C_ISR_NACKF;
		//isr &= ~I2C_ISR_NACKF;
	}

	// other errors
	uint32_t oerrmsk = (I2C_ISR_TIMEOUT | I2C_ISR_OVR | I2C_ISR_BERR);
	if (isr & oerrmsk)
	{
		regs->ICR = oerrmsk;
		//isr &= ~oerrmsk;
	}

	// check events

	if (isr & I2C_ISR_ADDR) // address matched, after start / restart
  {
		if (isr & I2C_ISR_DIR)
		{
			istx = true;
			runstate = 5;  // go to transfer data
		}
		else
		{
			istx = false;
			runstate = 1;  // go to receive data
		}

		regs->ICR = I2C_ISR_ADDR;

		OnAddressRw((isr >> I2C_ISR_ADDCODE_Pos) & 0x7F);
	}


	if (isr & I2C_ISR_RXNE)
	{
		uint8_t d = regs->RXDR;
		if (1 == runstate)
		{
			OnByteReceived(d);
		}
		else
		{
			// unexpected byte
		}
	}

	if (isr & I2C_ISR_TXIS)
	{
		if (5 == runstate)
		{
			uint8_t d = OnTransmitRequest();
			regs->TXDR = d;
			//TRACE("<< %02X\r\n", d);
		}
		else
		{
			// unexpected TX request !
			regs->TXDR = 0xFF;
			regs->CR2 = 0; // no reload after this
			isr &= ~I2C_ISR_TCR; // disable reload handling
		}
	}

	if (isr & I2C_ISR_TCR)
	{
		// transfer complete, reload
		regs->CR2 = 0
	    | (1  << 24)  // RELOAD: 1 = reload event after each byte
			| (1  << 16)  // NBYTES(8): 1 = reload event instead auto stop
		;
	}

	// check stop
	if (isr & I2C_ISR_STOPF)
	{
		//TRACE(" STOP DETECTED.\r\n");
		regs->ICR = I2C_ISR_STOPF;
	}
}

#endif
