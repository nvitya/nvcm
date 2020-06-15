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
 *  file:     hwusbctrl_atsam.cpp
 *  brief:    ATSAM USB Controller
 *  version:  1.00
 *  date:     2018-12-11
 *  authors:  nvitya
*/

#include "platform.h"

#include "string.h"
#include <stdio.h>
#include <stdarg.h>
#include <hwusbctrl.h>
#include "traces.h"

#if defined(UDP_CSR_TXPKTRDY)

#define UDP_REG_NO_EFFECT_1_ALL (UDP_CSR_RX_DATA_BK0 | UDP_CSR_RX_DATA_BK1 | UDP_CSR_STALLSENT | UDP_CSR_RXSETUP | UDP_CSR_TXCOMP)

// the endpoint control and status registers have special handling needs

void udp_ep_csreg_bit_clear(volatile uint32_t * preg, uint32_t amask)
{
	uint32_t tmp = *preg;
	tmp |= UDP_REG_NO_EFFECT_1_ALL;
	tmp &= ~amask;
	*preg = tmp;
	int i = 0;
	while (*preg & amask)
	{
		++i;
		if (i > 3)  break;
	}
}

void udp_ep_csreg_bit_set(volatile uint32_t * preg, uint32_t amask)
{
	uint32_t tmp = *preg;
	tmp |= UDP_REG_NO_EFFECT_1_ALL;
	tmp |= amask;
	*preg = tmp;
	int i = 0;
	while ((*preg & amask) == 0)
	{
		++i;
		if (i > 3)  break;
	}
}

bool THwUsbEndpoint_atsam::ConfigureHwEp()
{
	if (!usbctrl)
	{
		return false;
	}

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

	return true;
}

int THwUsbEndpoint_atsam::ReadRecvData(void * buf, uint32_t buflen)
{
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
}

int THwUsbEndpoint_atsam::StartSendData(void * buf, unsigned len)
{
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
}

void THwUsbEndpoint_atsam::SendAck()
{
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
}

void THwUsbEndpoint_atsam::Nak()
{
}

void THwUsbEndpoint_atsam::EnableRecv()
{
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
}

void THwUsbEndpoint_atsam::DisableRecv()
{
	udp_ep_csreg_bit_set(csreg, UDP_CSR_FORCESTALL);
}

void THwUsbEndpoint_atsam::StopSend()
{
	udp_ep_csreg_bit_clear(csreg, UDP_CSR_TXCOMP | UDP_CSR_TXPKTRDY);
}

void THwUsbEndpoint_atsam::FinishSend()
{
	udp_ep_csreg_bit_clear(csreg, UDP_CSR_TXCOMP);
}

void THwUsbEndpoint_atsam::Stall()
{
	if (iscontrol)
	{
		// the DIR bit must be set before the RXSETUP is cleared !
		udp_ep_csreg_bit_set(csreg, UDP_CSR_DIR);
		if (*csreg & UDP_CSR_RXSETUP)
		{
			udp_ep_csreg_bit_clear(csreg, UDP_CSR_RXSETUP);
		}
	}

	udp_ep_csreg_bit_set(csreg, UDP_CSR_FORCESTALL);
}

/************************************************************************************************************
 * THwUsbCtrl_atsam
 ************************************************************************************************************/

bool THwUsbCtrl_atsam::InitHw()
{
	regs = nullptr;

	unsigned perid = ID_UDP;
	if (perid < 32)
	{
		PMC->PMC_PCER0 = (1 << perid);
	}
	else
	{
		PMC->PMC_PCER1 = (1 << (perid-32));
	}

	regs = UDP;

  // the USB clock (48 MHz) is already prepared in the hwclkctrl

	// enable USB clock
#if defined(PMC_SCER_UDP)
	PMC->PMC_SCER |= PMC_SCER_UDP;
#elif defined(PMC_SCER_USBCLK)
	PMC->PMC_SCER |= PMC_SCER_USBCLK;
#elif defined(PMC_SCER_UOTGCLK)
	PMC->PMC_SCER |= PMC_SCER_UOTGCLK;
#endif

	DisableIrq();

	// clear clearable interrupts
	regs->UDP_ICR = UDP_ISR_RXSUSP | UDP_ISR_RXRSM | UDP_ISR_EXTRSM | UDP_ISR_SOFINT | UDP_ISR_ENDBUSRES | UDP_ISR_WAKEUP;

	irq_mask = 0xFF | UDP_ISR_ENDBUSRES;

  // clear device address:
  regs->UDP_FADDR = 0;
  regs->UDP_GLB_STAT = 0; //UDP_GLB_STAT_RMWUPE | UDP_GLB_STAT_ESR;

  regs->UDP_IER = irq_mask;

  ResetEndpoints();

	return true;
}

