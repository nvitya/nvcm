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
 *     register names are from NXP, so it is advised to read NXP manual (e.g. LPC43xx)
*/

#include <stdio.h>
#include "string.h"
#include <stdarg.h>

#include "hweth.h"
#include "traces.h"
#include "clockcnt.h"

#if defined(ETH_BASE)

bool THwEth_stm32::InitMac(void * prxdesclist, uint32_t rxcnt, void * ptxdesclist, uint32_t txcnt)
{
	uint32_t n;
	uint32_t tmp;

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

	// MAC_CONFIG / MACCR

	tmp = 0
		| (0 <<  2)  // RE: receive enable
		| (0 <<  3)  // TE: transmit enable
		| (0 <<  4)  // DC: Deferral check
		| (0 <<  5)  // BL(2): back-off limit
		| (0 <<  7)  // ACS: Automatic PAD/CRC stripping
		| (1 <<  9)  // RD: Retry disable
		| (1 << 10)  // IPCO: generate frame checksum by hardware
		| (1 << 11)  // DM: Duplex Mode
		| (0 << 12)  // LM: Loopback Mode
		| (0 << 13)  // DO: Receive Own Disable
		| (1 << 14)  // FES: Fast Ethernet, 1 = 100 MBit/s
		| (1 << 15)  // PS:
		| (0 << 16)  // CSD: Carrier Sense Disable
		| (0 << 17)  // IFG(3): Inter-Frame Gap, 3 = 72 Bit times
		| (0 << 22)  // JD: Jabber Disable
		| (0 << 23)  // WD: Watchdog Disable
		| (0 << 25)  // CSTF: CRC Stripping
	;
	if (loopback)        tmp |= (1 << 12);
	if (hw_ip_checksum)  tmp |= (1 << 10);
	regs->MAC_CONFIG = tmp;


	/* Setup filter */

	// MAC_FRAME_FILTER / MACFFR

	tmp = 0
	  | (0 <<  0)  // PR: Promiscous Mode
	  | (0 <<  1)  // HUC: Hash Unicast
	  | (0 <<  2)  // HMC: Hash Multicast
	  | (0 <<  3)  // DAIF: DA Inverse Filtering
	  | (0 <<  4)  // PM: Pass All Multicast
	  | (0 <<  5)  // DBF: Disable Broadcast Frames
	  | (1 <<  6)  // PCF: Pass Control Frames
	  | (0 << 10)  // HPF: Hash or Perfect Filter
	  | (0 << 31)  // RA: Receive All
	;
	if (promiscuous_mode)  tmp |= ((1 << 0) | (1u << 31));
	regs->MAC_FRAME_FILTER = tmp;

	/* Flush transmit FIFO */
	regs->DMA_OP_MODE |= HWETH_DMA_OM_FTF;
	while (regs->DMA_OP_MODE & HWETH_DMA_OM_FTF)
	{
		// wait
	}

	tmp = 0
	  | (0 <<  1)  // SR: Start/stop receive
	  | (1 <<  2)  // OSF: Operate on second frame
	  | (0 <<  3)  // RTC(2): Receive threshold contro
	  | (1 <<  6)  // FUGF: Forward undersized good frames
	  | (1 <<  7)  // FEF: Forward error frames
	  | (0 << 13)  // ST: Start/stop transmission
	  | (0 << 14)  // TTC(3): Transmit threshold control
	  | (0 << 20)  // FTF: Flush transmit FIFO
	  | (1 << 21)  // TSF: Transmit store and forward
	  | (0 << 24)  // DFRF: Disable flushing of received frames
	  | (1 << 25)  // RSF: Receive store and forward
	  | (0 << 26)  // DTCEFD: Dropping of TCP/IP checksum error frames disable
	;
	regs->DMA_OP_MODE = tmp;

	/* Enhanced descriptors, burst length = 1 */
	regs->DMA_BUS_MODE = 0
		| (0  << 26)  // MB: Mixed burst
		| (1  << 25)  // AAB: Address-aligned beats
		| (0  << 24)  // FPM: 4xPBL mode
		| (1  << 23)  // USP: Use separate PBL
		| (32 << 17)  // RDP(6): Rx DMA PBL
		| (1  << 16)  // FB: Fixed burst
		| (0  << 14)  // PM2): Rx Tx priority ratio
		| (32 <<  8)  // PBL(6): Programmable burst length
		| (1  <<  7)  // EDFE: Enhanced descriptor format enable, 1 = use enhanced descriptors
		| (0  <<  2)  // DSL(5): Descriptor skip length
		| (0  <<  1)  // DA: DMA Arbitration
		| (0  <<  0)  // SR: Software reset
	;

	/* Clear all MAC interrupts */
	regs->DMA_STAT = HWETH_DMA_ST_ALL;

	/* Enable MAC interrupts */
	regs->DMA_INT_EN = 0;

	// Save MAC address
	SetMacAddress(&mac_address[0]);

	// initialize with 100M + Full Duplex, but later this will be overridden from the Speed Info of the PHY
	SetSpeed(true);
	SetDuplex(true);

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

	return true;
}

