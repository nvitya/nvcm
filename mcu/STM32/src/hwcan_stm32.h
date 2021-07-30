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
 *  file:     hwcan_stm32.h
 *  brief:    STM32 CAN
 *  version:  1.00
 *  date:     2019-01-12
 *  authors:  nvitya
*/

#ifndef HWCAN_STM32_H_
#define HWCAN_STM32_H_

#include "platform.h"

#if !defined(MCUSF_H7) && (defined(MCAN0CAN1_BASE) || defined(CAN_BASE) || defined(CAN1_BASE) || defined(FDCAN1_BASE))

#define HWCAN_PRE_ONLY
#include "hwcan.h"

#if defined(FDCAN1_BASE)
  #define HWCAN_STM32_FD  1
  #define HW_CAN_REGS     FDCAN_GlobalTypeDef

	typedef struct
	{
		__IO uint32_t  ID;    // ID
		__IO uint32_t  DLCTS; // Length Code + TimeStamp
		__IO uint32_t  DATAL; // DATA
		__IO uint32_t  DATAH;
		__IO uint32_t  DATAX[14];
	//
	} __attribute__((packed)) hwcan_rx_fifo_t;

	typedef struct
	{
		__IO uint32_t  IDFL;  // ID + Flags
		__IO uint32_t  DLC;   // Length code
		__IO uint32_t  DATAL; // data low
		__IO uint32_t  DATAH; // data high
		__IO uint32_t  DATAX[14];
	//
	} __attribute__((packed)) hwcan_tx_fifo_t;

	typedef struct
	{
		__IO uint32_t  IDFL;   // ID + Flags
		__IO uint32_t  DLCTS;  // Length code, Timestamp, MM (mark)
	//
	} __attribute__((packed)) hwcan_txev_fifo_t;

#else
  #define HWCAN_STM32_FD  0
  #define HW_CAN_REGS     CAN_TypeDef
#endif

class THwCan_stm32 : public THwCan_pre
{
public: // mandatory
	bool HwInit(int adevnum);

	void Enable();
	void Disable();
	bool Enabled();

	void HandleTx();
	void HandleRx();

	void AcceptListClear();
	void AcceptAdd(uint16_t cobid, uint16_t amask);

	void SetSpeed(uint32_t aspeed);
	bool IsBusOff();
	bool IsWarning();

	void UpdateErrorCounters();

public:
	HW_CAN_REGS *      regs = nullptr;

	uint8_t            filtercnt = 0;

#if HWCAN_STM32_FD
	uint32_t           canram_base = 0;

	uint32_t *         stdfilters = nullptr;

	hwcan_rx_fifo_t *  rxfifo = nullptr;
	hwcan_tx_fifo_t *  txfifo = nullptr;
	hwcan_txev_fifo_t *  txevfifo = nullptr;

	ALWAYS_INLINE uint32_t           ReadPsr(); // special function to handle the reset on read fields
#endif
};

#define HWCAN_IMPL THwCan_stm32

#endif

#endif /* HWCAN_STM32_H_ */
