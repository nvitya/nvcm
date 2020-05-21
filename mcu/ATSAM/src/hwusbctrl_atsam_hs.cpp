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
 *  file:     hwusbctrl_atsam_hs.cpp
 *  brief:    ATSAM USB HS Controller
 *  version:  1.00
 *  date:     2020-04-13
 *  authors:  nvitya
*/

#include "platform.h"

#if defined(USBHS)

#include "string.h"
#include <stdio.h>
#include <stdarg.h>
#include <hwusbctrl.h>
#include "traces.h"
#include "clockcnt.h"

#define USBHS_RAM_ADDR  0xA0100000

bool THwUsbEndpoint_atsam_hs::ConfigureHwEp()
{
	if (!usbctrl)
	{
		return false;
	}

	cfg_reg = &usbctrl->regs->USBHS_DEVEPTCFG[index];
	isr_reg = &usbctrl->regs->USBHS_DEVEPTISR[index];
	icr_reg = &usbctrl->regs->USBHS_DEVEPTICR[index];
	imr_reg = &usbctrl->regs->USBHS_DEVEPTIMR[index];
	ier_reg = &usbctrl->regs->USBHS_DEVEPTIER[index];
	idr_reg = &usbctrl->regs->USBHS_DEVEPTIDR[index];

	fifo_reg = (volatile uint8_t *)(USBHS_RAM_ADDR + 0x8000 * index);

	uint8_t epsize;
	if      (maxlen <=    8)  epsize = 0;
	else if (maxlen <=   16)  epsize = 1;
	else if (maxlen <=   32)  epsize = 2;
	else if (maxlen <=   64)  epsize = 3;
	else if (maxlen <=  128)  epsize = 4;
	else if (maxlen <=  256)  epsize = 5;
	else if (maxlen <=  512)  epsize = 6;
	else          /* 1024 */  epsize = 7;

	uint8_t eptype;
	if      ((attr & HWUSB_EP_TYPE_MASK) == HWUSB_EP_TYPE_INTERRUPT)  	eptype = 3;
	else if ((attr & HWUSB_EP_TYPE_MASK) == HWUSB_EP_TYPE_BULK)     		eptype = 2;
	else if ((attr & HWUSB_EP_TYPE_MASK) == HWUSB_EP_TYPE_ISO)         	eptype = 1;
	else                                 /* HWUSB_EP_TYPE_CONTROL */  	eptype = 0;

	uint8_t epdir = (dir_htod ? 0 : 1);

	uint32_t cfg = 0
		| (1      <<  1)  // ALLOC
		| (0      <<  2)  // EPBK(2): 0 = single bank
		| (epsize <<  4)  // EPSIZE(3)
		| (epdir  <<  8)  // EPDIR: 0 = out (dev rx), 1 = in (dev tx)
		| (0      <<  9)  // AUTOSW: 0 = automatic bank switching disabled
		| (eptype << 11)  // EPTYPE(2)
		| (0      << 13)  // NBTRANS(2): transaction count for isochronous EPs
	;

	*cfg_reg = cfg;

	if (0 == (*isr_reg & USBHS_DEVEPTISR_CFGOK))
	{
		TRACE("EP(%i) config error\r\n", index);
		return false;
	}

	usbctrl->regs->USBHS_DEVEPT |= (1 << index); // enable the endpoint

	*idr_reg = 0x7F;
	*icr_reg = (USBHS_DEVEPTIER_RXSTPES | USBHS_DEVEPTIER_RXOUTES);
	*ier_reg = (USBHS_DEVEPTIER_RXSTPES | USBHS_DEVEPTIER_RXOUTES);

	usbctrl->irq_mask |= (1 << (12 + index));

	return true;
}

int THwUsbEndpoint_atsam_hs::ReadRecvData(void * buf, uint32_t buflen)
{
	uint32_t tmp = *isr_reg;

	if ((tmp & (USBHS_DEVEPTISR_RXOUTI | USBHS_DEVEPTISR_RXSTPI)) == 0)
	{
		// no data to receive
		return 0;
	}

	uint32_t cnt = ((tmp >> 20) & 0x7FF);

	if (buflen < cnt)
	{
		return USBERR_BUFFER_TOO_SMALL;
	}

	uint8_t * psrc = (uint8_t *)fifo_reg;
	uint8_t * pdst = (uint8_t *)buf;
	uint8_t * pend = pdst + cnt;
	while (pdst < pend)
	{
		*pdst++ = *psrc++;
	}

	return cnt;
}

