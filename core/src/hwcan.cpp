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
 *  file:     hwcan.h
 *  brief:    CAN (Controller Area Network) vendor-independent implementation
 *  version:  1.00
 *  date:     2019-01-12
 *  authors:  nvitya
*/

#include "platform.h"
#include "string.h"
#include "hwcan.h"

void THwCan_pre::InitMsgBuffers(TCanMsg * arxbuf, uint16_t arxcnt, TCanMsg * atxbuf, uint16_t atxcnt)
{
	rxmsgbuf = arxbuf;
	rxmb_count = arxcnt;
	rxmb_idx_rd = 0;
	rxmb_idx_wr = 0;

	txmsgbuf = atxbuf;
	txmb_count = atxcnt;
	txmb_idx_rd = 0;
	txmb_idx_wr = 0;
}

bool THwCan_pre::TryGetRxMessage(TCanMsg * amsg)
{
	if (rxmb_idx_rd != rxmb_idx_wr)
	{
		*amsg = rxmsgbuf[rxmb_idx_rd];
		++rxmb_idx_rd;
		if (rxmb_idx_rd >= rxmb_count)  rxmb_idx_rd = 0;
		return true;
	}
	else
	{
		return false;
	}
}

void THwCan_pre::AddRxMessage(TCanMsg * amsg)
{
	rxmsgbuf[rxmb_idx_wr] = *amsg;
	++rxmb_idx_wr;
	if (rxmb_idx_wr >= rxmb_count)  rxmb_idx_wr = 0;

	if (rxmb_idx_wr == rxmb_idx_rd) // buffer full !
	{
		++lost_rx_msg_cnt;
		// remove the oldest
		++rxmb_idx_rd;
		if (rxmb_idx_rd >= rxmb_count)  rxmb_idx_rd = 0;
	}
}

bool THwCan_pre::TryGetTxMessage(TCanMsg * amsg)
{
	if (txmb_idx_rd != txmb_idx_wr)
	{
		*amsg = txmsgbuf[txmb_idx_rd];
		++txmb_idx_rd;
		if (txmb_idx_rd >= txmb_count)  txmb_idx_rd = 0;
		return true;
	}
	else
	{
		return false;
	}
}

void THwCan_pre::AddTxMessage(TCanMsg * amsg)
{
	txmsgbuf[txmb_idx_wr] = *amsg;
	++txmb_idx_wr;
	if (txmb_idx_wr >= txmb_count)  txmb_idx_wr = 0;

	if (txmb_idx_wr == txmb_idx_rd) // buffer full !
	{
		++lost_tx_msg_cnt;
		// remove the oldest
		++txmb_idx_rd;
		if (txmb_idx_rd >= txmb_count)  txmb_idx_rd = 0;
	}
}

void THwCan_pre::OnRxMessage(TCanMsg * amsg) // should be called from HandleRx()
{
	// can be overridden to process high priority messages,
	// normally we put the incoming messages into the software queue
	AddRxMessage(amsg);
}

//-------------------------------------------------------

bool THwCan::Init(int adevnum, TCanMsg * arxbuf, uint16_t arxcnt, TCanMsg * atxbuf, uint16_t atxcnt)
{
	initialized = false;

	if (!arxbuf || !arxcnt || !atxbuf || !atxcnt)
	{
		return false;
	}

	InitMsgBuffers(arxbuf, arxcnt, atxbuf, atxcnt);

	if (!HwInit(adevnum))
	{
		return false;
	}

	AcceptListClear();

  canbitcpuclocks = SystemCoreClock / speed;

  initialized = true;

  // but do not enable, because the filter list is empty.
  // It can be enabled only after the Acceptance list configuration

	return true;
}

void THwCan::StartSendMessage(TCanMsg * msg)
{
	AddTxMessage(msg);
	HandleTx(); // try to send it
}

bool THwCan::TryRecvMessage(TCanMsg * msg)
{
	HandleRx();
	return TryGetRxMessage(msg);
}

void THwCan::StartSendMessage(uint16_t cobid, void * srcptr, unsigned len)
{
	TCanMsg msg;
	msg.cobid = cobid;
	msg.len = len;

	uint8_t remaining = len;
	uint8_t * src = (uint8_t *)srcptr;
	uint8_t * dst = &msg.data[0];
	while (remaining > 0)
	{
		*dst++ = *src++;
		--remaining;
	}

	StartSendMessage(&msg);
}
