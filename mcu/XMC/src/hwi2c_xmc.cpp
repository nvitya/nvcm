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
 *  file:     hwi2c_xmc.cpp
 *  brief:    XMC I2C
 *  version:  1.00
 *  date:     2019-03-23
 *  authors:  nvitya
*/

#include <stdio.h>
#include <stdarg.h>

#include "hwi2c.h"
#include "xmc_utils.h"

#define XMC_I2C_TDF_MASTER_SEND           0
#define XMC_I2C_TDF_SLAVE_SEND            1
#define XMC_I2C_TDF_MASTER_RECEIVE_ACK    2
#define XMC_I2C_TDF_MASTER_RECEIVE_NACK   3
#define XMC_I2C_TDF_MASTER_START          4
#define XMC_I2C_TDF_MASTER_RESTART        5
#define XMC_I2C_TDF_MASTER_STOP           6

bool THwI2c_xmc::Init(int ausicnum, int achnum, int ainputpin_scl, int ainputpin_sda)
{
	unsigned code;
	unsigned rv;
	unsigned clockdiv = 1;

	usicnum = ausicnum;
	chnum = achnum;
	inputpin_scl = ainputpin_scl;
	inputpin_sda = ainputpin_sda;
	devnum = achnum;

	initialized = false;

	regs = xmc_usic_ch_init(usicnum, chnum);
	if (!regs)
	{
		return false;
	}

	unsigned stim = 0;
	unsigned oversampling = 10;
	if (speed > 100000)
	{
		oversampling = 25;
		stim = 1;
	}

	xmc_usic_set_baudrate(regs, speed, oversampling);


  regs->PCR_IICMode = 0
  	| (0    <<  0)  // SLAD(16): Slave Address
  	| (0    << 16)  // ACK00
  	| (stim << 17)  // STIM: 0 = 10 Time Quanta, 1 = 25 Time Quanta
  	| (0    << 18)  // INT.EN.BITS(7)
  	| (0    << 25)  // SACKDIS: Slave Ack Disable
  	| (0    << 26)  // HDEL(4): Hardware Delay
  	| (0    << 30)  // ACKIEN: Ack Interrupt Enable
  	| (0    << 31)  // MCLK: Master Clock Enable
  ;


  regs->SCTR = 0
  	| (1    <<  0)  // SDIR: 1 = MSB first
  	| (1    <<  1)  // PDL: passive data level
  	| (0    <<  2)  // DSM(2): data shift mode, 0 = single wire
  	| (3    <<  8)  // TRM(2): transmission mode
  	| (0x3F << 16)  // FLE(6): frame length, 0x3F = unlimited
  	| (7    << 24)  // WLE(4): word length, fix 8 bits
  ;


  /* Clear protocol status */
  regs->PSCR = 0xFFFFFFFF;

  /*Set input source path*/
  regs->DX0CR = 0
  	| (inputpin_sda << 0)  // DSEL:
  ;

  regs->DX1CR = 0
  	| (inputpin_scl << 0)  // DSEL:
  ;

  // Configure FIFO-s

  unsigned fifooffs = chnum * 32;

  // Configure transmit FIFO
  //XMC_USIC_CH_TXFIFO_Configure(regs, 32, XMC_USIC_CH_FIFO_SIZE_32WORDS, 1);

  /* Disable FIFO */
  regs->TBCTR = 0;

  /* LOF = 0, A standard transmit buffer event occurs when the filling level equals the limit value and gets
   * lower due to transmission of a data word
   * STBTEN = 0, the trigger of the standard transmit buffer event is based on the transition of the fill level
   *  from equal to below the limit, not the fact being below
   */
  regs->TBCTR = 0
    | ((fifooffs + 16) << 0)  // DPTR(6): data pointer
    | (1  << 8)  // LIMIT(6) for interrupt generation
    | (4  << 24) // SIZE(3): size code, 4 = 16 entries
  ;

  // Configure receive FIFO
  //XMC_USIC_CH_RXFIFO_Configure(regs,  0, XMC_USIC_CH_FIFO_SIZE_32WORDS, 0);

  /* Disable FIFO */
  regs->RBCTR = 0;

  /* LOF = 1, A standard receive buffer event occurs when the filling level equals the limit value and gets bigger
   *  due to the reception of a new data word
   */
  regs->RBCTR = 0
      | (fifooffs << 0)  // DPTR(6): data pointer
      | (0 << 8)  // LIMIT(6) for interrupt generation
      | (4 << 24) // SIZE(3): size code, 4 = 16 entries
      | (1 << 28) // LOF: event on limit overflow
  ;

  /* Enable transfer buffer */
  regs->TCSR = 0
  	| (1 << 10) // TDEN
  	| (1 << 8)  // TDSSM
  ;

  // Channel Control Register

  rv = 0
  	| (4 << 0)  // set I2C mode
  ;

  regs->CCR = rv;  // this enables the I2C

	initialized = true;

	return true;
}

