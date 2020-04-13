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

bool THwUsbEndpoint_atsam_hs::ConfigureHwEp()
{
	if (!usbctrl)
	{
		return false;
	}

#if 0

	fiforeg = (__IO uint8_t *)&(usbctrl->regs->UDP_FDR[index]);
	csreg = &(usbctrl->regs->UDP_CSR[index]);

	uint32_t tmp;

	tmp = 0
	  | (0 <<  0)  // TXCOMP,rwc: 1 = TX Completed, w0: clear
	  | (0 <<  1)  // RX_DATA_BK0: 1 = RX data received into BK0, w0 = clear
	  | (0 <<  2)  // RXSETUP: 1 = Setup packet is received, w0 = clear
	  | (0 <<  3)  // STALLSENT: 1 = Host has acknowledged the stall
	  | (0 <<  4)  // TXPKTRDY: w1 = New data is written into the FIFO and ready to send
	  | (0 <<  5)  // FORCESTALL: w1 = send stall to host
	  | (0 <<  6)  // RX_DATA_BK1: 1 = RX data received into BK1 (only for ping-pong endpoints)
	  | (0 <<  7)  // DIR: control ep direction, 1 = Device to Host (dtoh), 0 = Host do Device (htod)
	  | (0 <<  8)  // EPTYPE(2): 0 = CTRL, 1 = ISOCHRONOUS, 2 = BULK, 3 = INTERRUPT
	  | (0 << 10)  // EPDIR: 0 = OUT (htod), 1 = IN (dtoh)
	  | (0 << 11)  // DTGLE,ro: data toggle
	  | (1 << 15)  // EPEDS: 1 = enable endpoint, 0 = disable
	  | (0 << 16)  // RXBYTECNT(11),ro:  Number of Bytes Available in the FIFO
	;


	if ((attr & HWUSB_EP_TYPE_MASK) == HWUSB_EP_TYPE_INTERRUPT)
	{
		tmp |= (3 << 8);
	}
	else if ((attr & HWUSB_EP_TYPE_MASK) == HWUSB_EP_TYPE_BULK)
	{
		tmp |= (2 << 8);
	}
	else if ((attr & HWUSB_EP_TYPE_MASK) == HWUSB_EP_TYPE_ISO)
	{
		tmp |= (1 << 8);
	}
	// else control by default

	if (!iscontrol && !dir_htod)
	{
		tmp |= (1 << 10);
	}

	*csreg = tmp;

	//usbctrl->regs->UDP_IER = (1 << index); // enable the usb interrupt

#endif

	return true;
}

int THwUsbEndpoint_atsam_hs::ReadRecvData(void * buf, uint32_t buflen)
{
#if 0

	uint32_t tmp = *csreg;

	if ((tmp & (UDP_CSR_RX_DATA_BK0 | UDP_CSR_RX_DATA_BK1 | UDP_CSR_RXSETUP)) == 0)
	{
		// no data to receive
		return 0;
	}

	uint32_t cnt = ((tmp >> 16) & 0x7FF);

	if (buflen < cnt)
	{
		return USBERR_BUFFER_TOO_SMALL;
	}

	uint8_t * pdst = (uint8_t *)buf;
	for (unsigned i = 0; i < cnt; ++i)
	{
		*pdst++ = *fiforeg;
	}

	return cnt;
#else
	return 0;
#endif
}

int THwUsbEndpoint_atsam_hs::StartSendData(void * buf, unsigned len)
{
#if 0
	int sendlen = len;
	if (sendlen > maxlen)  sendlen = maxlen;

	if (iscontrol)
	{
		// the DIR bit must be set before the RXSETUP is cleared !
		udp_ep_csreg_bit_set(csreg, UDP_CSR_DIR);
		if (*csreg & UDP_CSR_RXSETUP)
		{
			udp_ep_csreg_bit_clear(csreg, UDP_CSR_RXSETUP);
		}
	}

	uint8_t * psrc = (uint8_t *)(buf);

	for (uint32_t n = 0; n < sendlen; ++n)
	{
		*fiforeg = *psrc++;
	}

	udp_ep_csreg_bit_set(csreg, UDP_CSR_TXPKTRDY);

	if (*csreg & UDP_CSR_TXCOMP)
	{
		udp_ep_csreg_bit_clear(csreg, UDP_CSR_TXCOMP);
	}

	return sendlen;
#else
	return 0;
#endif
}

void THwUsbEndpoint_atsam_hs::SendAck()
{
#if 0
	if (iscontrol)
	{
		// the DIR bit must be set before the RXSETUP is cleared !
		udp_ep_csreg_bit_set(csreg, UDP_CSR_DIR);
		if (*csreg & UDP_CSR_RXSETUP)
		{
			udp_ep_csreg_bit_clear(csreg, UDP_CSR_RXSETUP);
		}
	}

	udp_ep_csreg_bit_set(csreg, UDP_CSR_TXPKTRDY);

	if (*csreg & UDP_CSR_TXCOMP)
	{
		udp_ep_csreg_bit_clear(csreg, UDP_CSR_TXCOMP);
	}
#endif
}

void THwUsbEndpoint_atsam_hs::Nak()
{
}

