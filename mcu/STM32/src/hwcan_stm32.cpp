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
 *  file:     hwcan_stm32.cpp
 *  brief:    STM32 CAN
 *  version:  1.00
 *  date:     2019-01-12
 *  authors:  nvitya
 *  notes:
 *    only 16 bit filters are used
 *    only RX FIFO 1 is used
*/

#include "platform.h"
#include "hwcan_stm32.h"
#include "clockcnt.h"

#include "stm32_utils.h"

#if defined(HWCAN_IMPL) && !HWCAN_STM32_FD

#include "hwcan.h" // for the eclise indexer

bool THwCan_stm32::HwInit(int adevnum)
{
	uint8_t busid = STM32_BUSID_APB1;
	uint32_t tmp;

	if (false)  { }
#ifdef CAN_BASE
	else if ((adevnum == 1) || (adevnum == 0))
	{
		devnum = 1;
		regs = (HW_CAN_REGS *)CAN_BASE;
		RCC->APB1ENR |= RCC_APB1ENR_CANEN;
	}
#endif
#ifdef CAN1_BASE
	else if (adevnum == 1)
	{
		devnum = 1;
		regs = (HW_CAN_REGS *)CAN1_BASE;
		RCC->APB1ENR |= RCC_APB1ENR_CAN1EN;
	}
#endif
#ifdef CAN2_BASE
	else if (adevnum == 2)
	{
		devnum = 2;
		regs = (HW_CAN_REGS *)CAN2_BASE;
		RCC->APB1ENR |= RCC_APB1ENR_CAN2EN;
	}
#endif
	else
	{
		regs = nullptr;
		devnum = -1;
		return false;
	}
	
	instance_id = devnum - 1;	

	// by default CAN is in sleep mode, so exit from sleep, and go inactive

	regs->MCR &= ~CAN_MCR_SLEEP;
	while (regs->MSR & CAN_MSR_SLAK) { } // wait until it exits from sleep

	regs->MCR |= CAN_MCR_INRQ; // request inactive mode
	while (!(regs->MSR & CAN_MSR_INAK)) { } // wait until it enters inactive mode

	// now it is ready to set up
	regs->MCR = 0
		| (0 << 16)   // DBF: Debug freeze, 0 = can is working during debug, 1 = stopped during debug
		| (0 << 15)   // RESET: 1 = force reset
		| (0 <<  7)   // TTCM: time triggered mode, 0 = disabled
		| (1 <<  6)   // ABOM: Automatic bus-off management, 1 = busoff state is left automatically
		| (0 <<  5)   // AWUM: Automatic wakeup mode, 0 = off
		| (0 <<  4)   // NART: No automatic retransmission, 0 = automatic retransmission
		| (0 <<  3)   // RFLM: Receive FIFO locked mode, 0 = overrun mode, 1 = locked mode
		| (0 <<  2)   // TXFP: Transmit FIFO priority, 0 = by COBID, 1 = chronological
		| (0 <<  1)   // SLEEP: Sleep mode request
		| (1 <<  0)   // INRQ: initialization request
	;

	// set speed

	uint32_t periphclock = stm32_bus_speed(busid);

	uint32_t brp = 1;
	uint32_t ts1, ts2;

	// timing:
  //   1 + ts1 + ts2 = total number of clocks for one CAN bit time
	//   1x bit quanta reserved for sync (at the beginning)
	//   ts1 = offset of sampling point (in bit quanta clocks)
	//   ts2 = bit quanta clocks from sampling point to next bit


	uint32_t bitclocks = periphclock / (brp * speed);
	while (bitclocks > 25)
	{
		++brp;
		bitclocks = periphclock / (brp * speed);
	}

	ts2 = (bitclocks - 1) / 3;
	if (ts2 > 8) ts2 = 8;
	if (ts2 < 1) ts2 = 1;
	ts1 = bitclocks - 1 - ts2;  // should not bigger than 16

	regs->BTR = 0
	  | ((silent_monitor_mode ? 1 : 0) << 31)  // SILM: Silent mode (debug), 1 = silent mode
	  | ((loopback_mode ? 1 : 0)       << 30)  // LBKM: Loop back mode (debug), 1 = loopback mode
	  | (1         << 24)  // SJW(2): Resynchronization jump width, 1 = 2x bit quanta
	  | ((ts2 - 1) << 20)  // TS2(3): Time segment 2
	  | ((ts1 - 1) << 16)  // TS1(4): Time segment 1
	  | ((brp - 1) <<  0)  // BRP(10): Baud rate prescaler
	;

	// setup filtering

	regs->FMR = 1;  // set filter initialization mode

	regs->FM1R = 0; // set ID+MASK mode
	regs->FS1R = 0; // set dual (16 bit scale) mode: 2 entries per filter bank
	regs->FFA1R = 0; // assign all filters to the FIFO1

	regs->FA1R = 0;  // disable all filters for now

	return true;
}