int THwUsbEndpoint_atsam_hs::StartSendData(void * buf, unsigned len)
{
	int sendlen = len;
	if (sendlen > maxlen)  sendlen = maxlen;

	if (iscontrol)
	{
		if (*isr_reg & (USBHS_DEVEPTISR_RXSTPI | USBHS_DEVEPTISR_RXOUTI))  // this must be done before the sending, probably for the direction change
		{
			*icr_reg = (USBHS_DEVEPTISR_RXSTPI | USBHS_DEVEPTISR_RXOUTI)
					;
			__DSB();
		}
	}

	uint8_t * psrc = (uint8_t *)(buf);
	uint8_t * pdst = (uint8_t *)fifo_reg;
	uint8_t * pend = psrc + sendlen;
	while (psrc < pend)
	{
		*pdst++ = *psrc++;
	}

	__DSB();
	__ISB();

	*icr_reg = USBHS_DEVEPTICR_TXINIC; // start sending

	*ier_reg = USBHS_DEVEPTIER_TXINES; // enable send interrupt

	__DSB();

	return sendlen;
}

void THwUsbEndpoint_atsam_hs::SendAck()
{
	if (iscontrol)
	{
		if (*isr_reg & (USBHS_DEVEPTISR_RXSTPI | USBHS_DEVEPTISR_RXOUTI))  // this must be done before the sending, probably for the direction change
		{
			*icr_reg = (USBHS_DEVEPTISR_RXSTPI | USBHS_DEVEPTISR_RXOUTI);
			__DSB();
		}
	}

	*icr_reg = USBHS_DEVEPTICR_TXINIC; // start sending (empty packet)

	*ier_reg = USBHS_DEVEPTIER_TXINES; // enable send interrupt

	__DSB();
}

void THwUsbEndpoint_atsam_hs::Nak()
{
}

void THwUsbEndpoint_atsam_hs::EnableRecv()
{
	if (iscontrol)
	{
		if (*isr_reg & USBHS_DEVEPTISR_RXSTPI)
		{
			*icr_reg = USBHS_DEVEPTISR_RXSTPI;
		}
	}

	if (*isr_reg & USBHS_DEVEPTISR_RXOUTI)
	{
		*icr_reg = USBHS_DEVEPTISR_RXOUTI;
	}

	if (*imr_reg & USBHS_DEVEPTIMR_STALLRQ)
	{
		*idr_reg = USBHS_DEVEPTIDR_STALLRQC;
	}

	__DSB();
}

void THwUsbEndpoint_atsam_hs::DisableRecv()
{
	//udp_ep_csreg_bit_set(csreg, UDP_CSR_FORCESTALL);
}

void THwUsbEndpoint_atsam_hs::StopSend()
{
	//udp_ep_csreg_bit_clear(csreg, UDP_CSR_TXCOMP | UDP_CSR_TXPKTRDY);
}

void THwUsbEndpoint_atsam_hs::FinishSend()
{
	//udp_ep_csreg_bit_clear(csreg, UDP_CSR_TXCOMP);
}

void THwUsbEndpoint_atsam_hs::Stall()
{
	*ier_reg = USBHS_DEVEPTIER_STALLRQS;
}

/************************************************************************************************************
 * THwUsbCtrl_atsam_hs
 ************************************************************************************************************/

