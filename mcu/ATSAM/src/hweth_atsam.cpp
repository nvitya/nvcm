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
 *  file:     hweth_atsam.cpp
 *  brief:    ATSAM Ethernet MAC (GMAC)
 *  version:  1.00
 *  date:     2018-05-31
 *  authors:  nvitya
*/

#include <stdio.h>
#include "string.h"
#include <stdarg.h>

#include "platform.h"

#if defined(GMAC)

#include "hweth.h"
#include "traces.h"
#include "clockcnt.h"

bool THwEth_atsam::InitMac(void * prxdesclist, uint32_t rxcnt, void * ptxdesclist, uint32_t txcnt)
{
	uint32_t n;

	unsigned perid = ID_GMAC;

	// Enable the peripheral
	if (perid < 32)
	{
		PMC->PMC_PCER0 = (1 << perid);
	}
	else
	{
		PMC->PMC_PCER1 = (1 << (perid-32));
	}

	delay_us(1000);

	regs = GMAC;

  regs->GMAC_NCR = 0
    | (0 <<  1)  // LB: Loop Back Local
    | (0 <<  2)  // RXEN: Receive Enable
    | (0 <<  3)  // TXEN: Transmit Enable
    | (1 <<  4)  // MPE: Management Port Enable
    | (0 <<  5)  // CLRSTAT: Clear Statistics Registers
    | (0 << 15)  // SRTSM: store timestamp (nanoseconds) instead of CRC
  ;

  regs->GMAC_NCFGR = 0
    | (1 <<  0)  // SPD: 1 = 100 MBit
    | (1 <<  1)  // FD: 1 = full duplex
    | (0 <<  2)  // DNVLAN: 1 = discard VLAN Frames
    | (0 <<  3)  // JFRAME: 1 = enable jumbo frames
  | (1 <<  4)  // CAF: copy all frames (promiscous mode?)
    | (0 <<  5)  // NBC: 1 = do not accept broadcast frames
    | (1 <<  8)  // MAXFS: 1 = Allow 1536 byte frames
    | (1 << 17)  // RFCS: 1 = remove FCS from received frame data
    | (4 << 18)  // CLK(3): MDC clock divisor
  | (0 << 24)  // RXCOEN: 1 = Enable RX Checksum offload (wrong checksum frames will be discarded)
  | (0 << 25)  // EFRHD: Enable Receive in half duplex
  | (1 << 26)  // IRXFCS: Ignore RX FCS
  | (1 << 29)  // RXBP: Receive Bad Preamble
  | (1 << 30)  // IRXER: Ignore IPG GRXER
  ;

  regs->GMAC_DCFGR = 0
  	| (0 << 24)  // DDRP: 1 = Discard Receive Packets when no free descriptors are available
  	| (24 << 16)  // DRBS(8): DMA Receive Buffer Size in 64 byte units, 24 x 64 = 1536
	| (1 << 11)  // TXCOEN: 1 = IP Checksum offload enable for sending
  	| (1 << 10)  // TXPBMS: 4K TX Buffer Memory
  	| (3 <<  8)  // RXBMS(2): 4K RX buffer memory
  	| (0 <<  7)  // ESPA: 0 = LSB endian mode for packet data
  	| (0 <<  6)  // ESMA: 0 = LSB endian mode for descriptor data
  	| (4 <<  0)  // FBLDO(5): DMA Burst (default = 4)
  ;

  SetMacAddress(&mac_address[0]);

	// initialize descriptor lists

	rx_desc_list = (HW_ETH_DMA_DESC *)prxdesclist;
	rx_desc_count = rxcnt;
	tx_desc_list = (HW_ETH_DMA_DESC *)ptxdesclist;
	tx_desc_count = txcnt;

	// Initialize Tx Descriptors list: Chain Mode
	InitDescList(true, tx_desc_count, tx_desc_list);
	regs->GMAC_TBQB = (uint32_t)&tx_desc_list[0];

	// Initialize Rx Descriptors list: Chain Mode
	InitDescList(false, rx_desc_count, rx_desc_list);
	regs->GMAC_RBQB = (uint32_t)&rx_desc_list[0];

	actual_tx_idx = 0;
	actual_rx_desc = &rx_desc_list[0];

	return true;
}

void THwEth_atsam::InitDescList(bool istx, int bufnum, HW_ETH_DMA_DESC * pdesc_list)
{
	int i;
	HW_ETH_DMA_DESC *  pdesc = pdesc_list;

	memset(pdesc_list, 0, bufnum * sizeof(HW_ETH_DMA_DESC));

	for (i = 0; i < bufnum; ++i)
	{
		pdesc->ADDR = (istx ? 0 : 1); // do not assign data yet, set own bit for RX
		pdesc->STATUS = (istx ? (1u << 31) : 0); // set the own bit for the TX

		if (i == bufnum - 1)
		{
			// last descriptor
			if (istx)
			{
			  pdesc->STATUS |= (1 << 30); // set the WRAP bit for the TX desc
			}
			else
			{
			  pdesc->ADDR |= 2; // set the WRAP bit for the RX desc
			}
		}

		++pdesc;
	}
}

void THwEth_atsam::AssignRxBuf(uint32_t idx, void * pdata, uint32_t datalen)
{
	if (idx >= rx_desc_count)  return;

	int i;
	HW_ETH_DMA_DESC *  pdesc = &rx_desc_list[idx];

	pdesc->STATUS = datalen;
	uint32_t wrapbit = (pdesc->ADDR & 2);
	pdesc->ADDR = (uint32_t)pdata | wrapbit; // do not set the own bit (0) = ready to receive
}

