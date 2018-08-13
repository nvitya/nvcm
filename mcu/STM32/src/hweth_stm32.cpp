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
 *  file:     hweth_stm32.cpp
 *  brief:    STM32 Ethernet MAC
 *  version:  1.00
 *  date:     2018-05-30
 *  authors:  nvitya
 *  notes:
 *     actually this driver applicable for some other manufacturers like NXP, Infineon
*/

#include <stdio.h>
#include "string.h"
#include <stdarg.h>

#include "hweth.h"
#include "traces.h"
#include "clockcnt.h"

#if defined(ETH_BASE)

bool THwEth_stm32::Init(void * prxdesclist, uint32_t rxcnt, void * ptxdesclist, uint32_t txcnt)
{
	uint32_t n;

	initialized = false;

  // Select RMII Mode, must be done before enabling the hw
	RCC->APB2ENR |= RCC_APB2ENR_SYSCFGEN;
  if (RCC->APB2ENR) { } // read back for delay
  SYSCFG->PMC |= SYSCFG_PMC_MII_RMII_SEL;

  // enable clocks
  RCC->AHB1ENR |= RCC_AHB1ENR_ETHMACEN;
  if (RCC->AHB1ENR) { } // read back for delay
  RCC->AHB1ENR |= RCC_AHB1ENR_ETHMACRXEN;
  if (RCC->AHB1ENR) { } // read back for delay
  RCC->AHB1ENR |= RCC_AHB1ENR_ETHMACTXEN;
  if (RCC->AHB1ENR) { } // read back for delay

	regs = (HW_ETH_REGS *) ETH_BASE;

	/* This should be called prior to IP_ENET_Init. The MAC controller may
	   not be ready for a call to init right away so a small delay should
	   occur after this call. */
	regs->DMA_BUS_MODE |= HWETH_DMA_BM_SWR;
	for (n = 0; n < 1000; ++n) { }

	// start the nanosecond timer and enable timestamping

	regs->MAC_TIMESTP_CTRL = 0x0003; // enable time in fine mode

	regs->SUBSECOND_INCR = 0;

	// start from 0
	regs->NANOSECONDSUPDATE = 0;
	regs->SECONDSUPDATE = 0;

	regs->MAC_TIMESTP_CTRL = 0x0007;

	// the SystemCoreClock must be higher than 100 MHz !!!

	float mhz = SystemCoreClock / 1000000; // this is round
	regs->ADDEND = (100. / mhz) * (float)0x100000000;
	regs->MAC_TIMESTP_CTRL |= 0x0020;  // use the new ADDEND register

	regs->SUBSECOND_INCR = 10;	// increment by 10 if the addend accumulator overflows

	regs->MAC_TIMESTP_CTRL = 0x0103;  // start fine mode, timestamp all frames

	SetupMii(CalcMdcClock(), phy_address);

	/* Enhanced descriptors, burst length = 1 */
	regs->DMA_BUS_MODE = HWETH_DMA_BM_ATDS | HWETH_DMA_BM_PBL(1) | HWETH_DMA_BM_RPBL(1);

	/* Initial MAC configuration for checksum offload, full duplex,
	   100Mbps, disable receive own in half duplex, inter-frame gap of 64-bits */
	regs->MAC_CONFIG = HWETH_MAC_CFG_BL(0) | HWETH_MAC_CFG_IPC | HWETH_MAC_CFG_DM |
			               HWETH_MAC_CFG_DO | HWETH_MAC_CFG_FES | HWETH_MAC_CFG_PS | HWETH_MAC_CFG_IFG(3);

	/* Setup filter */
	regs->MAC_FRAME_FILTER = HWETH_MAC_FF_PR | HWETH_MAC_FF_RA;  // Promiscous mode, receive all

	/* Flush transmit FIFO */
	regs->DMA_OP_MODE = HWETH_DMA_OM_FTF;

	/* Setup DMA to flush receive FIFOs at 32 bytes, service TX FIFOs at 64 bytes */
	regs->DMA_OP_MODE |= HWETH_DMA_OM_RTC(1) | HWETH_DMA_OM_TTC(0);

	/* Clear all MAC interrupts */
	regs->DMA_STAT = HWETH_DMA_ST_ALL;

	/* Enable MAC interrupts */
	regs->DMA_INT_EN = 0;

	// Save MAC address
	SetMacAddress(&mac_address[0]);

	// initialize with 100M + Full Duplex, but later this will be overridden from the Speed Info of the PHY
	AdjustSpeed(HWETH_PHY_SPEEDINFO_100M | HWETH_PHY_SPEEDINFO_FULLDX);

	// Initialize the RMII PHY
	if (!PhyInit())
	{
		TRACE("Error initializing the Ethernet PHY!\r\n");
		return false;
	}

	TRACE("EtherNET PHY initialized, link status = %i.\r\n", (link_up ? 1 : 0));

	// the PHY link might not be up yet !

	// initialize descriptor lists

	rx_desc_list = (HW_ETH_DMA_DESC *)prxdesclist;
	rx_desc_count = rxcnt;
	tx_desc_list = (HW_ETH_DMA_DESC *)ptxdesclist;
	tx_desc_count = txcnt;

	// Initialize Tx Descriptors list: Chain Mode
	InitDescList(true, tx_desc_count, tx_desc_list, nullptr);
	regs->DMA_TRANS_DES_ADDR = (uint32_t)&tx_desc_list[0];

	// Initialize Rx Descriptors list: Chain Mode
	InitDescList(false, rx_desc_count, rx_desc_list, nullptr);
	regs->DMA_REC_DES_ADDR = (uint32_t)&rx_desc_list[0];

	actual_rx_desc = &rx_desc_list[0];

	initialized = true;

	return true;
}