void THwUsbEndpoint_atsam_hs::EnableRecv()
{
#if 0
	if (iscontrol)
	{
#if 0
		// the DIR bit must be set before the RXSETUP is cleared !
		if (*csreg & UDP_CSR_DIR)
		{
			udp_ep_csreg_bit_clear(csreg, UDP_CSR_DIR);
		}
#endif

		if (*csreg & UDP_CSR_RXSETUP)
		{
			udp_ep_csreg_bit_clear(csreg, UDP_CSR_RXSETUP);
		}
	}

#if 0
	if (*csreg & UDP_CSR_TXCOMP)
	{
		udp_ep_csreg_bit_clear(csreg, UDP_CSR_TXCOMP);
	}
#endif

	if (*csreg & (UDP_CSR_FORCESTALL | UDP_CSR_STALLSENT))
	{
		udp_ep_csreg_bit_clear(csreg, UDP_CSR_FORCESTALL | UDP_CSR_STALLSENT);
	}
#endif
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
	//udp_ep_csreg_bit_set(csreg, UDP_CSR_FORCESTALL);
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

	// reference recommends enable the USB before enabling its clock

	regs->USBHS_CTRL |= USBHS_CTRL_USBE;
	regs->USBHS_CTRL &= ~USBHS_CTRL_FRZCLK;

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

	// Always authorize asynchrone USB interrupts to exit of sleep mode
	// For SAM USB wake up device except BACKUP mode
	PMC->PMC_FSMR |= PMC_FSMR_USBAL;

	// ID pin not used then force device mode
	regs->USBHS_CTRL = USBHS_CTRL_UIMOD_DEVICE;

	tmp = regs->USBHS_DEVCTRL;
	tmp &= ~USBHS_DEVCTRL_LS; // clear low speed force
	tmp &= ~USBHS_DEVCTRL_SPDCONF_Msk; // normal mode, high speed enabled
	regs->USBHS_DEVCTRL = tmp;

	// Check USB clock

	while (0 == (regs->USBHS_SR & USBHS_SR_CLKUSABLE))
	{
		// wait until the clock is usable
	}

	// enable device interrupts (reset only)
	regs->USBHS_DEVIER = (USBHS_DEVIER_EORSTES);
	regs->USBHS_DEVICR = (USBHS_DEVICR_EORSTC); // clear pending reset

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
	regs->USBHS_DEVCTRL &= ~USBHS_DEVCTRL_ADDEN;
	regs->USBHS_DEVCTRL &= ~0x7F;
	regs->USBHS_DEVCTRL |= (aaddr & 0x7F);
	regs->USBHS_DEVCTRL |= USBHS_DEVCTRL_ADDEN;
}

void THwUsbCtrl_atsam_hs::HandleIrq()
{

#if 0

	uint32_t isr = regs->UDP_ISR;

	if (isr & 0xFF)  // some endpoint interrupt ?
	{
		//TRACE("[EP IRQ, ISR=%08X]\r\n", isr);

		// Endpoint transfer finished.
		//PCD_EP_ISR_Handler(hpcd);

		uint32_t rev_epirq = __RBIT(isr & 0xFF);  // prepare for CLZ
		while (true)
		{
			uint32_t epid = __CLZ(rev_epirq); // returns leading zeros, 32 when the argument = 0
			if (epid > 7)  break; // -->

			volatile uint32_t * pepreg = &regs->UDP_CSR[epid];
			uint32_t epreg = *pepreg;
			//TRACE("[EP(%i)=%08X]\r\n", epid, epreg);

			if (epreg & UDP_CSR_RXSETUP)
			{
				if (!HandleEpTransferEvent(epid, true))
				{
					// todo: handle error
				}

				// warning, the RXSETUP bit must be cleared only AFTER setting the DIR bit so the RXSETUP clear
				// is included in the handler routines !
				if (*pepreg & UDP_CSR_RXSETUP)
				{
					udp_ep_csreg_bit_clear(pepreg, UDP_CSR_RXSETUP);
				}
			}
			else if (epreg & UDP_CSR_RX_DATA_BK0)
			{
				if (!HandleEpTransferEvent(epid, true))
				{
					// todo: handle error
				}

				if (*pepreg & UDP_CSR_RX_DATA_BK0)
				{
					udp_ep_csreg_bit_clear(pepreg, UDP_CSR_RX_DATA_BK0);
				}
			}
			else if (epreg & UDP_CSR_RX_DATA_BK1)
			{
				if (!HandleEpTransferEvent(epid, true))
				{
					// todo: handle error
				}

				if (*pepreg & UDP_CSR_RX_DATA_BK1)
				{
					udp_ep_csreg_bit_clear(pepreg, UDP_CSR_RX_DATA_BK1);
				}
			}
			else if (epreg & UDP_CSR_TXCOMP)
			{
				if (!HandleEpTransferEvent(epid, false))
				{
					// todo: handle error
				}
				if (*pepreg & UDP_CSR_TXCOMP)
				{
				  udp_ep_csreg_bit_clear(pepreg, UDP_CSR_TXCOMP);
				}
			}

			if (*pepreg & UDP_CSR_STALLSENT)
			{
				udp_ep_csreg_bit_clear(pepreg, UDP_CSR_STALLSENT);
			}

			rev_epirq &= ~(1 << (31-epid));
		}
	}

	if (regs->UDP_ISR & UDP_ISR_ENDBUSRES)  // RESET
	{
		TRACE("USB RESET, ISR=%04X\r\n", regs->UDP_ISR);

		// disable address, configured state

		HandleReset();

		regs->UDP_ICR = UDP_ISR_ENDBUSRES;

		// clear the address
		//regs->UDP_FADDR = UDP_FADDR_FEN | (0 << 0);  // enable address, set address to 0
	  regs->UDP_GLB_STAT &= ~(UDP_GLB_STAT_FADDEN);
	  regs->UDP_GLB_STAT &= ~(UDP_GLB_STAT_CONFG);

	  regs->UDP_IER = irq_mask;
	}

	regs->UDP_ICR = 0xFF00;

#endif

}


#endif
