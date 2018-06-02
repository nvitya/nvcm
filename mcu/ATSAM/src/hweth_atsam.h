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
 *  file:     hweth_atsam.h
 *  brief:    ATSAM Ethernet MAC (GMAC)
 *  version:  1.00
 *  date:     2018-05-31
 *  authors:  nvitya
*/

#ifndef HWETH_ATSAM_H_
#define HWETH_ATSAM_H_

#include "platform.h"

#define HWETH_PRE_ONLY
#include "hweth.h"

#define HW_ETH_REGS  Gmac

typedef struct
{
	uint32_t  ADDR;
	uint32_t  STATUS;
//
} HW_ETH_DMA_DESC;

class THwEth_atsam : public THwEth_pre
{
public:
	void       Start();
	void       Stop();

public:
	void       SetMdcClock(void);
	void       SetMacAddress(uint8_t * amacaddr);
	void       SetSpeed(bool speed100);
	void       SetDuplex(bool full);

	void       AdjustSpeed(uint16_t aphy_speedinfo);

	void       InitDescList(bool istx, int bufnum, HW_ETH_DMA_DESC * pdesc_list);

public:
	uint8_t    phy_poll_state = 0;

	void       PhyStatusPoll(void); // this must be called regularly !

	void       SetupMii(uint32_t div, uint8_t addr);
	void       StartMiiWrite(uint8_t reg, uint16_t data);
	void       StartMiiRead(uint8_t reg);
	bool       IsMiiBusy();

	bool       PhyInit();

	bool       MiiWaitBusy(int amaxms);

	bool       MiiWrite(uint8_t reg, uint16_t data); // blocking mii write (for setup only)
	bool       MiiRead(uint8_t reg, uint16_t * data); // blocking mii read (for setup only)
	bool 			 PhyWaitReset();

public:
	HW_ETH_REGS *      regs = nullptr;

	HW_ETH_DMA_DESC *  rx_desc_list = nullptr;
	HW_ETH_DMA_DESC *  tx_desc_list = nullptr;

	bool               Init(void * prxdesclist, uint32_t rxcnt, void * ptxdesclist, uint32_t txcnt);
	void               AssignRxBuf(uint32_t idx, void * pdata, uint32_t datalen);

	bool               TryRecv(uint32_t * pidx, void * * ppdata, uint32_t * pdatalen);
	void               ReleaseRxBuf(uint32_t idx);
	bool               TrySend(uint32_t * pidx, void * pdata, uint32_t datalen);


	HW_ETH_DMA_DESC *  actual_rx_desc;

	uint32_t           phy_config;
	uint16_t           phy_speedinfo;
	uint16_t           phy_speedinfo_prev;
	uint16_t           phy_bsr_value;
};

#define HWETH_IMPL THwEth_atsam

#endif // def HWETH_ATSAM_H_