void THwEth_stm32::InitDescList(bool istx, int bufnum, HW_ETH_DMA_DESC * pdesc_list)
{
	int i;
	HW_ETH_DMA_DESC *  pdesc = pdesc_list;

	memset(pdesc_list, 0, bufnum * sizeof(HW_ETH_DMA_DESC));

	for (i = 0; i < bufnum; ++i)
	{
		if (istx)
		{
			// different register usage!
	    pdesc->STATUS = HWETH_DMADES_TCH; // no hardware checksum insertion for EtherCAT!
			pdesc->CTRL = 0;
		}
		else
		{
			pdesc->STATUS = 0; // HWETH_DMADES_OWN;  // do not enable it yet because there is no buffer assignment
			pdesc->CTRL = HWETH_DMADES_RCH | 0;   // interrupt enabled, to disable add ETH_DMARXDESC_DIC
		}

		pdesc->B1ADD = 0; // do not assign data yet
		pdesc->B2ADD = (uint32_t)(pdesc + 1);

		if (i == bufnum - 1)
		{
			// last descriptor
			pdesc->B2ADD = (uint32_t)pdesc_list;  // link back

			// signal descriptor ring, it seems, that this is not really necessary, but does not hurt
			if (istx)
			{
				pdesc->CTRL |= HWETH_DMADES_TER;
			}
			else
			{
				pdesc->CTRL |= HWETH_DMADES_RER;
			}
		}

		++pdesc;
		pbuffer += HWETH_MAX_PACKET_SIZE;
	}
}

void THwEth_stm32::AssignRxBuf(uint32_t idx, void * pdata, uint32_t datalen)
{
	if (idx >= rx_desc_count)  return;

	int i;
	HW_ETH_DMA_DESC *  pdesc = &rx_desc_list[idx];

	pdesc->B1ADD = (uint32_t)pdata;
	pdesc->CTRL = HWETH_DMADES_RCH | datalen;   // interrupt enabled, to disable add ETH_DMARXDESC_DIC
	pdesc->STATUS = HWETH_DMADES_OWN;  // enable receive on this decriptor
}

