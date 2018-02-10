// hwuart_imxrt.cpp

#include <stdio.h>
#include <stdarg.h>

#include "imxrt_utils.h"
#include "hwuart.h"

bool THwUart_imxrt::Init(int adevnum)  // devnum: 0 - 4 = UART0..4, 0x100..0x103 = USART0..3
{
	unsigned code;
	unsigned tmp;
	unsigned perid;

	devnum = adevnum;

	initialized = false;

	// UART base clock: PLL3 = 480 MHz / 6 (fixed) = 80 MHz
	tmp = CCM->CSCDR1;
	tmp &= ~(CCM_CSCDR1_UART_CLK_SEL_MASK | CCM_CSCDR1_UART_CLK_PODF_MASK);
	tmp |=  CCM_CSCDR1_UART_CLK_SEL(0);    // 0 = PLL3 = 80MHz
	tmp |=  CCM_CSCDR1_UART_CLK_PODF(0);   // 0 = no post division (/6 already applied fix before)
	CCM->CSCDR1 = tmp;
	unsigned baseclock = 80000000;

	regs = nullptr;

	if (1 == devnum)
	{
		regs = (HW_UART_REGS *)LPUART1;
		imxrt_set_clock_gate(5, 24, 3);
	}
	else if (2 == devnum)
	{
		regs = (HW_UART_REGS *)LPUART2;
		imxrt_set_clock_gate(0, 28, 3);
	}
	else if (3 == devnum)
	{
		regs = (HW_UART_REGS *)LPUART3;
		imxrt_set_clock_gate(0, 12, 3);
	}
	else if (4 == devnum)
	{
		regs = (HW_UART_REGS *)LPUART4;
		imxrt_set_clock_gate(0, 12, 3);
	}
	else if (5 == devnum)
	{
		regs = (HW_UART_REGS *)LPUART5;
		imxrt_set_clock_gate(3,  2, 3);
	}
	else if (6 == devnum)
	{
		regs = (HW_UART_REGS *)LPUART6;
		imxrt_set_clock_gate(3,  6, 3);
	}
	else if (7 == devnum)
	{
		regs = (HW_UART_REGS *)LPUART7;
		imxrt_set_clock_gate(5, 26, 3);
	}
	else if (8 == devnum)
	{
		regs = (HW_UART_REGS *)LPUART8;
		imxrt_set_clock_gate(6, 14, 3);
	}

	if (!regs)
	{
		return false;
	}

	// From IMXRT FSL library:

  /* This LPUART instantiation uses a slightly different baud rate calculation
   * The idea is to use the best OSR (over-sampling rate) possible
   * Note, OSR is typically hard-set to 16 in other LPUART instantiations
   * loop to find the best OSR value possible, one that generates minimum baudDiff
   * iterate through the rest of the supported values of OSR */

  uint32_t temp;
  uint16_t sbr, sbrTemp;
  uint32_t osr, osrTemp, tempDiff, calculatedBaud, baudDiff;

  baudDiff = baudrate;
  osr = 0;
  sbr = 0;
  for (osrTemp = 4; osrTemp <= 32; osrTemp++)
  {
      /* calculate the temporary sbr value   */
      sbrTemp = (baseclock / (baudrate * osrTemp));
      /*set sbrTemp to 1 if the sourceClockInHz can not satisfy the desired baud rate*/
      if (sbrTemp == 0)
      {
          sbrTemp = 1;
      }
      /* Calculate the baud rate based on the temporary OSR and SBR values */
      calculatedBaud = (baseclock / (osrTemp * sbrTemp));

      tempDiff = calculatedBaud - baudrate;

      /* Select the better value between srb and (sbr + 1) */
      if (tempDiff > (baudrate - (baseclock / (osrTemp * (sbrTemp + 1)))))
      {
          tempDiff = baudrate - (baseclock / (osrTemp * (sbrTemp + 1)));
          sbrTemp++;
      }

      if (tempDiff <= baudDiff)
      {
          baudDiff = tempDiff;
          osr = osrTemp; /* update and store the best OSR value calculated */
          sbr = sbrTemp; /* update store the best SBR value calculated */
      }
  }

  // reset
  regs->GLOBAL |=  LPUART_GLOBAL_RST_MASK;
  regs->GLOBAL &= ~LPUART_GLOBAL_RST_MASK;

  // Disalble transmit and receive
  regs->CTRL &= ~(LPUART_CTRL_TE_MASK | LPUART_CTRL_RE_MASK);

  // BAUD register: baud rate + some other line parameters
  //------------------------------------------------------
  tmp = 0
  	| ((osr-1) << 24)
		| (sbr << 0)
  ;

  if ((osr > 3) && (osr < 8))
  {
  	tmp |= LPUART_BAUD_BOTHEDGE_MASK; // for worse oversampling BOTHEDGE must be turned on
  }

	if (halfstopbits > 3)
	{
		tmp |= LPUART_BAUD_SBNS_MASK;
	}

	regs->BAUD = tmp;

	// CTRL register
  //------------------------------------------------------
  tmp = 0
		| (0 << 11)  // M7: 0 = 8 bit characters
		| (0 <<  7)  // LOOPS: 1 = loop mode select
		| (0 <<  4)  // M: 0 = 8 bit characters
  ;

  if (parity)
  {
  	tmp |= (1 < 1);
  	if (oddparity)  tmp |= (1 << 0);
  }

  regs->CTRL = tmp;

  // FIFO Setup
  //------------------------------------------------------

  /* Set tx/rx WATER watermark
     Note:
     Take care of the RX FIFO, RX interrupt request only assert when received bytes
     equal or more than RX water mark, there is potential issue if RX water
     mark larger than 1.
     For example, if RX FIFO water mark is 2, upper layer needs 5 bytes and
     5 bytes are received. the last byte will be saved in FIFO but not trigger
     RX interrupt because the water mark is 2.
   */
  regs->WATER = 0
  	| (1 << 16) // RX FIFO watermark
		| (1 <<  0) // TX FIFO watermark
	;

  // Enable tx/rx FIFO
  regs->FIFO |= (LPUART_FIFO_TXFE_MASK | LPUART_FIFO_RXFE_MASK);

  // Flush FIFO
  regs->FIFO |= (LPUART_FIFO_TXFLUSH_MASK | LPUART_FIFO_RXFLUSH_MASK);

  // STAT (status) register
  //------------------------------------------------------
  // Clear all status flags
  tmp = (LPUART_STAT_RXEDGIF_MASK | LPUART_STAT_IDLE_MASK | LPUART_STAT_OR_MASK | LPUART_STAT_NF_MASK
  		    | LPUART_STAT_FE_MASK | LPUART_STAT_PF_MASK
					| LPUART_STAT_LBKDIF_MASK
					| LPUART_STAT_MA1F_MASK | LPUART_STAT_MA2F_MASK
				);

  regs->STAT = tmp;

  //------------------------------------------------------

	// Enable:
  regs->CTRL |= (LPUART_CTRL_TE_MASK | LPUART_CTRL_RE_MASK);

	initialized = true;

	return true;
}

bool THwUart_imxrt::TrySendChar(char ach)
{
	if (regs->STAT & LPUART_STAT_TDRE_MASK)
	{
		regs->DATA = ach;
		return true;
	}
	else
	{
		return false;
	}
}

bool THwUart_imxrt::TryRecvChar(char * ach)
{
	if (regs->WATER & LPUART_WATER_RXCOUNT_MASK)
	{
		*ach = regs->DATA;
		return true;
	}
	else
	{
		return false;
	}
}

#if 0

void THwUart_imxrt::DmaAssign(bool istx, THwDmaChannel * admach)
{
	if (istx)
	{
		txdma = admach;
		admach->Prepare(istx, (void *)&regs->UART_THR, 0);
	}
	else
	{
		rxdma = admach;
		admach->Prepare(istx, (void *)&regs->UART_RHR, 0);
	}
}

bool THwUart_imxrt::DmaStartSend(THwDmaTransfer * axfer)
{
	if (!txdma)
	{
		return false;
	}

	txdma->StartTransfer(axfer);

	return true;
}

bool THwUart_imxrt::DmaStartRecv(THwDmaTransfer * axfer)
{
	if (!rxdma)
	{
		return false;
	}

	rxdma->StartTransfer(axfer);

	return true;
}

#endif
