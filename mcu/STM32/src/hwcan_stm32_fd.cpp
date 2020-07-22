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
 *  file:     hwcan_stm32_fd.cpp
 *  brief:    STM32 CAN driver for devices with FD CAN controller (G4)
 *  version:  1.00
 *  date:     2020-03-21
 *  authors:  nvitya
 *  notes:
 *    Classic CAN only, FD features are disabled
 *    The HW is very similar to ATSAM_V2 (ATSAME5x)
*/

#include "string.h"
#include "platform.h"
#include "hwcan_stm32.h"
#include "clockcnt.h"

#include "stm32_utils.h"

#if defined(HWCAN_IMPL) && HWCAN_STM32_FD

#include "hwcan.h" // for the eclise indexer

#define HWCAN_MAX_FILTERS   16

#define SRAMCAN_FLSSA  0x0000  // Filter List Standard Start Address
#define SRAMCAN_FLESA  0x0070  // Filter List Extended Start Address
#define SRAMCAN_RF0SA  0x00B0  // Rx FIFO 0 Start Address (warning the reference manual shows 0x80! for this)
#define SRAMCAN_RF1SA  0x0188  // Rx FIFO 1 Start Address
#define SRAMCAN_TEFSA  0x0260  // Tx Event FIFO Start Address
#define SRAMCAN_TBSA   0x0278  // Tx FIFO/Queue Start Address
#define SRAMCAN_SIZE   0x0350  // Message RAM size

bool THwCan_stm32::HwInit(int adevnum)
{
	uint8_t busid = STM32_BUSID_APB1;
	uint32_t tmp;

	if (false)  { }
#ifdef FDCAN1
	else if (adevnum == 1)
	{
		devnum = 1;
		regs = FDCAN1;
		canram_base = SRAMCAN_BASE;
		RCC->APB1ENR1 |= RCC_APB1ENR1_FDCANEN;
	}
#endif
#ifdef FDCAN2
	else if (adevnum == 2)
	{
		devnum = 2;
		regs = FDCAN2;
		canram_base = SRAMCAN_BASE + SRAMCAN_SIZE;
		RCC->APB1ENR1 |= RCC_APB1ENR1_FDCANEN;
	}
#endif
#ifdef FDCAN3
	else if (adevnum == 3)
	{
		devnum = 3;
		regs = FDCAN3;
		canram_base = SRAMCAN_BASE + (SRAMCAN_SIZE << 1);
		RCC->APB1ENR1 |= RCC_APB1ENR1_FDCANEN;
	}
#endif
	else
	{
		regs = nullptr;
		devnum = -1;
		return false;
	}
	
	instance_id = devnum - 1;

	// reset sram
	memset((void *)(canram_base), 0, SRAMCAN_SIZE);

	// set CAN memory pointers
	stdfilters = (uint32_t *)(canram_base + SRAMCAN_FLSSA);
	rxfifo = (hwcan_rx_fifo_t *)(canram_base + SRAMCAN_RF0SA);
	txfifo = (hwcan_tx_fifo_t *)(canram_base + SRAMCAN_TBSA);

	// select PCLK as the CANFD base clock (=SystemCoreClock)
	tmp = RCC->CCIPR;
	tmp &= ~RCC_CCIPR_FDCANSEL_Msk;
	tmp |= RCC_CCIPR_FDCANSEL_1;
	RCC->CCIPR = tmp;

	// by default CAN is in sleep mode, so exit from sleep, and go inactive

	regs->CCCR &= ~FDCAN_CCCR_CSR;
	while (regs->CCCR & FDCAN_CCCR_CSA) { } // wait until it exits from sleep

	regs->CCCR |= FDCAN_CCCR_INIT; // request inactive mode
	while (!(regs->CCCR & FDCAN_CCCR_INIT)) { } // wait until it enters inactive mode

	regs->CCCR |= FDCAN_CCCR_CCE; // enable configuration change

	// now it is ready to set up
	regs->CCCR = 0
		| (0  << 15)  // NISO: 0 = ISO compatible FD Frame format
		| (0  << 14)  // TXP: 0 = no 2 bit transmit pause
		| (0  << 13)  // EFBI: 0 = no edge filtering
		| (0  << 12)  // PXHD: 0 = protocol exception handling enabled (not disabled)
		| (0  <<  9)  // BRSE: 0 = disable bit rate switching
		| (0  <<  8)  // FDOE: 0 = disable FD operation
		| (0  <<  7)  // TEST: 0 = normal operation, 1 = enable write access to the TEST register
		| (0  <<  6)  // DAR: 0 = autoamic retransmission on failure
		| (0  <<  5)  // MON: 0 = Bus monitoring mode disabled
		| (0  <<  4)  // CSR: 0 = normal mode, 1 = power down
		| (0  <<  3)  // CSA: clock stop (power down) ack
		| (0  <<  2)  // ASM: 0 = normal CAN operation, 1 = ASM restricted operation
		| (1  <<  1)  // CCE: Configuration Change Enable (already set before)
		| (1  <<  0)  // INIT: Initialization (already set before)
	;

	if (loopback_mode)
	{
		regs->CCCR |= (1 << 7); // enable write access to the TEST register

		regs->TEST = 0
			| (0 << 5) // TX(2): 0 = normal mode
			| (1 << 4) // LBCK: 1 = enable loopback mode
		;
	}

	regs->TSCC = 0
	  | (0 << 16)  // TCP(4): Timestamp Counter Prescaler
	  | (1 <<  0)  // TSS(2): Timestamp Select, 1 = Count CAN Bit times
	;

	regs->TOCC = 0xFFFF0000; // disable timeout counters

	SetSpeed(speed);

	// setup filtering
	AcceptListClear();

	regs->RXGFC = 0
	  | (0  << 24)  // LSE(4): List Size Extended
	  | (HWCAN_MAX_FILTERS << 16)  // LSS(5): List Size Standard
	  | (0  <<  9)  // F0OM: 0 = FIFO 0 blocking mode, 1 = overwrite
	  | (0  <<  8)  // F1OM: 0 = FIFO 1 blocking mode, 1 = overwrite
	  | (3  <<  4)  // ANFS(2): 3 = reject non-mathching frames in standard filters
	  | (3  <<  2)  // ANFE(2): 3 = reject non-mathching frames in extended filters
	  | (0  <<  1)  // RRFS: 0 = filter remote frames (standard)
	  | (0  <<  0)  // RRFE: 0 = filter remote frames (extended)
	;

	regs->TXBC = 0
		| (0 << 24) // TFQM: 0 = Tx FIFO operation
	;

	// setup interrupts
	regs->ILS = 0; // select line 0 for all the interrupts
	regs->ILE = 1; // enable only IRQ line 0
	regs->IE = 1;  // enable only RX FIFO0 New Message Interrupt

	return true;
}