void THwEth_stm32::InitDescList(bool istx, int bufnum, HW_ETH_DMA_DESC * pdesc_list, uint8_t * pbuffer)
{
	int i;
	HW_ETH_DMA_DESC *  pdesc = pdesc_list;

	memset(pdesc_list, 0, bufnum * sizeof(HW_ETH_DMA_DESC));

	for (i = 0; i < bufnum; ++i)
	{
		if (istx)
		{
			// different register usage!
	    pdesc->DES0 = HWETH_DMADES_TCH;
	    if (hw_ip_checksum)
	    {
	    	pdesc->DES0 |= (3 << 22);  // setup HW IP Checksum calculation
	    }
			pdesc->DES1 = 0;
		}
		else
		{
			pdesc->DES0 = 0;    // do not enable it yet because there is no buffer assignment
			pdesc->DES1 = HWETH_DMADES_RCH | 0;   // interrupt enabled
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
				pdesc->DES0 |= HWETH_DMADES_TER;
			}
			else
			{
				pdesc->DES1 |= HWETH_DMADES_RER;
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
	pdesc->DES1 |= datalen;
	pdesc->DES0 = HWETH_DMADES_OWN;  // enable receive on this decriptor
}

bool THwEth_stm32::TryRecv(uint32_t * pidx, void * * ppdata, uint32_t * pdatalen)
{
	if (!(regs->MAC_CONFIG & HWETH_MAC_CFG_RE))
	{
		return false;
	}

	__DSB();

	HW_ETH_DMA_DESC * pdesc = (HW_ETH_DMA_DESC *)regs->DMA_CURHOST_REC_DES;

  while (1)
	{
		uint32_t stat = actual_rx_desc->DES0;
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
			*pdatalen = ((result->DES0 >> 16) & 0x1FFF);
			return true;
		}

		// free this, and go to the next.
		actual_rx_desc->DES0 = HWETH_DMADES_OWN;
		actual_rx_desc = (HW_ETH_DMA_DESC *)actual_rx_desc->B2ADD;

		// restart the dma controller if it was out of secriptors.

		__DSB();
		regs->DMA_REC_POLL_DEMAND = 1;
	}

}

void THwEth_stm32::ReleaseRxBuf(uint32_t idx)
{
	HW_ETH_DMA_DESC *  pdesc = &rx_desc_list[idx];
	pdesc->DES0 = HWETH_DMADES_OWN;
	__DSB();
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
		if ((pdesc->DES0 & HWETH_DMADES_OWN) == 0)
		{
			//TRACE("TX using desc %i\r\n", i);

			// use this descriptor
			pdesc->B1ADD  = (uint32_t) pdata;
			pdesc->DES1   = datalen & 0x0FFF;
			pdesc->DES0 |= (HWETH_DMADES_OWN | (3 << 28));  // set First + Last descriptor as well

			// Tell DMA to poll descriptors to start transfer
			__DSB(); // required on Cortex-M7
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
	regs->DMA_OP_MODE |= HWETH_DMA_OM_ST | HWETH_DMA_OM_SR;

	// Start receive polling
	__DSB();
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

void THwEth_stm32::SetupMii(uint32_t div, uint8_t addr)
{
	/* Save clock divider and PHY address in MII address register */
	phy_config = HWETH_MAC_MIIA_PA(addr) | HWETH_MAC_MIIA_CR(div);
}

bool THwEth_stm32::IsMiiBusy()
{
	return (regs->MAC_MII_ADDR & HWETH_MAC_MIIA_GB) ? true : false;
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

void THwEth_stm32::NsTimeStart()
{
}

uint64_t THwEth_stm32::NsTimeRead()
{
	unsigned pm = __get_PRIMASK();  // save interrupt disable status
	__disable_irq();

  uint64_t result = 0;

 	__set_PRIMASK(pm); // restore interrupt disable status
	return result;
}

uint64_t THwEth_stm32::GetTimeStamp(uint32_t idx) // must be called within 2 s to get the right upper 32 bit
{
	return 0;
}


#endif