bool THwUsbCtrl_atsam_hs::InitHw()
{
	uint32_t tmp;

	regs = nullptr;

	unsigned perid = ID_USBHS;
	if (perid < 32)
	{
		PMC->PMC_PCER0 = (1 << perid);
	}
	else
	{
		PMC->PMC_PCER1 = (1 << (perid-32));
	}

	regs = USBHS;

	// Always authorize asynchrone USB interrupts to exit of sleep mode
	// For SAM USB wake up device except BACKUP mode
	PMC->PMC_FSMR |= PMC_FSMR_USBAL;

	// set device mode + reset USB
	regs->USBHS_CTRL = USBHS_CTRL_UIMOD_DEVICE;

	regs->USBHS_CTRL |= USBHS_CTRL_USBE; // enable the USB
	regs->USBHS_CTRL |= USBHS_CTRL_VBUSHWC;

	tmp = regs->USBHS_DEVCTRL;
	tmp &= ~USBHS_DEVCTRL_LS; // clear low speed force
	//tmp &= ~USBHS_DEVCTRL_SPDCONF_Msk; // normal mode, high speed enabled
	tmp |= USBHS_DEVCTRL_SPDCONF_Msk; // Force Full-Speed mode, disable high-speed
	regs->USBHS_DEVCTRL = tmp;


	// the reference manual suggest to enable USB clock after
	// enabling the USB device

	// Enable the UPLL (480 MHz clock)
	PMC->CKGR_UCKR = CKGR_UCKR_UPLLCOUNT(3) | CKGR_UCKR_UPLLEN;
	while (!(PMC->PMC_SR & PMC_SR_LOCKU))
	{
		// Wait UTMI PLL Lock Status
	}

	PMC->PMC_USB = 0
		| PMC_USB_USBS  // USBS: 1 =  select UPLL for the USB clock
		| (0 <<  8)     // USBDIV(4): 0 = no division
	;

	// enable USB clock
#if defined(PMC_SCER_UDP)
	PMC->PMC_SCER |= PMC_SCER_UDP;
#elif defined(PMC_SCER_USBCLK)
	PMC->PMC_SCER |= PMC_SCER_USBCLK;
#elif defined(PMC_SCER_UOTGCLK)
	PMC->PMC_SCER |= PMC_SCER_UOTGCLK;
#endif


	regs->USBHS_CTRL &= ~USBHS_CTRL_FRZCLK; // un-freeze USB clock

	// Check USB clock

	while (0 == (regs->USBHS_SR & USBHS_SR_CLKUSABLE))
	{
		// wait until the clock is usable
	}

	// enable device interrupts (reset only)
	regs->USBHS_DEVIDR = 0xFFFFF07F;
	irq_mask = USBHS_DEVIER_EORSTES | (1 << 12);

	regs->USBHS_DEVICR = 0xFFFFF07F; // clear pending reset
	regs->USBHS_DEVIER = irq_mask;

	//DisableIrq();

  ResetEndpoints();

	return true;
}

void THwUsbCtrl_atsam_hs::ResetEndpoints()
{
	// reset all endpoints
	regs->USBHS_DEVEPT |= (0x3FF << 16); // reset all endpoints
	if (regs->USBHS_DEVEPT) { } // some sync
	if (regs->USBHS_DEVEPT) { } // some sync
	if (regs->USBHS_DEVEPT) { } // some sync
	regs->USBHS_DEVEPT &= ~(0x3FF << 16); // remove reset
	if (regs->USBHS_DEVEPT) { } // some sync
	if (regs->USBHS_DEVEPT) { } // some sync
	if (regs->USBHS_DEVEPT) { } // some sync

	// disable all endpoints
	regs->USBHS_DEVEPT = 0;
	if (regs->USBHS_DEVEPT) { } // some sync

	// clear allocations
	for (int i = 0; i < HWUSB_MAX_ENDPOINTS; ++i)
	{
		regs->USBHS_DEVEPTCFG[i] = 0;
		if (regs->USBHS_DEVEPTCFG[i]) { } // some sync
	}
}

void THwUsbCtrl_atsam_hs::SetPullUp(bool aenable)
{
	if (aenable)
	{
		regs->USBHS_DEVCTRL &= ~USBHS_DEVCTRL_DETACH;
	}
	else
	{
		regs->USBHS_DEVCTRL |=  USBHS_DEVCTRL_DETACH;
	}
}

void THwUsbCtrl_atsam_hs::DisableIrq()
{
	//{  /* regs->CNTR &= ~irq_mask; */ }
}

void THwUsbCtrl_atsam_hs::EnableIrq()
{
	//{  /* regs->CNTR |=  irq_mask; */ }
}

void THwUsbCtrl_atsam_hs::SetDeviceAddress(uint8_t aaddr)
{
	// the ADDEN and the device address can not be written at the same time
	regs->USBHS_DEVCTRL &= ~USBHS_DEVCTRL_ADDEN;
	regs->USBHS_DEVCTRL &= ~0x7F;
	if (aaddr)
	{
		regs->USBHS_DEVCTRL |= (aaddr & 0x7F);
		regs->USBHS_DEVCTRL |= USBHS_DEVCTRL_ADDEN;
	}
}