int THwI2c_xmc::StartReadData(uint8_t adaddr, unsigned aextra, void * dstptr, unsigned len)
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

	dmaused = false;

	runstate = 0;
	busy = true;  // start the state machine

	Run();

	return ERROR_OK;
}

int THwI2c_xmc::StartWriteData(uint8_t adaddr, unsigned aextra, void * srcptr, unsigned len)
{
	if (busy)
	{
		return ERROR_BUSY;
	}

	istx = true;
	error = 0;
	devaddr = (adaddr & 0x7F);
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

	//extraremaining = 0;

	dmaused = false;

	runstate = 0;
	busy = true;  // start the state machine

	Run();

	return ERROR_OK;
}


void THwI2c_xmc::Run()
{
	if (!busy)
	{
		return;
	}

	uint8_t  isread = (istx ? 0 : 1);
	unsigned psr = regs->PSR_IICMode;

	// check error flags
	if (!error)
	{
		if (psr & (1 << 5))  // NACK
		{
			error = ERR_I2C_ACK;
		}
		else if (psr & (1 << 6)) // ARL
		{
			error = ERR_I2C_ARBLOST;
		}
		else if (psr & ((1 << 1) | (1 << 8)))  // WTDF + ERR
		{
			error = ERR_I2C_BUS;
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

		// start the transaction

		regs->TRBSCR = (3 << 14); // clear transmit and receive FIFO

		if (extraremaining > 0)
		{
			regs->IN[0] = 0
				| (XMC_I2C_TDF_MASTER_START << 8)
				| (devaddr << 1)  // ADDR(7)
				| (0       << 0)  // RD_WRN
			;

			// start with sending the extra data, no autoend, no reload
			while (extraremaining > 0)
			{
				regs->IN[0] = 0
					| (extradata[extracnt - extraremaining] << 0)
					| (XMC_I2C_TDF_MASTER_SEND << 8)
				;

				--extraremaining;
			}

			if (!istx)
			{
				// send restart
				regs->IN[0] = 0
					| (devaddr << 1)  // ADDR(7)
					| (isread  << 0)  // RD_WRN
					| (XMC_I2C_TDF_MASTER_RESTART << 8)
				;

				regs->IN[0] = 0
					| (XMC_I2C_TDF_MASTER_RECEIVE_ACK << 8)
					| (0 << 0) // ignored
				;

				runstate = 20;
			}
			else
			{
				runstate = 10;
			}
		}
		else
		{
			regs->IN[0] = 0
				| (XMC_I2C_TDF_MASTER_START << 8)
				| (devaddr << 1)  // ADDR(7)
				| (isread  << 0)  // RD_WRN
			;

			if (istx)
			{
				runstate = 10;
			}
			else // rx
			{
				regs->IN[0] = 0
					| (XMC_I2C_TDF_MASTER_RECEIVE_ACK << 8)
					| (0 << 0) // ignored
				;

				runstate = 20;
			}
		}

		break;

	case 10: //	sending bytes

		while (remainingbytes > 0)
		{
			if (regs->TRBSR & (1 << 12))  // is the Transmit FIFO full?
			{
				return;
			}

			regs->IN[0] = *dataptr++;  // normal send without any special command
			--remainingbytes;
		}

		if (regs->TRBSR & (1 << 12))  // is the Transmit FIFO full?
		{
			return;
		}

		regs->IN[0] = 0
			| (XMC_I2C_TDF_MASTER_STOP << 8)
			| (0 << 0) // ignored
		;

	  runstate = 30; // terminate
		break;

	case 20: //	receiving bytes

		while (remainingbytes > 0)
		{
			if (regs->TRBSR & (1 << 3))  // is the Receive Buffer Empty?
			{
				return;
			}

			if (regs->TRBSR & (1 << 12))  // is the Transmit FIFO full?
			{
				return;
			}

			*dataptr++ = regs->OUTR;
			--remainingbytes;

			if (remainingbytes > 0)
			{
				regs->IN[0] = 0
					| (XMC_I2C_TDF_MASTER_RECEIVE_ACK << 8)
					| (0 << 0) // ignored
				;
			}
		}

		if (regs->TRBSR & (1 << 12))  // is the Transmit FIFO full?
		{
			return;
		}

		regs->IN[0] = 0
			| (XMC_I2C_TDF_MASTER_STOP << 8)
			| (0 << 0) // ignored
		;

	  runstate = 30; // terminate

		break;

	case 30: // STOP is already initiated, wait for the bus to be idle

		// todo: check bus status

		busy = false; // finished.
		runstate = 50;
	  break;

	case 50: // finished
		break;

	case 90: // handling errors
		runstate = 91;
		break;

	case 91:
		// todo: check bus status
		busy = false; // finished.
		runstate = 50;
		break;

	} // case
}
