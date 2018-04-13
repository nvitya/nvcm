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
 *  file:     hwi2c_stm32.cpp
 *  brief:    STM32 I2C / TWI
 *  version:  1.00
 *  date:     2018-02-10
 *  authors:  nvitya
*/

#include "platform.h"
#include "hwpins.h"
#include "hwi2c.h"

#include "traces.h"

#if HW_VER == 2

bool THwI2c_stm32::Init(int adevnum)
{
	unsigned tmp;
	unsigned clockdiv = 2;

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
	}
#endif
#ifdef I2C2
	else if (2 == devnum)
	{
		regs = I2C2;
		RCC->APB1ENR |= RCC_APB1ENR_I2C2EN;
	}
#endif
#ifdef I2C3
	else if (3 == devnum)
	{
		regs = I2C3;
		RCC->APB1ENR |= RCC_APB1ENR_I2C3EN;
	}
#endif
#ifdef I2C4
	else if (4 == devnum)
	{
		regs = I2C4;
		RCC->APB1ENR |= RCC_APB1ENR_I2C4EN;
	}
#endif
	if (!regs)
	{
		return false;
	}

	// disable for setup
	regs->CR1 &= ~I2C_CR1_PE;  // resets the I2C internal logic too
	while (regs->CR1 & I2C_CR1_PE)
	{
		// wait until flag is cleared
	}

	unsigned cr1 = 0
		| (0 << 20)      // SMBBUS BITS(4)
		| (0 << 19)      // GCEN: general call enable
		| (0 << 17)      // NOSTRECH
		| (0 << 16)      // SBC: slave byte control
		| (0 << 15)      // RXDMAEN
		| (0 << 14)      // TXDMAEN
		| (0 << 12)      // ANFOFF: 0 = enabled
		| (0 <<  8)      // DNF(3): digital nois filter
		| (0 <<  1)      // IE(7): interrupt enable bits
		| (0 <<  0)      // PE: 1 = enable
	;
	regs->CR1 = cr1;

	if (SystemCoreClock <= 48000000)
	{
		clockdiv = 1;
	}
	else if (SystemCoreClock > 72000000)
	{
		clockdiv = (clockdiv << 1);
	}

	unsigned periphclock = SystemCoreClock / clockdiv;

	// TIMING

	regs->TIMEOUTR = 0; // disable timeout detection

	unsigned halfclockdiv = (periphclock / (speed * 2));
	unsigned presc = 0;
	while (halfclockdiv > 255)
	{
		++presc;
		halfclockdiv = (periphclock / ((1 + presc) * (speed * 2)));
	}

	regs->TIMINGR = 0
		| (presc << 28)  // PRESC: timing prescaler
		| (1 << 20)  // SCLDEL(4): Data setup time
		| (1 << 16)  // SDADEL(4): Data hold time
		| ((halfclockdiv) << 8) // SCLL(8): low period clocks
		| ((halfclockdiv) << 0) // SCLH(8): high period clocks
	;

	cr1 |= 1;  // enable I2C
	regs->CR1 |= cr1;

	initialized = true;

	return true;
}

int THwI2c_stm32::StartReadData(uint8_t adaddr, unsigned aextra, void * dstptr, unsigned len)
{
	unsigned cr2;

	if (busy)
	{
		return ERROR_BUSY;
	}

	istx = false;
	devaddr = adaddr;
	dataptr = (uint8_t *)dstptr;
	datalen = len;
	remainingbytes = datalen;

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
		rxdma.Prepare(false, (void *)&(regs->RXDR), 0);
		regs->CR1 |= I2C_CR1_RXDMAEN;
	}
	else
	{
		regs->CR1 &= ~I2C_CR1_RXDMAEN;
	}

	cr2 = 0
		| (0 << 26)     // PECBYTE
		| (0 << 25)     // AUTOEND
		| (0 << 24)     // RELOAD
		| (0 << 16)     // NBYTES(7)
		| (0 << 15)     // NACK
		| (0 << 14)     // STOP
		| (0 << 13)     // START
		| (0 << 12)     // HEAD10R
		| (0 << 11)     // ADD10
		| (0 << 10)     // RD_WRN
		| (devaddr << 1)   // SADD(10), bits 7..1 are used for 7-bit addressing!
	;
	regs->CR2 = cr2;

	runstate = 0;
	busy = true;  // start the state machine

	Run();

	return ERROR_OK;
}

int THwI2c_stm32::StartWriteData(uint8_t adaddr, unsigned aextra, void * srcptr, unsigned len)
{
	unsigned cr2;

	if (busy)
	{
		return ERROR_BUSY;
	}

	istx = true;
	devaddr = adaddr;
	dataptr = (uint8_t *)srcptr;
	datalen = len;
	remainingbytes = datalen;

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

	dmaused = txdma.initialized;
	if (dmaused)
	{
		txdma.Prepare(true, (void *)&(regs->TXDR), 0);
		regs->CR1 |= I2C_CR1_TXDMAEN;
	}
	else
	{
		regs->CR1 &= ~I2C_CR1_TXDMAEN;
	}

	cr2 = 0
		| (0 << 26)     // PECBYTE
		| (0 << 25)     // AUTOEND
		| (0 << 24)     // RELOAD
		| (0 << 16)     // NBYTES(7)
		| (0 << 15)     // NACK
		| (0 << 14)     // STOP
		| (0 << 13)     // START
		| (0 << 12)     // HEAD10R
		| (0 << 11)     // ADD10
		| (0 << 10)     // RD_WRN
		| (devaddr << 1)   // SADD(10), bits 7..1 are used for 7-bit addressing!
	;
	regs->CR2 = cr2;

	runstate = 0;
	busy = true;  // start the state machine

	Run();

	return ERROR_OK;
}