bool THwEth_stm32::TryRecv(uint32_t * pidx, void * * ppdata, uint32_t * pdatalen)
{
	HW_ETH_DMA_DESC * pdesc = (HW_ETH_DMA_DESC *)regs->DMA_CURHOST_REC_DES;

  while (1)
	{
		uint32_t stat = actual_rx_desc->STATUS;
		if (stat & HWETH_DMADES_OWN)
		{
			// nothing was received.
			if (actual_rx_desc != pdesc)
			{
				// some error, correct it
				actual_rx_desc = (HW_ETH_DMA_DESC *)actual_rx_desc->B2ADD;
				continue;
			}
			return false;
		}

		// check for errors
		if (stat & (1 << 15))  // Error Summary Bit
		{
			++recv_error_count;
		}
		else if ((stat & (3 << 8)) == (3 << 8))  // First + Last Descriptor bit
		{
			// this is ok
			++recv_count;
			HW_ETH_DMA_DESC * result = actual_rx_desc;
			actual_rx_desc = (HW_ETH_DMA_DESC *)actual_rx_desc->B2ADD;
			// resulting
			*pidx = (result - rx_desc_list); // / sizeof(HW_ETH_DMA_DESC);
			*ppdata = (void *)(result->B1ADD);
			*pdatalen = ((result->STATUS >> 16) & 0x1FFF);
			return true;
		}

		// free this, and go to the next.
		actual_rx_desc->STATUS = HWETH_DMADES_OWN;
		actual_rx_desc = (HW_ETH_DMA_DESC *)actual_rx_desc->B2ADD;

		// restart the dma controller if it was out of secriptors.
		regs->DMA_REC_POLL_DEMAND = 1;
	}

}

void THwEth_stm32::ReleaseRxBuf(uint32_t idx)
{
	HW_ETH_DMA_DESC *  pdesc = &rx_desc_list[idx];
	pdesc->STATUS = HWETH_DMADES_OWN;
	regs->DMA_REC_POLL_DEMAND = 1;  // for the case when we were out of descriptors
}

bool THwEth_stm32::TrySend(uint32_t * pidx, void * pdata, uint32_t datalen)
{
	// it is important to know the current descriptor otherwise the transfer won't be started
	// if this descriptor is owned by the CPU the sending is stopped and checks only this entry for the OWN bit change

	HW_ETH_DMA_DESC * pdesc = (HW_ETH_DMA_DESC *)regs->DMA_CURHOST_TRANS_DES;

	int i = 0;
	while (i < tx_desc_count)
	{
		if ((pdesc->STATUS & HWETH_DMADES_OWN) == 0)
		{
			//TRACE("TX using desc %i\n", i);

			// use this descriptor
			pdesc->B1ADD  = (uint32_t) pdata;
			pdesc->CTRL   = datalen & 0x0FFF;
			pdesc->STATUS |= (HWETH_DMADES_OWN | (3 << 28));  // set First + Last descriptor as well

			// Tell DMA to poll descriptors to start transfer
			regs->DMA_TRANS_POLL_DEMAND = 1;

			*pidx = i;
			return true;
		}

		pdesc = (HW_ETH_DMA_DESC *)pdesc->B2ADD;
		++i;
	}

	// no free descriptors

	return false;
}

void THwEth_stm32::Start(void)
{
	// Clear all MAC interrupts
	regs->DMA_STAT = HWETH_DMA_ST_ALL;

	// Enable MAC interrupts
	regs->DMA_INT_EN = HWETH_DMA_IE_RIE | HWETH_DMA_IE_NIE; // enable only receive interrupt (+Normal interrupt enable)

	// Enable packet reception
	regs->MAC_CONFIG |= HWETH_MAC_CFG_RE | HWETH_MAC_CFG_TE;

	// Setup DMA to flush receive FIFOs at 32 bytes, service TX FIFOs at 64 bytes
	regs->DMA_OP_MODE |= HWETH_DMA_OM_ST | HWETH_DMA_OM_SR | HWETH_DMA_OM_FTF | HWETH_DMA_OM_RTC(1) | HWETH_DMA_OM_TTC(0);

	// Start receive polling
	regs->DMA_REC_POLL_DEMAND = 1;
}

void THwEth_stm32::Stop(void)
{
	regs->DMA_INT_EN = 0; // enable only receive interrupt (+Normal interrupt enable)

	regs->DMA_OP_MODE &= ~(HWETH_DMA_OM_ST | HWETH_DMA_OM_SR); // stop transmit and receive
	regs->MAC_CONFIG &= ~(HWETH_MAC_CFG_RE | HWETH_MAC_CFG_TE);
}

