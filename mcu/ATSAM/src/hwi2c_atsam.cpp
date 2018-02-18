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
 *  file:     hwi2c_atsam.cpp
 *  brief:    ATSAM I2C / TWI
 *  version:  1.00
 *  date:     2018-02-10
 *  authors:  nvitya
*/

#include "platform.h"
#include "hwpins.h"
#include "hwi2c.h"

bool THwI2c_atsam::Init(int adevnum)
{
	unsigned tmp;
	unsigned perid;

	initialized = false;

	devnum = adevnum;

	regs = nullptr;

	// because of the peripheral IDs we need full multiple definitions
	// advanced SPIs
	if (false)
	{

	}
#ifdef TWI0
	else if (0 == devnum)
	{
		regs = (HW_I2C_REGS *)TWI0;
		perid = ID_TWI0;
	}
#endif
#ifdef TWIHS0
	else if (0 == devnum)
	{
		regs = (HW_I2C_REGS *)TWIHS0;
		perid = ID_TWIHS0;
	}
#endif
#ifdef TWI1
	else if (1 == devnum)
	{
		regs = (HW_I2C_REGS *)TWI1;
		perid = ID_TWI1;
	}
#endif
#ifdef TWIHS1
	else if (1 == devnum)
	{
		regs = (HW_I2C_REGS *)TWIHS1;
		perid = ID_TWIHS1;
	}
#endif
#ifdef TWI2
	else if (2 == devnum)
	{
		regs = (HW_I2C_REGS *)TWI2;
		perid = ID_TWI2;
	}
#endif
#ifdef TWIHS2
	else if (2 == devnum)
	{
		regs = (HW_I2C_REGS *)TWIHS2;
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

	unsigned periphclock = SystemCoreClock;
	if (periphclock > 150000000)  periphclock = periphclock / 2;

	unsigned halfclockdiv = ((periphclock / speed) >> 1);
	unsigned ckdiv = 0;
	while (halfclockdiv > 255)
	{
		ckdiv += 1;
		halfclockdiv = (halfclockdiv >> 1);
	}

	regs->CWGR = 0
		| (0 << 24)  // HOLD(5)
		| (0 << 20)  // CKSRC: 0 = periph. clock
		| (ckdiv << 16)  // CKDIV(3): big prescaler
		| (halfclockdiv << 8)  // CHDIV
		| (halfclockdiv << 0)  // CLDIV
  ;

	regs->CR = (1 << 2);  // Master mode enable

	if (txdma.initialized)
	{
		txdma.Prepare(true,  (void *)(regs->THR), 0);
	}

	if (rxdma.initialized)
	{
		rxdma.Prepare(false, (void *)(regs->RHR), 0);
	}

	initialized = true;

	return true;
}

int THwI2c_atsam::StartReadData(uint8_t adaddr, unsigned aextra, void * dstptr, unsigned len)
{
	if (busy)
	{
		return ERROR_BUSY;
	}

	istx = false;
	dataptr = (uint8_t *)dstptr;
	datalen = len;
	remainingbytes = datalen;

	unsigned extracnt = ((aextra >> 24) & 3);
	if (extracnt)
	{
		regs->IADR = (aextra & 0x00FFFFFF);
	}

	regs->MMR = 0
		| (adaddr << 16)
		| (1 << 12)  // MREAD: 1 = read
		| (extracnt <<  8)  // IADRSZ: internal address bytes
	;

	// DMA is not implemented so far
	dmaused = false;

	// Start

	unsigned tmp = (1 << 0);  // send a start condition
	if (remainingbytes <= 1)
	{
		tmp |= (1 << 0); // send a stop condition too
	}
	regs->CR = tmp;

	runstate = 0;

	busy = true;

	return ERROR_OK;
}

int THwI2c_atsam::StartWriteData(uint8_t adaddr, unsigned aextra, void * srcptr, unsigned len)
{
	if (busy)
	{
		return ERROR_BUSY;
	}

	istx = true;
	dataptr = (uint8_t *)srcptr;
	datalen = len;
	remainingbytes = datalen;

	unsigned extracnt = ((aextra >> 24) & 3);
	if (extracnt)
	{
		regs->IADR = (aextra & 0x00FFFFFF);
	}

	regs->MMR = 0
		| (adaddr << 16)
		| (0 << 12)  // MREAD: 0 = write
		| (extracnt <<  8)  // IADRSZ: internal address bytes
	;

	// DMA is not implemented so far
	dmaused = false;

	// Start

	unsigned tmp = (1 << 0);  // send a start condition
	if (remainingbytes <= 1)
	{
		tmp |= (1 << 0); // send a stop condition too
	}
	regs->CR = tmp;

	runstate = 0;

	busy = true;

	return ERROR_OK;
}


void THwI2c_atsam::Run()
{
	if (!busy)
	{
		return;
	}

	if (istx)
	{
#if 0
		if (dmaused && txdma.Enabled())
		{
			return;
		}
#endif
		if (remainingbytes > 0)
		{
			if ((regs->SR & (1 << 2)) == 0)  // TX Ready?
			{
				return;
			}

			if (remainingbytes == 1)
			{
				regs->CR = (1 << 1);  // send stop condition after the following read
			}

			regs->THR = *dataptr++;
			--remainingbytes;

			if (remainingbytes > 0)
			{
				return;
			}
		}
	}
	else
	{
#if 0
		if (dmaused && rxdma.Enabled())
		{
			return;
		}
#endif

		if (remainingbytes > 0)
		{
			if ((regs->SR & (1 << 1)) == 0)  // RX Ready?
			{
				return;
			}

			if (remainingbytes == 2)
			{
				regs->CR = (1 << 1);  // send stop condition after the following read
			}

			*dataptr++ = regs->RHR;
			--remainingbytes;

			if (remainingbytes > 0)
			{
				return;
			}
		}
	}

	if ((regs->SR & 1) == 0)   // transfer completed?
	{
		return;
	}

	busy = false;
}