void THwI2c_stm32::Run()
{
	if (!busy)
	{
		return;
	}

	uint8_t  firstbyte;
	unsigned cr2;
	unsigned isr = regs->ISR;
	unsigned nbytes;

	switch (runstate)
	{
	case 0:  // wait until busy to start
		// wait until busy
		if (isr & I2C_ISR_BUSY)
		{
			return;
		}

		cr2 = regs->CR2 & 0x3FF; // keep the slave address

		if (extraremaining > 0)
		{
			// start with sending the extra data, no autoend, no reload
			nbytes = extraremaining;
			if (istx)
			{
				nbytes += remainingbytes;
				if (nbytes > 255)
				{
					nbytes = 255;
					cr2 |= I2C_CR2_RELOAD;
				}
				else
				{
					cr2 |= I2C_CR2_AUTOEND;
				}
			}
			runstate = 5;
		}
		else if (istx)
		{
			nbytes = remainingbytes;
			if (nbytes > 255)
			{
				nbytes = 255;
				cr2 |= I2C_CR2_RELOAD;
			}
			else
			{
				cr2 |= I2C_CR2_AUTOEND;
			}
			runstate = 10;
		}
		else // rx
		{
			cr2 |= I2C_CR2_RD_WRN;
			nbytes = remainingbytes;
			if (nbytes > 255)
			{
				nbytes = 255;
				cr2 |= I2C_CR2_RELOAD;
			}
			else
			{
				cr2 |= I2C_CR2_AUTOEND;
			}
			runstate = 20;
		}

		cr2 |= (nbytes << 16) | I2C_CR2_START;  // start the transmission
		regs->CR2 = cr2;
		break;

	case 5: // send extra bytes
		if (isr & I2C_ISR_TXE)
		{
			regs->TXDR = extradata[extracnt - extraremaining];
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
		if (isr & I2C_ISR_TC)
		{
			cr2 = regs->CR2 & 0x3FF; // keep the slave address

			nbytes = remainingbytes;
			if (nbytes > 255)
			{
				nbytes = 255;
				cr2 |= I2C_CR2_RELOAD;
			}
			else
			{
				cr2 |= I2C_CR2_AUTOEND;
			}

			cr2 |= (nbytes << 16) | I2C_CR2_START | I2C_CR2_RD_WRN;
			regs->CR2 = cr2;
			runstate = 20;
		}
		break;

	case 10: // sending bytes
		if (dmaused && txdma.Active())
		{
			return; // wait until DMA finishes
		}

		if (remainingbytes > 0)
		{
			if (isr & I2C_ISR_TCR) // reload required?
			{
				cr2 = regs->CR2 & 0x3FF; // keep the slave address
				nbytes = remainingbytes;
				if (nbytes > 255)
				{
					nbytes = 255;
					cr2 |= I2C_CR2_RELOAD;
				}
				else
				{
					cr2 |= I2C_CR2_AUTOEND;
				}
				cr2 |= (nbytes << 16); // I2C_CR2_START;
				regs->CR2 = cr2;
			}

			if (dmaused)
			{
				if (!txdma.Active())
				{
					if ((isr & I2C_ISR_TXE) == 0)  // there were stop problems when this was not here ... if someone nows better, please tell
					{
						return;
					}

					// (re-)start DMA

					xfer.srcaddr = dataptr;
					xfer.bytewidth = 1;
					cr2 = regs->CR2;
					xfer.count = ((cr2 >> 16) & 0xFF) - extracnt; // todo: check count zero
					extracnt = 0;
					xfer.addrinc = true;
					dataptr += xfer.count;
					remainingbytes -= xfer.count;

					txdma.StartTransfer(&xfer);
				}
				return;
			}

			// manual sending

			if ((isr & I2C_ISR_TXE) == 0)  // TX Ready?
			{
				return;
			}

			regs->TXDR = *dataptr++;
			--remainingbytes;

			if (remainingbytes > 0)
			{
				return;
			}
		}
		runstate = 29; // finish
		break;

	case 20: //	receiving bytes
		if (dmaused && rxdma.Active())
		{
			return; // wait until DMA finishes
		}

		if (remainingbytes > 0)
		{
			if (isr & I2C_ISR_TCR) // reload required?
			{
				cr2 = regs->CR2 & 0x3FF; // keep the slave address
				nbytes = remainingbytes;
				if (nbytes > 255)
				{
					nbytes = 255;
					cr2 |= I2C_CR2_RELOAD;
				}
				else
				{
					cr2 |= I2C_CR2_AUTOEND;
				}
				cr2 |= (nbytes << 16); // I2C_CR2_START;
				regs->CR2 = cr2;
			}

			if (dmaused)
			{
				if (!rxdma.Active())
				{
					// (re-)start DMA
					xfer.dstaddr = dataptr;
					xfer.bytewidth = 1;
					cr2 = regs->CR2;
					xfer.count = ((cr2 >> 16) & 0xFF); // todo: check count zero
					xfer.addrinc = true;
					dataptr += xfer.count;
					remainingbytes -= xfer.count;

					rxdma.StartTransfer(&xfer);
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

	} // case
}

#endif
