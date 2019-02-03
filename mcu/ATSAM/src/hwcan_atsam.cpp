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
 *  file:     hwcan_atsam.cpp
 *  brief:    ATSAM CAN
 *  version:  1.00
 *  date:     2019-01-13
 *  authors:  nvitya
*/

#include "string.h"
#include "platform.h"
#include "hwcan_atsam.h"
#include "clockcnt.h"
#include "atsam_utils.h"

#ifdef HWCAN_IMPL

bool THwCan_atsam::HwInit(int adevnum)
{
	uint32_t tmp;
	unsigned perid;

	if (false)  { }
#ifdef MCAN0
	else if (adevnum == 0)
	{
		devnum = 0;
		regs = (HW_CAN_REGS *)MCAN0;
		perid = ID_MCAN0;

    #ifdef CCFG_CAN0_CAN0DMABA_Pos
			// set the upper address of the message buffers
		  MATRIX->CCFG_CAN0 &= 0x0000FFFF;
		  MATRIX->CCFG_CAN0 |= ((uint32_t)(&stdfilters) & 0xFFFF0000);
    #endif
	}
#endif
#ifdef MCAN1
	else if (adevnum == 1)
	{
		devnum = 1;
		regs = (HW_CAN_REGS *)MCAN1;
		perid = ID_MCAN1;

		#ifdef CCFG_SYSIO_CAN1DMABA_Pos
			// set the upper address of the message buffers
			MATRIX->CCFG_SYSIO &= 0x0000FFFF;
			MATRIX->CCFG_SYSIO |= ((uint32_t)(&stdfilters) & 0xFFFF0000);
		#endif
	}
#endif
	else
	{
		regs = nullptr;
		devnum = -1;
		return false;
	}

	atsam_enable_peripheral(perid);

	unsigned periphclock = atsam_peripheral_clock();

	// set to init mode
	regs->MCAN_CCCR |= MCAN_CCCR_INIT;
	while ((regs->MCAN_CCCR & MCAN_CCCR_INIT) == 0) { } // wait for ack

	regs->MCAN_CCCR |= MCAN_CCCR_CCE;  // set config change enable

	// initialize memory addresses

	memset((void *)&stdfilters[0], 0, sizeof(stdfilters));
	memset((void *)&rxfifo[0], 0, sizeof(rxfifo));
	memset((void *)&txfifo[0], 0, sizeof(txfifo));

	regs->MCAN_SIDFC = 0 // standard filters
		| (((uint32_t)(&stdfilters[0]) & 0xFFFF) << 0)
		| (HWCAN_MAX_FILTERS << 16)
	;

	regs->MCAN_XIDFC = 0 // no extended filters
		| (((uint32_t)(&extfilters[0]) & 0xFFFF) << 0)
		| (0 << 16)
	;

	regs->MCAN_GFC = 0
		| (1 << 0)  // RRFE: 1 = reject remote frames with extended ID
		| (1 << 1)  // RRFS: 1 = reject remote frames with standard ID
		| (3 << 2)  // ANFE(2): 3 = reject non-matching extended ID
		| (3 << 4)  // ANFS(2): 3 = reject non-matching standard ID
	;

	regs->MCAN_RXF0C = 0 // RX FIFO
		| (((uint32_t)(&rxfifo[0]) & 0xFFFF) << 0)
		| (HWCAN_RX_FIFO_SIZE << 16)
		| (0 << 24) // F0WM(7): fifo watermark, 0 = watermark interrupt disabled
		| (1 << 31) // F0OM: Operatin Mode, 0 = blocking, 1 = overwrite
	;

	regs->MCAN_RXF1C = 0; // no RX FIFO1, use only single FIFO
	regs->MCAN_RXBC = 0;  // no dedicated RX BUFFERS
	regs->MCAN_RXESC = 0; // all 8 byte data size

	regs->MCAN_TXBC = 0 // TX FIFO
		| (((uint32_t)(&txfifo[0]) & 0xFFFF) << 0)
		| (0                  << 16) // NDTB(6): Number of Dedicated Transmit Buffers
		| (HWCAN_TX_FIFO_SIZE << 24) // TFQS(6): Transmit FIFO/Queue Size
		| (0                  << 31) // TFQM: 0 = FIFO mode, 1 = queue mode
	;

	regs->MCAN_TXEFC = 0; // no TX event fifo

	regs->MCAN_RWD = 0;

	regs->MCAN_TSCC = 0
		| (1 <<  0)  // TSS(2): 1 = increment time stamp
		| (1 << 16)  // TCP(4): timestamp prescaler
	;

	regs->MCAN_TOCC = 0; // disable timeout counter

	regs->MCAN_CCCR = 0
		| (1 <<  0)  // INIT: 1 = initialization mode
		| (1 <<  1)  // CCE: Configuration Change Enable, 0 = protected, 1 = configurable
		| (0 <<  2)  // ASM: Restricted Operation Mode, 0 = normal mode
		| (0 <<  3)  // CSA: Clock Stop Acknowledge(ro)
		| (0 <<  4)  // CSR: Clock Stop Request
		| (0 <<  5)  // MON: Bus Monitoring Mode, 0 = disabled
		| (0 <<  6)  // DAR: Disable Automatic Retransmission, 0 = automati retransmit enabled
		| (0 <<  7)  // TEST: Test Mode Enable
		| (0 <<  8)  // FDOE: CAN FD Operation Enable, 0 = disabled
		| (0 <<  9)  // BRSE: Bit Rate Switching Enable, 0 = disabled
		| (0 << 12)  // PXHD: 0 = Protocol exception handling enabled
		| (0 << 13)  // EFBI: Edge Filtering during Bus Integration, 0 = disabled
		| (0 << 14)  // TXP: Tranmit Pause, 0 = disabled
		| (0 << 15)  // NISO: Non-Iso Operation, 0 = ISO operation
	;


	uint32_t brp = 1;
	uint32_t ts1, ts2;

	// timing:
  //   1 + ts1 + ts2 = total number of clocks for one CAN bit time
	//   1x bit quanta reserved for sync (at the beginning)
	//   ts1 = offset of sampling point (in bit quanta clocks)
	//   ts2 = bit quanta clocks from sampling point to next bit


	uint32_t bitclocks = periphclock / (brp * speed);
	while (bitclocks > 48)
	{
		++brp;
		bitclocks = periphclock / (brp * speed);
	}

	ts2 = (bitclocks - 1) / 3;
	if (ts2 > 16) ts2 = 16;
	if (ts2 < 1) ts2 = 1;
	ts1 = bitclocks - 1 - ts2;  // should not be bigger than 64

	// normal bit timing
	regs->MCAN_BTP = 0
    | (3         <<  0)  // SJW(4): Resynchronization jump width
    | ((ts2 - 1) <<  4)  // TSEG2(4):
	  | ((ts1 - 1) <<  8)  // TSEG1(6):
	  | ((brp - 1) << 16)  // BRP(10): baud rate prescaler
	;

	// fast bit timing (not used)
	regs->MCAN_FBTP = 0
    | (3         <<  0)  // SJW(4): Resynchronization jump width
    | ((ts2 - 1) <<  4)  // TSEG2(4):
	  | ((ts1 - 1) <<  8)  // TSEG1(6):
	  | ((brp - 1) << 16)  // BRP(10): baud rate prescaler
	  | (0         << 23)  // TDC: 1 = Transceiver Delay Compensation Enabled
	  | (0         << 24)  // TDCO(5):  Transceiver Delay Compensation Offset
	;

	regs->MCAN_ILE = 1;    // enable interrupt 1 only

	return true;
}