void THwCan_stm32::Enable()
{
	regs->CCCR &= ~(FDCAN_CCCR_CCE | FDCAN_CCCR_INIT);
}

bool THwCan_stm32::Enabled()
{
	return ((regs->CCCR & (FDCAN_CCCR_INIT | FDCAN_CCCR_CCE)) == 0);
}

void THwCan_stm32::Disable()
{
	regs->CCCR |= FDCAN_CCCR_INIT; // request inactive mode
	while (!(regs->CCCR & FDCAN_CCCR_INIT)) { } // wait until it enters inactive mode

	regs->CCCR |= FDCAN_CCCR_CCE; // enable configuration change
}

void THwCan_stm32::SetSpeed(uint32_t aspeed)
{
	bool wasenabled = Enabled();
	if (wasenabled)
	{
		Disable();
	}

	speed = aspeed;

	// setup common clock divider
	FDCAN_CONFIG->CKDIV = 0;

	// set speed (nominal only)

	uint32_t periphclock = SystemCoreClock / 1;

	uint32_t brp = 1;  // bit rate prescaler
	uint32_t ts1, ts2;

	// timing:
  //   1 + ts1 + ts2 = total number of clocks for one CAN bit time
	//   1x bit quanta reserved for sync (at the beginning)
	//   ts1 = offset of sampling point (in bit quanta clocks)
	//   ts2 = bit quanta clocks from sampling point to next bit


	uint32_t bitclocks = periphclock / (brp * speed);
	while (bitclocks > 384)
	{
		++brp;
		bitclocks = periphclock / (brp * speed);
	}

	ts2 = (bitclocks - 1) / 3;
	if (ts2 > 128) ts2 = 128;
	if (ts2 < 1) ts2 = 1;
	ts1 = bitclocks - 1 - ts2;  // should not bigger than 16

	regs->NBTP = 0
	  | (ts2  << 25)  // NSJW(7): Resynchronization jump width
	  | ((brp - 1)  << 16)  // NBRP(9): Bit Rate Prescaler
	  | ((ts1 - 1)  <<  8)  // NTSEG1(8): Time segment 1
	  | ((ts2 - 1)  <<  0)  // NTSEG2(7): Time segment 2
	;

	// do not change the (fast) Data Bit timing register, it won't be used anyway
	// regs->DBTP = 0;

	if (wasenabled)
	{
		Enable();
	}
}

