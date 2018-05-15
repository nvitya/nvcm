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
 *  file:     hwspi_xmc.cpp
 *  brief:    XMC SPI (master only)
 *  version:  1.00
 *  date:     2018-05-09
 *  authors:  nvitya
*/

#include "platform.h"
//#include "hwclkctrl.h"
#include "hwspi.h"

#include "xmc_utils.h"

bool THwSpi_xmc::Init(int ausicnum, int achnum, int ainputpin)
{
	unsigned code;
	unsigned rv;
	unsigned clockdiv = 1;

	usicnum = ausicnum;
	chnum = achnum;
	inputpin = ainputpin;
	devnum = achnum;

	initialized = false;

	regs = nullptr;
	if (0 == usicnum)
	{
		if (chnum)
		{
			regs = USIC0_CH1;
		}
		else
		{
			regs = USIC0_CH0;
		}

		xmc_enable_periph_clock(SCU_CLK_CGATSTAT0_USIC0_Msk);
	}
#if defined(USIC1)
	else if (1 == usicnum)
	{
		if (chnum)
		{
			regs = USIC1_CH1;
		}
		else
		{
			regs = USIC1_CH0
		}
		xmc_enable_periph_clock(SCU_CLK_CGATSTAT0_USIC1_Msk);
	}
#endif
#if defined(USIC2)
	else if (2 == usicnum)
	{
		if (chnum)
		{
			regs = USIC2_CH1;
		}
		else
		{
			regs = USIC2_CH0
		}
		xmc_enable_periph_clock(SCU_CLK_CGATSTAT0_USIC2_Msk);
	}
#endif
#if defined(USIC3)
	else if (2 == usicnum)
	{
		if (chnum)
		{
			regs = USIC3_CH1;
		}
		else
		{
			regs = USIC3_CH0
		}
		xmc_enable_periph_clock(SCU_CLK_CGATSTAT0_USIC3_Msk);
	}
#endif

	if (!regs)
	{
		return false;
	}
  regs->KSCFG = ((1 << 0) | (1 << 1)); // MODEN + BPMODEN
  while ((regs->KSCFG & 1) == 0U)
  {
    // Wait till the channel is enabled
  }


  // Configure baud rate

  uint32_t clock_divider;
  uint32_t clock_divider_min;

  uint32_t pdiv;
  uint32_t pdiv_int;
  uint32_t pdiv_int_min;

  uint32_t pdiv_frac;
  uint32_t pdiv_frac_min;

  uint32_t peripheral_clock = SystemCoreClock / 100;
  uint32_t rate = speed / 100;

	clock_divider_min = 1;
	pdiv_int_min = 1;
	pdiv_frac_min = 0x3ff;

	for(clock_divider = 1023; clock_divider > 0; --clock_divider)
	{
		pdiv = ((peripheral_clock * clock_divider) / (rate));
		pdiv_int = pdiv >> 10;
		pdiv_frac = pdiv & 0x3ff;

		if ((pdiv_int < 1024) && (pdiv_frac < pdiv_frac_min))
		{
			pdiv_frac_min = pdiv_frac;
			pdiv_int_min = pdiv_int;
			clock_divider_min = clock_divider;
		}
	}

  regs->FDR = ((1 << 15) | (clock_divider_min << 0));

  regs->BRG = 0
  	| ((idleclk_high ? 1 : 0) << 30)    // SCLKCFG(2): 1 = high idle, no delay
  	| (0 << 0)     // CLKSEL(2): 0 = fractional divider
    | (0 << 10)    // DCTQ(5)
    | ((pdiv_int_min - 1) << 16); // PDIV(10)
  ;

  // Configure frame format
  regs->PCR_SSCMode = 0
  	| (1 << 0)    // MSLSEN: 1 = MSLS generation enabled (master mode)
  	| (1 << 1)    // SELCTR: 1 = direct select mode enabled, 0 = coded select mode enabled
  	| (1 << 2)    // SELINV: 1 = active low select
  	| (1 << 3)    // FEM: frame end mode, 1 = no automatic CS pull back
  	| (0 << 4)    // CTQSEL1(2): input freq
  	| (0 << 6)    // PCTQ1(2)
  	| (0 << 8)    // DCTQ1(5)
  	| ((1 << selonum) << 16)   // SELO(8): enabled selection lines
  	| (0 << 24)    // TIWEN: 1 = enable inter-word delay
  	| (0 << 31)    // MCLK: 1 = master clock generation enabled
  ;

  /* Set passive data level, high
     Set word length. Data bits - 1
     If frame length is > 0, frame_lemgth-1; else, FLE = WLE (Data bits - 1)
     Transmission Mode: The shift control signal is considered active if it
     is at 1-level. This is the setting to be programmed to allow data transfers */
  regs->SCTR = 0
  	| ((lsb_first ? 0 : 1) << 0)  // SDIR: 1 = MSB first
  	| (1 << 1)  // PDL: passive data level
  	| (0 << 2)  // DSM(2): data shift mode
  	| (1 << 8)  // TRM(2): transmission mode
  	| (63 << 16) // FLE(6): frame length
  	| ((databits - 1) << 24) // WLE(4): word length
  ;

  /* Clear protocol status */
  regs->PSCR = 0xFFFFFFFF;

  /*Set input source path*/
  regs->DX0CR = 0
  	| (inputpin << 0)  // DSEL: DX0n selected
		| (1 << 4)
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
  	| (1 << 0)  // set SPI mode
  ;
  regs->CCR = rv;  // this enables the SPI

	initialized = true;

	return true;
}

bool THwSpi_xmc::TrySendData(uint16_t adata)
{
	if (regs->TRBSR & (1 << 12))  // is the Transmit FIFO full?
	{
		return false;
	}

	regs->IN[0] = adata; // put the character into the transmit fifo

	return true;
}

bool THwSpi_xmc::TryRecvData(uint16_t * dstptr)
{
	if (regs->TRBSR & (1 << 3))  // is the Receive buffer Empty?
	{
		return false;
	}

	*dstptr = regs->OUTR;

	return true;
}

bool THwSpi_xmc::SendFinished()
{
	if (regs->TRBSR & (1 << 11))  // Transmit buffer empty?
	{
		return true;
	}
	else
	{
		return false;
	}
}