bool THwEth_atsam::TryRecv(uint32_t * pidx, void * * ppdata, uint32_t * pdatalen)
{
	__DSB();

	HW_ETH_DMA_DESC * pdesc = rx_desc_list;

	int i = 0;
	while (i < rx_desc_count)
	{
		if (pdesc->ADDR & 1)
		{
			// use this descriptor
			*pidx = i;
			*ppdata = (void *)(pdesc->ADDR & ~0x3);
			*pdatalen = (pdesc->STATUS & 0x1FFF);
			return true;
		}

		++pdesc;
		++i;
	}

	return false;
}

void THwEth_atsam::ReleaseRxBuf(uint32_t idx)
{
	HW_ETH_DMA_DESC *  pdesc = &rx_desc_list[idx];
	pdesc->ADDR &= ~1;
}

bool THwEth_atsam::TrySend(uint32_t * pidx, void * pdata, uint32_t datalen)
{
	// we need to find the next inactive decriptor, otherwise the sending won't happen

	HW_ETH_DMA_DESC * pdesc = &tx_desc_list[actual_tx_idx];
	if (pdesc->STATUS & (1u << 31)) // OWN bit set = the descriptor is not used by the HW
	{
		pdesc->ADDR = (uint32_t)pdata;
		uint32_t tmp = pdesc->STATUS;
		tmp &= 0x40000000; // keep the WRAP bit only
		tmp |= (datalen & 0x1FFF) | (1 << 15); // set length and last, OWN=0
		pdesc->STATUS = tmp;

		__DSB();

		regs->GMAC_NCR |= GMAC_NCR_TSTART; // start the transmission

		*pidx = actual_tx_idx;
		++actual_tx_idx;
		if (actual_tx_idx >= tx_desc_count)  actual_tx_idx = 0;
		return true;
	}
	else
	{
		return false;
	}
}

void THwEth_atsam::Start(void)
{
  regs->GMAC_NCR |= (0
    | (1 <<  2)  // RXEN: Receive Enable
    | (1 <<  3)  // TXEN: Transmit Enable
  );
}

void THwEth_atsam::Stop(void)
{
  regs->GMAC_NCR &= ~(0
    | (1 <<  2)  // RXEN: Receive Enable
    | (1 <<  3)  // TXEN: Transmit Enable
  );
}

void THwEth_atsam::SetMacAddress(uint8_t * amacaddr)
{
	if (amacaddr != &mac_address[0])
	{
		memcpy(&mac_address, amacaddr, 6);
	}

	regs->GMAC_SA[0].GMAC_SAB = ((uint32_t) amacaddr[3] << 24) | ((uint32_t) amacaddr[2] << 16)
					                  | ((uint32_t) amacaddr[1] <<  8) | ((uint32_t) amacaddr[0]);

	regs->GMAC_SA[0].GMAC_SAT = ((uint32_t) amacaddr[5] << 8) | ((uint32_t) amacaddr[4]);
}

void THwEth_atsam::SetSpeed(bool speed100)
{
	if (speed100)
	{
		regs->GMAC_NCFGR |= GMAC_NCFGR_SPD;
	}
	else
	{
		regs->GMAC_NCFGR &= ~GMAC_NCFGR_SPD;
	}
}

void THwEth_atsam::SetDuplex(bool full)
{
	if (full)
	{
		regs->GMAC_NCFGR |= GMAC_NCFGR_FD;
	}
	else
	{
		regs->GMAC_NCFGR &= ~GMAC_NCFGR_FD;
	}
}

void THwEth_atsam::StartMiiWrite(uint8_t reg, uint16_t data)
{
	// it works only with clause 22 mode...
	uint32_t tmp = 0
		| (1 << 30) // CLTTO: 1 = clause 22 mode, 0 = clause 54 mode
		| (1 << 28) // OP(2): 1 = write, 2 = read
		| (phy_address << 23)
		| (reg << 18)
		| (2 << 16)  // WTN(2): must be fix 10 binary
		| (data << 0)
	;
	regs->GMAC_MAN = tmp;
}

void THwEth_atsam::StartMiiRead(uint8_t reg)
{
	// it works only with clause 22 mode...
	uint32_t tmp = 0
		| (1 << 30) // CLTTO: 1 = clause 22 mode, 0 = clause 54 mode
		| (2 << 28) // OP(2): 1 = write, 2 = read
		| (phy_address << 23)
		| (reg << 18)
		| (2 << 16)  // WTN(2): must be fix 10 binary
	;
	regs->GMAC_MAN = tmp;
}

bool THwEth_atsam::IsMiiBusy()
{
	return ((regs->GMAC_NSR & GMAC_NSR_IDLE) ? false : true);
}

void THwEth_atsam::NsTimeStart()
{
}

uint64_t THwEth_atsam::NsTimeRead()
{
	unsigned pm = __get_PRIMASK();  // save interrupt disable status
	__disable_irq();

  uint64_t result = 0;

 	__set_PRIMASK(pm); // restore interrupt disable status
	return result;
}

uint64_t THwEth_atsam::GetTimeStamp(uint32_t idx) // must be called within 2 s to get the right upper 32 bit
{
	return 0;
}


#endif