void THwUsbCtrl_atsam::ResetEndpoints()
{
	// reset all endpoints
	regs->UDP_RST_EP = 0xFF;
	if (regs->UDP_RST_EP) { } // some sync
	if (regs->UDP_RST_EP) { } // some sync
	if (regs->UDP_RST_EP) { } // some sync
	regs->UDP_RST_EP = 0x00;
	if (regs->UDP_RST_EP) { } // some sync
	if (regs->UDP_RST_EP) { } // some sync
	if (regs->UDP_RST_EP) { } // some sync
}

void THwUsbCtrl_atsam::SetPullUp(bool aenable)
{
	regs->UDP_TXVC = 0
		| (0 << 8)  // 0 = transceiver enabled, 1 = transceived disabled
		| (1 << 9)  // enable pullup
	;
}

void THwUsbCtrl_atsam::DisableIrq()
{
	//{  /* regs->CNTR &= ~irq_mask; */ }
}

void THwUsbCtrl_atsam::EnableIrq()
{
	//{  /* regs->CNTR |=  irq_mask; */ }
}

void THwUsbCtrl_atsam::SetDeviceAddress(uint8_t aaddr)
{
	regs->UDP_GLB_STAT &= ~UDP_GLB_STAT_FADDEN;
  regs->UDP_FADDR &= UDP_FADDR_FEN;

  regs->UDP_FADDR = (aaddr & 0x7F);
  regs->UDP_FADDR |= UDP_FADDR_FEN;
	regs->UDP_GLB_STAT |= UDP_GLB_STAT_FADDEN;
}

void THwUsbCtrl_atsam::HandleIrq()
{
	//source:  HAL_PCD_IRQHandler

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

			if (epreg & UDP_CSR_TXCOMP)
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

#if 0

	if (regs->UDP_ISR & UDP_ISR_WAKEUP)
	{
		TRACE("USB WKUP, ISR=%04X\r\n", regs->UDP_ISR);
		regs->UDP_ICR = (UDP_ISR_RXRSM | UDP_ISR_EXTRSM | UDP_ISR_WAKEUP);
		if (regs->UDP_ISR) { }
		if (regs->UDP_ISR) { }
		if (regs->UDP_ISR) { }
		regs->UDP_GLB_STAT &= ~(UDP_GLB_STAT_RMWUPE);
		regs->UDP_GLB_STAT |=  (UDP_GLB_STAT_ESR);
		regs->UDP_ICR = UDP_ISR_RXSUSP;
	}

	if (regs->UDP_ISR & UDP_ISR_RXRSM)
	{
		TRACE("USB RESUME, ISTR=%04X\r\n", regs->UDP_ISR);
		regs->UDP_ICR = (UDP_ISR_RXRSM | UDP_ISR_EXTRSM | UDP_ISR_WAKEUP);
		regs->UDP_GLB_STAT &= ~(UDP_GLB_STAT_ESR);
	  regs->UDP_IER = ((0x3F << 8) | 0xFF); // enable all special interrupts + endpoint interrupts
		regs->UDP_ICR = UDP_ISR_RXSUSP;
	}


	if (regs->UDP_ISR & UDP_ISR_RXSUSP)
	{
		TRACE("USB SUSP, ISTR=%04X\r\n", regs->UDP_ISR);
		//regs->UDP_ICR = (UDP_ISR_RXRSM | UDP_ISR_EXTRSM | UDP_ISR_WAKEUP);
		regs->UDP_GLB_STAT |= (UDP_GLB_STAT_RMWUPE);
		regs->UDP_ICR = UDP_ISR_RXSUSP;
	}


	if (regs->UDP_ISR & UDP_ISR_SOFINT)
	{
		//TRACE("USB SOF, ISTR=%04X\r\n", regs->UDP_ISR);
		regs->UDP_ICR = UDP_ISR_SOFINT;
	}

#endif

	regs->UDP_ICR = 0xFF00;
}


#endif
