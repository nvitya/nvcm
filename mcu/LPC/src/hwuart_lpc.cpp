// hwuart_lpc.cpp

#include <stdio.h>
#include <stdarg.h>

#include "hwuart.h"

#include "lpc_utils.h"

bool THwUart_lpc::Init(int adevnum)
{
	unsigned tmp;

	devnum = adevnum;
	initialized = false;

	unsigned pclock;
	unsigned bclock;

	regs = nullptr;

	if (false)
	{

	}
	else if (0 == devnum)
	{
		regs = LPC_USART0;
		lpc_enable_clock(450, 1); // base clock
		lpc_enable_clock(129, 7); // uart clock
	}
	else if (1 == devnum)
	{
		regs = LPC_UART1;
		lpc_enable_clock(418, 1); // base clock
		lpc_enable_clock(130, 7); // uart clock
	}
	else if (2 == devnum)
	{
		regs = LPC_USART2;
		lpc_enable_clock(386, 1); // base clock
		lpc_enable_clock(161, 7); // uart clock
	}
	else if (3 == devnum)
	{
		regs = LPC_USART2;
		lpc_enable_clock(354, 1); // base clock
		lpc_enable_clock(162, 7); // uart clock
	}

	if (!regs)
	{
		return false;
	}

	// Enable FIFOs by default, reset them
	regs->FCR = (UART_FCR_FIFO_EN | UART_FCR_RX_RS | UART_FCR_TX_RS);

  // Disable Tx
  regs->TER2 = 0;

  // Disable interrupts
	regs->IER = 0;
	// Set LCR to default state
	regs->LCR = 0;
	// Set ACR to default state
	regs->ACR = 0;
  // Set RS485 control to default state
	regs->RS485CTRL = 0;
	// Set RS485 delay timer to default state
	regs->RS485DLY = 0;
	// Set RS485 addr match to default state
	regs->RS485ADRMATCH = 0;

	// Clear MCR
	if (regs == LPC_UART1)
	{
		// Set Modem Control to default state
		regs->MCR = 0;
		// Dummy Reading to Clear Status
		tmp = regs->MSR;
	}

	// Default 8N1, with DLAB disabled
	// LINE control

	tmp = UART_LCR_WLEN8; // start with 8 bit characters

	if (parity)
	{
		tmp |= UART_LCR_PARITY_EN;
		if (!oddparity)  tmp |= UART_LCR_PARITY_EVEN;
	}

	if (halfstopbits >= 2)  tmp |= UART_LCR_SBS_2BIT;

	regs->LCR = tmp;

	// Disable fractional divider
	regs->FDR = 0x10;

	unsigned baseclock = SystemCoreClock;

	unsigned div = baseclock / (baudrate * 16);

	// High and low halves of the divider
	unsigned divh = div / 256;
	unsigned divl = div - (divh * 256);

	regs->LCR |= UART_LCR_DLAB_EN;  //	Chip_UART_EnableDivisorAccess(pUART);
	regs->DLL = divl;
	regs->DLM = divh;
	regs->LCR &= ~UART_LCR_DLAB_EN; //	Chip_UART_EnableDivisorAccess(pUART);

	// Enable:
  regs->TER2 = UART_TER2_TXEN;

	initialized = true;

	return true;
}

bool THwUart_lpc::TrySendChar(char ach)
{
	if (regs->LSR & UART_LSR_THRE)
	{
		regs->THR = ach;
		return true;
	}

	return false;
}

bool THwUart_lpc::TryRecvChar(char * ach)
{
	if (regs->LSR & 1)
	{
		*ach = regs->RBR;
		return true;
	}

	return false;
}