void THwCan_stm32::HandleTx() // warning it can be called from multiple contexts!
{
	unsigned pm = __get_PRIMASK();  // save interrupt disable status
	__disable_irq();

	while (HasTxMessage())
	{
		uint32_t txfs = regs->TXFQS;  // store Tx FIFO status register

		if (txfs & FDCAN_TXFQS_TFQF) // is the FIFO full?
		{
			break;
		}

		uint32_t tpi = ((txfs >> 16) & 0x03); // get the put index
		if (tpi >= 3)
		{
			break;
		}

		// ok, we can send the message
		TCanMsg msg;
		if (!TryGetTxMessage(&msg))
		{
			break; // should not happen.
		}

		hwcan_tx_fifo_t * txmb = (txfifo + tpi);

		txmb->DATAL = *(uint32_t *)&msg.data[0]; // must be aligned
		txmb->DATAH = *(uint32_t *)&msg.data[4];
		txmb->DLC = (msg.len << 16);
		uint32_t idfl = ((msg.cobid & 0x7FF) << 18);
		if (msg.cobid & HWCAN_RTR_FLAG)  idfl |= (1 << 29);
		txmb->IDFL = idfl;

		regs->TXBAR = (1 << tpi); // add the transmit request

		++tx_msg_counter;
	}

	// update the error counter here, because the HandleTx() called always regularly
	uint32_t ecnt_reg = regs->ECR; // reading this clears the CEL (bits16..23) !
	bus_error_count += ((ecnt_reg >> 16) & 0xFF);

 	__set_PRIMASK(pm); // restore interrupt disable status
}

void THwCan_stm32::HandleRx()
{
	while (true)
	{
		uint32_t rxfs = regs->RXF0S;  // store Rx FIFO(0) status register

		if ((rxfs & 0x3) == 0)
		{
			regs->IR = 1; // clear the interrupt (RF0N = New Message in RxFIFO0)
			if (regs->IR) { } // some sync
			return; // there are no (more) messages
		}

		uint8_t rgi = ((rxfs >> 8) & 3);

		// read the message
		hwcan_rx_fifo_t * rxmb = (rxfifo + rgi);

		TCanMsg msg;

		msg.cobid = ((rxmb->ID >> 18) & 0x7FF);
		*((uint32_t *)&(msg.data[0])) = rxmb->DATAL;
		*((uint32_t *)&(msg.data[4])) = rxmb->DATAH;
		uint32_t dt = rxmb->DLCTS;
		msg.len = ((dt >> 16) & 15);
		msg.timestamp = CLOCKCNT; // TODO: use the CAN timestamp

		++rx_msg_counter;

		OnRxMessage(&msg); // call the virtual function

		regs->RXF0A = rgi; // acknowledge the read
	}
}

void THwCan_stm32::AcceptListClear()
{
	bool wasenabled = Enabled();
	if (wasenabled)
	{
		Disable();
	}

	memset(stdfilters, 0, 4 * HWCAN_MAX_FILTERS);
	filtercnt = 0;

	if (wasenabled)
	{
		Enable();
	}
}

void THwCan_stm32::AcceptAdd(uint16_t cobid, uint16_t amask)
{
	if (filtercnt >= HWCAN_MAX_FILTERS)
	{
		return;
	}

	// assembly the filter word
	uint32_t fw = 0
		| ((amask & 0x7FF) <<  0)
		| ((cobid & 0x7FF) << 16)
		| (1               << 27)  // SFEC(3): 1 = store in Rx FIFO 0 if filter matches
		| (2               << 30)  // SFT(2): 2 = mask + id, (0 = range, 1 = dual ID)
	;

	stdfilters[filtercnt] = fw;

	++filtercnt;
}


bool THwCan_stm32::IsBusOff()
{
	return (regs->PSR & FDCAN_PSR_BO);
}

bool THwCan_stm32::IsWarning()
{
	return (regs->PSR & FDCAN_PSR_EW);
}

#endif
