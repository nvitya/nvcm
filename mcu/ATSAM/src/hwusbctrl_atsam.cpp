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

#define UDP_REG_NO_EFFECT_1_ALL (UDP_CSR_RX_DATA_BK0 | UDP_CSR_RX_DATA_BK1 | UDP_CSR_STALLSENT | UDP_CSR_RXSETUP | UDP_CSR_TXCOMP)

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

	usbctrl->regs->UDP_IER = (1 << index); // enable the usb interrupt

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

int THwUsbEndpoint_atsam::SendRemaining()
{
	uint16_t  sendlen = tx_remaining_len;
	if (sendlen > maxlen)  sendlen = maxlen;

	uint16_t * psrc = (uint16_t *)(tx_remaining_dataptr);

	for (uint32_t n = 0; n < sendlen; ++n)
	{
		*fiforeg = *tx_remaining_dataptr++;
	}

	tx_remaining_len -= sendlen;

	uint32_t tmp = (*csreg | UDP_REG_NO_EFFECT_1_ALL);

	tmp |= UDP_CSR_TXPKTRDY;
	if (iscontrol)
	{
	 tmp |= UDP_CSR_DIR;  // set the direction too
	}
	*csreg = tmp;

	if (*csreg) { }
	if (*csreg) { }
	if (*csreg) { }

	return sendlen;
}

void THwUsbEndpoint_atsam::SendAck()
{
	uint32_t tmp = (*csreg | UDP_REG_NO_EFFECT_1_ALL);

	tmp |= UDP_CSR_TXPKTRDY;
	if (iscontrol)
	{
	 tmp |= UDP_CSR_DIR;  // set the direction too
	}
	*csreg = tmp;

	if (*csreg) { }
	if (*csreg) { }
	if (*csreg) { }
}

void THwUsbEndpoint_atsam::FinishRecv(bool reenable)
{
	// clear RX flags
	uint32_t tmp = (*csreg | UDP_REG_NO_EFFECT_1_ALL);

	if (tmp & UDP_CSR_RXSETUP)
	{
		tmp &= ~(UDP_CSR_RXSETUP);
	}
	else if (tmp & UDP_CSR_RX_DATA_BK0)
	{
		tmp &= ~(UDP_CSR_RX_DATA_BK0);
	}
	else if (tmp & UDP_CSR_RX_DATA_BK1)
	{
		tmp &= ~(UDP_CSR_RX_DATA_BK1);
	}
	*csreg = tmp;

	if (*csreg) { }
	if (*csreg) { }
	if (*csreg) { }

	if (reenable)
	{
		EnableRecv();
	}
}

void THwUsbEndpoint_atsam::EnableRecv()
{
	uint32_t tmp = (*csreg | UDP_REG_NO_EFFECT_1_ALL);
	tmp &= ~(UDP_CSR_FORCESTALL | UDP_CSR_STALLSENT);
	if (iscontrol)
	{
		tmp &= ~(UDP_CSR_DIR);
	}
	*csreg = tmp;

	if (*csreg) { }
	if (*csreg) { }
	if (*csreg) { }
}

void THwUsbEndpoint_atsam::DisableRecv()
{
	//set_epreg_rx_status(preg, 0);
}

void THwUsbEndpoint_atsam::StopSend()
{
	uint32_t tmp = (*csreg | UDP_REG_NO_EFFECT_1_ALL);
	tmp &= ~(UDP_CSR_TXCOMP);
	*csreg = tmp;

	if (*csreg) { }
	if (*csreg) { }
	if (*csreg) { }

	tx_remaining_len = 0;
}

void THwUsbEndpoint_atsam::FinishSend()
{
	uint32_t tmp = (*csreg | UDP_REG_NO_EFFECT_1_ALL);
	tmp &= ~(UDP_CSR_TXCOMP);
	*csreg = tmp;
}

void THwUsbEndpoint_atsam::Stall()
{
	uint32_t tmp = (*csreg | UDP_REG_NO_EFFECT_1_ALL);
	tmp |= UDP_CSR_FORCESTALL;
	*csreg = tmp;
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
	PMC->PMC_SCER = PMC_SCER_UDP;
#elif defined(PMC_SCER_USBCLK)
	PMC->PMC_SCER = PMC_SCER_USBCLK;
#elif defined(PMC_SCER_UOTGCLK)
	PMC->PMC_SCER = PMC_SCER_UOTGCLK;
#endif

	DisableIrq();

	// clear clearable interrupts
	regs->UDP_ICR = UDP_ISR_RXSUSP | UDP_ISR_RXRSM | UDP_ISR_EXTRSM | UDP_ISR_SOFINT | UDP_ISR_ENDBUSRES | UDP_ISR_WAKEUP;

  // clear device address:
  regs->UDP_FADDR = 0;
  regs->UDP_GLB_STAT = UDP_GLB_STAT_RMWUPE | UDP_GLB_STAT_ESR;

  regs->UDP_IER = (0x3F << 8); // enable all special interrupts

  ResetEndpoints();

	return true;
}

