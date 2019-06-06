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
 *  file:     hweth.cpp
 *  brief:    Ethernet vendor-independent implementations
 *  version:  1.00
 *  date:     2018-05-30
 *  authors:  nvitya
*/

#include "hweth.h"
#include "traces.h"
#include "clockcnt.h"
#include "platform.h"

bool THwEth::Init(void* prxdesclist, uint32_t rxcnt, void* ptxdesclist, uint32_t txcnt)
{
	initialized = false;

	if (!InitMac(prxdesclist, rxcnt, ptxdesclist, txcnt))
	{
		return false;
	}

	// Initialize the RMII PHY
	if (!PhyInit())
	{
		TRACE("Error initializing the Ethernet PHY at address %i!\r\n", phy_address);
		return false;
	}

	TRACE("EtherNET PHY initialized, link status = %i.\r\n", (link_up ? 1 : 0));

	// the PHY link might not be up yet !

	initialized = true;
	return true;
}

bool THwEth::PhyInit()
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
	else if ((id1 == 0x0022) && (id2 == 0x1560))
	{
		// KSZ8081 (on SAME70 X-Plained)
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

bool THwEth::MiiWaitBusy(int amaxms)
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

	mii_running = false; // won't end...

	return false;
}

bool THwEth::MiiWrite(uint8_t reg, uint16_t data) // blocking mii write (for setup only)
{
	if (!MiiWaitBusy(250))  return false;

	StartMiiWrite(reg, data);  // Start Write value for register

	return MiiWaitBusy(250);
}

bool THwEth::MiiRead(uint8_t reg, uint16_t * data) // blocking mii read (for setup only)
{
	if (!MiiWaitBusy(250))  return false;

	StartMiiRead(reg);  // Start register read

	if (!MiiWaitBusy(250))  return false;

	*data = MiiData();
	return true;
}

bool THwEth::PhyWaitReset()
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


void THwEth::AdjustSpeed(uint16_t aphy_speedinfo)
{
	SetSpeed(aphy_speedinfo & HWETH_PHY_SPEEDINFO_100M);
	SetDuplex(aphy_speedinfo & HWETH_PHY_SPEEDINFO_FULLDX);

	phy_speedinfo_prev = (aphy_speedinfo & HWETH_PHY_SPEEDINFO_MASK);
}

void THwEth::PhyStatusPoll(void)
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
      phy_bsr_value = MiiData();
			StartMiiRead(HWETH_PHY_SPEEDINFO_REG);
			phy_poll_state = 2;
		}
		break;

	case 2:
		if (!IsMiiBusy())
		{
      phy_speedinfo = MiiData();

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

