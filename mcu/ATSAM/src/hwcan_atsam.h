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
 *  file:     hwcan_atsam.h
 *  brief:    ATSAM CAN
 *  version:  1.00
 *  date:     2019-01-13
 *  authors:  nvitya
*/

#ifndef HWCAN_ATSAM_H_
#define HWCAN_ATSAM_H_

#include "platform.h"

#if defined(MCAN0)

#define HWCAN_PRE_ONLY
#include "hwcan.h"

#define HW_CAN_REGS Mcan

#define HWCAN_MAX_FILTERS   32
#define HWCAN_RX_FIFO_SIZE   8
#define HWCAN_TX_FIFO_SIZE   8

typedef struct hwcan_rx_fifo_t
{
	__IO uint32_t  ID;    // ID
	__IO uint32_t  DLCTS; // Length Code + TimeStamp
	__IO uint32_t  DATAL; // DATA
	__IO uint32_t  DATAH;
//
} hwcan_rx_fifo_t;

typedef struct hwcan_tx_fifo_t
{
	__IO uint32_t  IDFL;  // ID + Flags
	__IO uint32_t  DLC;   // Length code
	__IO uint32_t  DATAL; // data low
	__IO uint32_t  DATAH; // data high
//
} hwcan_tx_fifo_t;

class THwCan_atsam : public THwCan_pre
{
public: // mandatory
	bool HwInit(int adevnum);

	void Enable();
	bool Enabled();

	void HandleTx();
	void HandleRx();

	void AcceptListClear();
	void AcceptAdd(uint16_t cobid, uint16_t amask);

	bool IsBusOff();
	bool IsWarning();

public:
	HW_CAN_REGS *      regs = nullptr;
	uint8_t            filtercnt = 0;

public:
	// CAN Message Memory

	// you might need to align the THwCan instance to 256 Bytes in order to ensure these data
	// within the same 64k Address Range, because on ATSAME70 they share the upper 16 Address bits

	__IO uint32_t      stdfilters[HWCAN_MAX_FILTERS];
	hwcan_rx_fifo_t    rxfifo[HWCAN_RX_FIFO_SIZE];
	hwcan_tx_fifo_t    txfifo[HWCAN_RX_FIFO_SIZE];
};

#define HWCAN_IMPL THwCan_atsam

#endif

#endif /* HWCAN_ATSAM_H_ */