void THwCan_atsam::Enable()
{
	regs->MCAN_CCCR &= ~MCAN_CCCR_CCE;
	regs->MCAN_CCCR &= ~MCAN_CCCR_INIT;
	//while (regs->MCAN_CCCR & MCAN_CCCR_INIT) { }
}

void THwCan_atsam::HandleTx()
{
	if (HasTxMessage())
	{
		uint32_t txfs = regs->MCAN_TXFQS;  // store Tx FIFO status register

		if (txfs & MCAN_TXFQS_TFQF) // is the FIFO full?
		{
			return;
		}

		// ok, we can send the message
		TCanMsg msg;
		if (!TryGetTxMessage(&msg))
		{
			return; // should not happen.
		}

		uint8_t tpi = ((txfs >> 16) & 0x1F); // get the put index

		hwcan_tx_fifo_t * txmb = &txfifo[tpi];

		txmb->DATAL = *(uint32_t *)&msg.data[0]; // must be aligned
		txmb->DATAH = *(uint32_t *)&msg.data[4];
		txmb->DLC = (msg.len << 16);
		txmb->IDFL = (msg.cobid << 18);

		regs->MCAN_TXBAR = (1 << tpi); // add the transmit request
	}
}

void THwCan_atsam::HandleRx()
{
	while (true)
	{
		uint32_t rxfs = regs->MCAN_RXF0S;  // store Rx FIFO status register

		if ((rxfs & 0x3F) == 0)
		{
			return; // there are no (more) messages
		}

		uint8_t rgi = ((rxfs >> 8) & 0x3F);

		// read the message
		hwcan_rx_fifo_t * rxmb = &rxfifo[rgi];

		TCanMsg msg;

		msg.cobid = ((rxmb->ID >> 18) & 0x7FF);
		*((uint32_t *)&(msg.data[0])) = rxmb->DATAL;
		*((uint32_t *)&(msg.data[4])) = rxmb->DATAH;
		uint32_t dt = rxmb->DLCTS;
		msg.len = ((dt >> 16) & 15);
		msg.timestamp = CLOCKCNT; // TODO: use the CAN timestamp

		AddRxMessage(&msg);

		regs->MCAN_RXF0A = rgi; // acknowledge the read
	}
}

void THwCan_atsam::AcceptListClear()
{
	filtercnt = 0;
	memset((void *)&stdfilters[0], 0, sizeof(stdfilters));
}

void THwCan_atsam::AcceptAdd(uint16_t cobid, uint16_t amask)
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

bool THwCan_atsam::Enabled()
{
	return ((regs->MCAN_CCCR & MCAN_CCCR_INIT) == 0);
}

bool THwCan_atsam::IsBusOff()
{
	return ((regs->MCAN_PSR & MCAN_PSR_BO) != 0);
}

bool THwCan_atsam::IsWarning()
{
	return ((regs->MCAN_PSR & MCAN_PSR_EW) != 0);
}

#endif