void THwCan_stm32::Enable()
{
	regs->MCR &= ~CAN_MCR_INRQ;
	while (regs->MSR & CAN_MSR_INAK) { } // wait until leaves inactive mode

	regs->FMR = 0; // enable filters
}

void THwCan_stm32::HandleTx()
{
	while (HasTxMessage())
	{
		uint32_t tsr = regs->TSR;
		uint32_t tmi = (tsr >> 24) & 3;  // get the next free Tx mailbox or the lowest priority
		if (tsr & (1 << (26 + tmi))) // is the mailbox free ?
		{
			// ok, we can send the message
			TCanMsg msg;
			if (!TryGetTxMessage(&msg))
			{
				return; // should not happen.
			}

			CAN_TxMailBox_TypeDef * txmb = &regs->sTxMailBox[tmi];

			txmb->TDLR = *(uint32_t *)&msg.data[0];
			txmb->TDHR = *(uint32_t *)&msg.data[4];
			txmb->TDTR = msg.len;

			uint32_t tir = 0
				| ((msg.cobid & 0x7FF) << 21)
				| (1 << 0) // request to start
			;
			if (msg.cobid & HWCAN_RTR_FLAG)  tir |= (1 << 1);
			txmb->TIR = tir;

		  ++tx_msg_counter;
		}
	}
}

void THwCan_stm32::HandleRx()
{
	while (true)
	{
		if ((regs->RF0R & 3) == 0)
		{
			return; // there are no (more) messages
		}

		// read the message
		CAN_FIFOMailBox_TypeDef * rxmb = &regs->sFIFOMailBox[0];

		TCanMsg msg;
		msg.cobid = (rxmb->RIR >> 21);
		*((uint32_t *)&(msg.data[0])) = rxmb->RDLR;
		*((uint32_t *)&(msg.data[4])) = rxmb->RDHR;
		uint32_t rdt = rxmb->RDTR;
		msg.len = (rdt & 15);
		msg.timestamp = CLOCKCNT; // TODO: use the CAN timestamp

		++rx_msg_counter;

		OnRxMessage(&msg); // call the virtual function

		regs->RF0R = (1 << 5); // release the message in the fifo
	}
}

void THwCan_stm32::AcceptListClear()
{
	regs->FMR = 1;  // set filter initialization mode
	regs->FA1R = 0;  // disable all filters
	filtercnt = 0;
}

void THwCan_stm32::AcceptAdd(uint16_t cobid, uint16_t amask)
{
	// in 16 bit mode we can access the filter memory as a 32 bit array
	volatile uint32_t * pu32 = (volatile uint32_t *)&regs->sFilterRegister[0];
	pu32 += filtercnt;
	uint8_t bankid = (filtercnt >> 1); // the enabling is based on banks (filter pairs)

	regs->FA1R &= ~(1 << bankid);  // disable the filter for manipulation

	// assembly the filter word
	uint32_t fw = 0
		| ((cobid <<  5) & 0x0000FFE0)
		| ((amask << 21) & 0xFFE00000)
	;

	*pu32 = fw;
	if ((filtercnt & 1) == 0)
	{
		// write something to the other field too
		++pu32;
		*pu32 = 0xFFE0FFE0;  // should never match
	}

	regs->FA1R |= (1 << bankid);  // enable the filter

	++filtercnt;
}

bool THwCan_stm32::Enabled()
{
	return ((regs->MSR & CAN_MSR_INAK) == 0);
}

bool THwCan_stm32::IsBusOff()
{
	return ((regs->ESR & CAN_ESR_BOFF) != 0);
}

bool THwCan_stm32::IsWarning()
{
	return ((regs->ESR & CAN_ESR_EWGF) != 0);
}

#endif