void THwUsbCtrl_atsam_hs::HandleIrq()
{
	uint32_t isr = regs->USBHS_DEVISR;

	if (isr & 0x3FF000)  // some endpoint interrupt ?
	{
		//TRACE("[EP IRQ, ISR=%08X]\r\n", isr);

		uint32_t rev_epirq = __RBIT((isr >> 12) & 0x3FF);  // prepare for CLZ
		while (true)
		{
			uint32_t epid = __CLZ(rev_epirq); // returns leading zeros, 32 when the argument = 0
			if (epid >= HWUSB_MAX_ENDPOINTS)  break; // -->

			volatile uint32_t * pepreg   = &regs->USBHS_DEVEPTISR[epid];
			volatile uint32_t * pepicreg = &regs->USBHS_DEVEPTICR[epid];
			uint32_t epreg = *pepreg;
			uint32_t epint = (epreg & regs->USBHS_DEVEPTIMR[epid]);

			TRACE("[EP(%i)=%08X, int=%02X]\r\n", epid, epreg, epint);

			if (epint & USBHS_DEVEPTISR_RXSTPI)
			{
				if (!HandleEpTransferEvent(epid, true))
				{
					// todo: handle error
				}

				if (*pepreg & USBHS_DEVEPTISR_RXSTPI)
				{
					*pepicreg = USBHS_DEVEPTISR_RXSTPI;
				}
			}
			else if (epint & USBHS_DEVEPTISR_RXOUTI)
			{
				if (!HandleEpTransferEvent(epid, true))
				{
					// todo: handle error
				}

				regs->USBHS_DEVEPTIDR[epid] = USBHS_DEVEPTISR_TXINI; // disable TXIN interrupt

				if (*pepreg & USBHS_DEVEPTISR_RXOUTI)
				{
					// this flag can not be cleared !!! ???
					*pepicreg = USBHS_DEVEPTISR_RXOUTI;

					__DSB();

					if (*pepreg & USBHS_DEVEPTISR_RXOUTI)
					{
						TRACE("RXOUTI flag stuck !!\r\n");
#if 0
						regs->USBHS_DEVEPT |= (1 << (16 + epid)); // reset
						if (regs->USBHS_DEVEPT) { } // some sync
						regs->USBHS_DEVEPT &= ~(1 << (16 + epid)); // remove reset
						if (regs->USBHS_DEVEPT) { } // some sync
#endif
					}
				}
			}
			else if (epint & USBHS_DEVEPTISR_TXINI)
			{
				*pepicreg = USBHS_DEVEPTISR_TXINI;
				regs->USBHS_DEVEPTIDR[epid] = USBHS_DEVEPTISR_TXINI; // disable TXIN interrupt

				if (!HandleEpTransferEvent(epid, false))
				{
					// todo: handle error
				}
			}
			else
			{
				TRACE("Unhandled EPINT.\r\n");
			}

			if (*pepreg & USBHS_DEVEPTISR_NAKINI)
			{
				*pepicreg = USBHS_DEVEPTISR_NAKINI;
			}

			if (*pepreg & USBHS_DEVEPTISR_NAKOUTI)
			{
				*pepicreg = USBHS_DEVEPTISR_NAKOUTI;
			}

			if (*pepreg & USBHS_DEVEPTISR_SHORTPACKET)
			{
				*pepicreg = USBHS_DEVEPTISR_SHORTPACKET;
			}

			__DSB();
			__DSB();

			TRACE("  EPS=%08X\r\n", *pepreg);

#if 0
			if (*pepreg & UDP_CSR_STALLSENT)
			{
				udp_ep_csreg_bit_clear(pepreg, UDP_CSR_STALLSENT);
			}
#endif

			rev_epirq &= ~(1 << (31-epid));

			regs->USBHS_DEVICR = (1 << (12 + epid));
		}
	}

	if (isr & USBHS_DEVISR_EORST)  // RESET
	{
		TRACE("USB RESET, ISR=%08X\r\n", isr);

		regs->USBHS_DEVICR = USBHS_DEVICR_EORSTC;

		SetDeviceAddress(0);

		irq_mask = USBHS_DEVIER_EORSTES; // reset the IRQ mask, the EP interrupts will be added later

		HandleReset();

		//regs->USBHS_DEVIDR = 0xFFFFF07F;
		regs->USBHS_DEVIER = irq_mask;
	}

	if (isr & USBHS_DEVISR_SUSP)
	{
		regs->USBHS_DEVICR = USBHS_DEVICR_SUSPC;
	}

	if (isr & USBHS_DEVISR_WAKEUP)
	{
		regs->USBHS_DEVICR = USBHS_DEVICR_WAKEUPC;
	}

	if (isr & USBHS_DEVISR_EORSM)
	{
		regs->USBHS_DEVICR = USBHS_DEVICR_EORSMC;
	}
}


#endif
