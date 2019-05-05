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
 *  brief:    CAN (Controller Area Network) vendor-independent definitions
 *  version:  1.00
 *  date:     2019-01-12
 *  authors:  nvitya
*/

#ifndef _HWCAN_H_PRE_
#define _HWCAN_H_PRE_

#include "platform.h"
#include "hwpins.h"
#include "errors.h"

typedef struct TCanMsg
{
	uint16_t   cobid;
	uint8_t    len;
	uint8_t    _reserved0;
	uint32_t   timestamp;   // in CPU clocks

	uint8_t    data[8];
//
} TCanMsg; // 16 Bytes

class THwCan_pre
{
public:	// settings
	bool        initialized = false;
	unsigned    speed = 1000000;  // 1 MBit/s by default
	int         devnum = -1;

	bool        silent_monitor_mode = false;
	bool        loopback_mode = false;

	unsigned    canbitcpuclocks = 0;

public: // software queues
	TCanMsg *   rxmsgbuf = nullptr;
	TCanMsg *   txmsgbuf = nullptr;
	uint16_t    rxmb_count = 0;
	uint16_t    txmb_count = 0;

	uint16_t    rxmb_idx_wr = 0;
	uint16_t    rxmb_idx_rd = 0;

	uint16_t    txmb_idx_wr = 0;
	uint16_t    txmb_idx_rd = 0;

	uint32_t    lost_rx_msg_cnt = 0;
	uint32_t    lost_tx_msg_cnt = 0;

	void        InitMsgBuffers(TCanMsg * arxbuf, uint16_t arxcnt, TCanMsg * atxbuf, uint16_t atxcnt);

	bool        TryGetRxMessage(TCanMsg * amsg);
	void        AddRxMessage(TCanMsg * amsg);

	bool        TryGetTxMessage(TCanMsg * amsg);
	void        AddTxMessage(TCanMsg * amsg);
	inline bool HasTxMessage() { return (txmb_idx_rd != txmb_idx_wr); }

public:
	virtual ~THwCan_pre() { } // virtual destructor to avoid warning

	virtual void OnRxMessage(TCanMsg * amsg);
};

#endif // ndef HWCAN_H_PRE_

#ifndef HWCAN_PRE_ONLY

//-----------------------------------------------------------------------------

#ifndef HWCAN_H_
#define HWCAN_H_

#include "mcu_impl.h"

#ifndef HWCAN_IMPL

class THwCan_noimpl : public THwCan_pre
{
public: // mandatory
	bool HwInit(int adevnum)                          { return false; }

	void HandleTx()                                   { }
	void HandleRx()                                   { }

	void SetSpeed(uint32_t aspeed)                    { }
	void AcceptListClear()                            { }
	void AcceptAdd(uint16_t cobid, uint16_t amask)    { }

	bool IsBusOff()                                   { return false; }
	bool IsWarning()                                  { return false; }

	void Enable()                                     { }
	void Disable()                                    { }
	bool Enabled()                                    { return false; }
};

#define HWCAN_IMPL THwCan_noimpl

#endif // ndef HWCAN_IMPL

//-----------------------------------------------------------------------------

class THwCan : public HWCAN_IMPL
{
public:
	bool  Init(int adevnum, TCanMsg * arxbuf, uint16_t arxcnt, TCanMsg * atxbuf, uint16_t atxcnt);

	bool  TryRecvMessage(TCanMsg * msg);

	void  StartSendMessage(TCanMsg * msg);
	void  StartSendMessage(uint16_t cobid, void * srcptr, unsigned len);
};

#endif // HWCAN_H_

#else
  #undef HWCAN_PRE_ONLY
#endif