uint32_t THwEth_stm32::CalcMdcClock(void)
{
	uint32_t val = SystemCoreClock / 1000000;

	if (val <  20)  return 0; // invalid
	if (val <  35)  return 2;
	if (val <  60)	return 3;
	if (val < 100)	return 0;
	if (val < 150)	return 1;
	if (val < 250)	return 4;
	if (val < 300)	return 5;

	return 0;
}

void THwEth_stm32::SetMacAddress(uint8_t * amacaddr)
{
	memcpy(&mac_address, amacaddr, 6);

	regs->MAC_ADDR0_LOW = ((uint32_t) amacaddr[3] << 24) | ((uint32_t) amacaddr[2] << 16)
			                  | ((uint32_t) amacaddr[1] << 8) | ((uint32_t) amacaddr[0]);

	regs->MAC_ADDR0_HIGH = ((uint32_t) amacaddr[5] << 8) | ((uint32_t) amacaddr[4]);
}

void THwEth_stm32::SetSpeed(bool speed100)
{
	if (speed100)
	{
		regs->MAC_CONFIG |= HWETH_MAC_CFG_FES;
	}
	else
	{
		regs->MAC_CONFIG &= ~HWETH_MAC_CFG_FES;
	}
}

void THwEth_stm32::SetDuplex(bool full)
{
	if (full)
	{
		regs->MAC_CONFIG |= HWETH_MAC_CFG_DM;
	}
	else
	{
		regs->MAC_CONFIG &= ~HWETH_MAC_CFG_DM;
	}
}

void THwEth_stm32::AdjustSpeed(uint16_t aphy_speedinfo)
{
	SetSpeed(aphy_speedinfo & HWETH_PHY_SPEEDINFO_100M);
	SetDuplex(aphy_speedinfo & HWETH_PHY_SPEEDINFO_FULLDX);

	phy_speedinfo_prev = (aphy_speedinfo & HWETH_PHY_SPEEDINFO_MASK);
}

void THwEth_stm32::SetupMii(uint32_t div, uint8_t addr)
{
	/* Save clock divider and PHY address in MII address register */
	phy_config = HWETH_MAC_MIIA_PA(addr) | HWETH_MAC_MIIA_CR(div);
}

void THwEth_stm32::StartMiiWrite(uint8_t reg, uint16_t data)
{
	/* Write value at PHY address and register */
	regs->MAC_MII_ADDR = phy_config | HWETH_MAC_MIIA_GR(reg) | HWETH_MAC_MIIA_W;
	regs->MAC_MII_DATA = (uint32_t) data;
	regs->MAC_MII_ADDR |= HWETH_MAC_MIIA_GB;
}

void THwEth_stm32::StartMiiRead(uint8_t reg)
{
	/* Read value at PHY address and register */
	regs->MAC_MII_ADDR = phy_config | HWETH_MAC_MIIA_GR(reg);
	regs->MAC_MII_ADDR |= HWETH_MAC_MIIA_GB;
}

inline bool THwEth_stm32::IsMiiBusy()
{
	return (regs->MAC_MII_ADDR & HWETH_MAC_MIIA_GB) ? true : false;
}

bool THwEth_stm32::MiiWaitBusy(int amaxms)
{
	int32_t mst = amaxms;
	while (mst > 0)
	{
		if (IsMiiBusy())
		{
			mst--;
			delay_us(1000);
		}
		else
		{
			return true;
		}
	}

	return false;
}

bool THwEth_stm32::MiiWrite(uint8_t reg, uint16_t data) // blocking mii write (for setup only)
{
	if (!MiiWaitBusy(250))  return false;

	StartMiiWrite(reg, data);  // Start Write value for register

	return MiiWaitBusy(250);
}

bool THwEth_stm32::MiiRead(uint8_t reg, uint16_t *data) // blocking mii read (for setup only)
{
	if (!MiiWaitBusy(250))  return false;

	StartMiiRead(reg);  // Start register read

	if (!MiiWaitBusy(250))  return false;

	*data = regs->MAC_MII_DATA;
	return true;
}