void THwUsbCtrl_atsam::HandleIrq()
{
	//source:  HAL_PCD_IRQHandler

	uint32_t isr = regs->UDP_ISR;

	if (isr & 0xFF)  // some endpoint interrupt ?
	{
		TRACE("Endpoint interrupt: ISR=%08X\r\n", isr);

		TRACE_FLUSH();

		__BKPT();

#if 0
		// Endpoint transfer finished.
		//PCD_EP_ISR_Handler(hpcd);

		int epid = (istr & 7);
		bool htod = ((istr & 0x10) != 0);

		//TRACE("EP(%03X) event\r\n", istr & 0x17);

		if (!HandleEpTransferEvent(epid, htod))
		{
			TRACE("Unhandled endpoint: %u, htod=%u\r\n", epid, htod);
		}

#endif

		//strace("CTR on EP(%i), ISRX=%i, EPREG=%04X\r\n", epid, isrx, *epdef->preg);

		//uartx.printf("EPR0 = %04X\r\n", regs->EP0R);

		//uartx.printf("  EP.COUNT_RX = %04X\r\n", epdef->pdesc->COUNT_RX);

		// EPR0 = EA60
		//  EP.COUNT_RX = 8808

		//show_ep_rxdata(0);

//  	__BKPT(0);

	}

	if (regs->UDP_ISR & UDP_ISR_ENDBUSRES)
	{
		TRACE("USB RESET, ISR=%04X\r\n", regs->UDP_ISR);

		// disable address, configured state

		HandleReset();

		// clear the address
		regs->UDP_FADDR = UDP_FADDR_FEN | (0 << 0);  // enable address, set address to 0

	  regs->UDP_GLB_STAT = UDP_GLB_STAT_RMWUPE; // | UDP_GLB_STAT_ESR;
	  regs->UDP_IER = (0x3F << 8); // enable all special interrupts

		regs->UDP_ICR = UDP_ISR_ENDBUSRES;
	}

	if (regs->UDP_ISR & UDP_ISR_WAKEUP)
	{
		TRACE("USB WKUP, ISR=%04X\r\n", regs->UDP_ISR);
		regs->UDP_ICR = (UDP_ISR_RXRSM | UDP_ISR_EXTRSM | UDP_ISR_WAKEUP);
		if (regs->UDP_ISR) { }
		if (regs->UDP_ISR) { }
		if (regs->UDP_ISR) { }
		//regs->UDP_GLB_STAT &= ~(UDP_GLB_STAT_RMWUPE);
		//regs->UDP_GLB_STAT |=  (UDP_GLB_STAT_ESR);
		regs->UDP_ICR = UDP_ISR_RXSUSP;
	}

	if (regs->UDP_ISR & UDP_ISR_RXRSM)
	{
		TRACE("USB RESUME, ISTR=%04X\r\n", regs->UDP_ISR);
		regs->UDP_ICR = (UDP_ISR_RXRSM | UDP_ISR_EXTRSM | UDP_ISR_WAKEUP);
		//regs->UDP_GLB_STAT &= ~(UDP_GLB_STAT_ESR);
		regs->UDP_ICR = UDP_ISR_RXSUSP;
	}


	if (regs->UDP_ISR & UDP_ISR_RXSUSP)
	{
		TRACE("USB SUSP, ISTR=%04X\r\n", regs->UDP_ISR);
		//regs->UDP_ICR = (UDP_ISR_RXRSM | UDP_ISR_EXTRSM | UDP_ISR_WAKEUP);
		//regs->UDP_GLB_STAT |= (UDP_GLB_STAT_RMWUPE);
		regs->UDP_ICR = UDP_ISR_RXSUSP;
	}


	if (regs->UDP_ISR & UDP_ISR_SOFINT)
	{
		//TRACE("USB SOF, ISTR=%04X\r\n", regs->UDP_ISR);
		regs->UDP_ICR = UDP_ISR_SOFINT;
	}
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
  regs->UDP_FADDR = (UDP_FADDR_FEN | (aaddr & 0x7F));
	regs->UDP_GLB_STAT |= UDP_GLB_STAT_FADDEN;

}