bool THwEth_stm32::PhyWaitReset()
{
	uint16_t bcr;
	int i = 0;
	while (1)
	{
		delay_us(1000);
		if (!MiiRead(HWETH_PHY_BCR_REG, &bcr)) 	return false;

		if ((bcr & (HWETH_PHY_BCR_RESET | HWETH_PHY_BCR_POWER_DOWN)) == 0)
		{
			break;
		}
		else
		{
			++i;
			if (i > 400)
			{
				return false;
			}
		}
	}

	return true;
}

bool THwEth_stm32::PhyInit()
{
	uint16_t bsr;

	// we don't reset the PHY always in order to keep the link active and achive a fast device start

	// wait until the reset finishes.
	if (!PhyWaitReset())  return false;

	// status ok, check the PHY ID for checking the communication

	uint16_t id1, id2;
	if ( !MiiRead(HWETH_PHY_PHYID1_REG, &id1) )  return false;
	if ( !MiiRead(HWETH_PHY_PHYID2_REG, &id2) )  return false;
	id2 = (id2 & 0xFFF0);

	if ((id1 == 0x0007) && (id2 == 0xC0F0))
	{
		// LAN8720A (on LPCXpresso)
	}
	else if ((id1 == 0x0007) && (id2 == 0xC130))
	{
		// SMSC8742A (on ST-Nucleo)
	}
	else
	{
		TRACE("Unknown ETH PHY!, id1=%04x, id2=%04x!\r\n", id1, id2);
		return false;
	}

	// read the basic status reg
	if (!MiiRead(HWETH_PHY_BSR_REG, &bsr))  return false;

	TRACE("ETH PHY BSR = %04X\r\n", bsr);

	bool status_error = ((bsr & (HWETH_PHY_BSR_RMT_FAULT | HWETH_PHY_BSR_JABBER_DETECT)) != 0);
	if (status_error)
	{
		TRACE("PHY status error detected!\r\n");
	}

	if ((bsr & HWETH_PHY_BSR_LINK_STATUS) && !status_error)
	{
		link_up = true;

		TRACE("PHY Link was up, no reset.\r\n");

		// get the connection settings
		uint16_t  phy_speedinfo;
		if (!MiiRead(HWETH_PHY_SPEEDINFO_REG, &phy_speedinfo))  return false;

		AdjustSpeed(phy_speedinfo);
	}
	else
	{
		// the link is down or reset required.
		link_up = false;

		TRACE("Resetting ETH PHY...\r\n");

		// issue reset
		if (!MiiWrite(HWETH_PHY_BCR_REG, HWETH_PHY_BCR_RESET))  return false;

		delay_us(20000);

		if (!PhyWaitReset())  return false;

		// start auto-negotiation
		if (!MiiWrite(HWETH_PHY_BCR_REG, HWETH_PHY_BCR_AUTONEG | HWETH_PHY_BCR_RESTART_AUTONEG))  return false;
	}

	return true;
}

void THwEth_stm32::PhyStatusPoll(void)
{
	switch (phy_poll_state)
	{
	default:
	case 0:
		/* Read BMSR to clear faults */
		StartMiiRead(HWETH_PHY_BSR_REG);
		phy_poll_state = 1;
		break;

	case 1:
		if (!IsMiiBusy())
		{
			phy_bsr_value = regs->MAC_MII_DATA;
			StartMiiRead(HWETH_PHY_SPEEDINFO_REG);
			phy_poll_state = 2;
		}
		break;

	case 2:
		if (!IsMiiBusy())
		{
			phy_speedinfo = regs->MAC_MII_DATA;

			uint16_t msi = (phy_speedinfo & HWETH_PHY_SPEEDINFO_MASK);
			if (msi != phy_speedinfo_prev)
			{
				AdjustSpeed(phy_speedinfo);
			}

			// the statuses were read, process them
			bool link_up_cur = ((phy_bsr_value & HWETH_PHY_BSR_LINK_STATUS) != 0);
			if (link_up != link_up_cur)
			{
				if (link_up_cur)
				{
					Start();
				}
				else
				{
					Stop();
				}

				link_up = link_up_cur;
			}

			phy_poll_state = 0;  // repeat the process
		}
		break;
	}
}

#endif
