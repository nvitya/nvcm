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
 *  file:     hwusbctrl_atsam_v2.cpp
 *  brief:    ATSAM_v2 USB Controller
 *  version:  1.00
 *  date:     2020-05-22
 *  authors:  nvitya
*/

#include "platform.h"

#include "string.h"
#include <stdio.h>
#include <stdarg.h>
#include <hwusbctrl.h>
#include "traces.h"

#if defined(USB)

UsbDeviceDescriptor hwusb_desc_table[USB_MAX_ENDPOINTS]  __attribute__((aligned(32)));

uint8_t hwusb_rx_buffer[USB_RX_BUFFER_SIZE];
uint8_t hwusb_tx_buffer[USB_TX_BUFFER_SIZE];

bool THwUsbEndpoint_atsam_v2::ConfigureHwEp()
{
	uint32_t tmp;

	if (!usbctrl)
	{
		return false;
	}

	regs = &usbctrl->regs->DEVICE.DeviceEndpoint[index];
	rxdesc = &hwusb_desc_table[index].DeviceDescBank[0];
	txdesc = &hwusb_desc_table[index].DeviceDescBank[1];

	if ((attr & HWUSB_EP_TYPE_MASK) == HWUSB_EP_TYPE_INTERRUPT)
	{
		tmp = 4;
	}
	else if ((attr & HWUSB_EP_TYPE_MASK) == HWUSB_EP_TYPE_BULK)
	{
		tmp = 3;
	}
	else if ((attr & HWUSB_EP_TYPE_MASK) == HWUSB_EP_TYPE_ISO)
	{
		tmp = 2;
	}
	else
	{
		tmp = 0x11;  // both directions are used!
	}
	// else control by default

	if (!iscontrol && !dir_htod)
	{
		tmp <<= 4;
	}
	regs->EPCFG.reg = tmp;

	// TODO: allocate memory

#if 0
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
#endif

	return true;
}

int THwUsbEndpoint_atsam_v2::ReadRecvData(void * buf, uint32_t buflen)
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

int THwUsbEndpoint_atsam_v2::StartSendData(void * buf, unsigned len)
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
#endif
}

void THwUsbEndpoint_atsam_v2::SendAck()
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

void THwUsbEndpoint_atsam_v2::Nak()
{
}

void THwUsbEndpoint_atsam_v2::EnableRecv()
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

void THwUsbEndpoint_atsam_v2::DisableRecv()
{
	//udp_ep_csreg_bit_set(csreg, UDP_CSR_FORCESTALL);
}

void THwUsbEndpoint_atsam_v2::StopSend()
{
	//udp_ep_csreg_bit_clear(csreg, UDP_CSR_TXCOMP | UDP_CSR_TXPKTRDY);
}

void THwUsbEndpoint_atsam_v2::FinishSend()
{
	//udp_ep_csreg_bit_clear(csreg, UDP_CSR_TXCOMP);
}

void THwUsbEndpoint_atsam_v2::Stall()
{
	//udp_ep_csreg_bit_set(csreg, UDP_CSR_FORCESTALL);
}

/************************************************************************************************************
 * THwUsbCtrl_atsam_v2
 ************************************************************************************************************/

bool THwUsbCtrl_atsam_v2::InitHw()
{
	regs = USB;

	// setup peripheral clock
	GCLK->PCHCTRL[USB_GCLK_ID].reg = ((1 << 6) | (0 << 0));   // select main clock frequency + enable

	MCLK->AHBMASK.bit.USB_ = 1;
	MCLK->APBBMASK.bit.USB_ = 1;

	// reset
	if (!regs->DEVICE.SYNCBUSY.bit.SWRST)
	{
		if (regs->DEVICE.CTRLA.bit.ENABLE)
		{
			regs->DEVICE.CTRLA.bit.ENABLE = 1;
			while (regs->DEVICE.SYNCBUSY.bit.ENABLE)
			{
				// wait
			}
		}
		regs->DEVICE.CTRLA.bit.SWRST = 1;
	}

	while (regs->DEVICE.SYNCBUSY.bit.SWRST)
	{
		// wait for reset end
	}

	LoadCalibration();

	regs->DEVICE.CTRLA.bit.RUNSTDBY = 1;
	regs->DEVICE.CTRLB.bit.SPDCONF = 0; // 0 = full speed, 1 = low speed
	regs->DEVICE.CTRLB.bit.DETACH = 1;

	// reset endpoints
	regs->DEVICE.DESCADD.reg = (uint32_t)&hwusb_desc_table[0];

	ResetEndpoints();

	// enable the IRQs:
	irq_mask = USB_DEVICE_INTENSET_EORST;
	  //(USB_DEVICE_INTENSET_SOF | USB_DEVICE_INTENSET_EORST | USB_DEVICE_INTENSET_RAMACER
	  //| USB_DEVICE_INTFLAG_LPMSUSP | USB_DEVICE_INTFLAG_SUSPEND); // suspend state IRQ flags

	regs->DEVICE.INTENSET.reg = irq_mask;

	// enable the device
	regs->DEVICE.CTRLA.bit.ENABLE = 1;

	return true;
}

void THwUsbCtrl_atsam_v2::LoadCalibration()
{
  // this code is taken from the ASF library

#define NVM_USB_PAD_TRANSN_POS 32
#define NVM_USB_PAD_TRANSN_SIZE 5
#define NVM_USB_PAD_TRANSP_POS 37
#define NVM_USB_PAD_TRANSP_SIZE 5
#define NVM_USB_PAD_TRIM_POS 42
#define NVM_USB_PAD_TRIM_SIZE 3
	uint32_t pad_transn
	    = (*((uint32_t *)(NVMCTRL_SW0) + (NVM_USB_PAD_TRANSN_POS / 32)) >> (NVM_USB_PAD_TRANSN_POS % 32))
	      & ((1 << NVM_USB_PAD_TRANSN_SIZE) - 1);
	uint32_t pad_transp
	    = (*((uint32_t *)(NVMCTRL_SW0) + (NVM_USB_PAD_TRANSP_POS / 32)) >> (NVM_USB_PAD_TRANSP_POS % 32))
	      & ((1 << NVM_USB_PAD_TRANSP_SIZE) - 1);
	uint32_t pad_trim = (*((uint32_t *)(NVMCTRL_SW0) + (NVM_USB_PAD_TRIM_POS / 32)) >> (NVM_USB_PAD_TRIM_POS % 32))
	                    & ((1 << NVM_USB_PAD_TRIM_SIZE) - 1);
	if (pad_transn == 0 || pad_transn == 0x1F)
	{
		pad_transn = 9;
	}
	if (pad_transp == 0 || pad_transp == 0x1F)
	{
		pad_transp = 25;
	}
	if (pad_trim == 0 || pad_trim == 0x7)
	{
		pad_trim = 6;
	}

	regs->DEVICE.PADCAL.reg = USB_PADCAL_TRANSN(pad_transn) | USB_PADCAL_TRANSP(pad_transp) | USB_PADCAL_TRIM(pad_trim);

	regs->DEVICE.QOSCTRL.bit.CQOS = 3;
	regs->DEVICE.QOSCTRL.bit.DQOS = 3;
}


void THwUsbCtrl_atsam_v2::ResetEndpoints()
{
	memset(&hwusb_desc_table, 0, sizeof(hwusb_desc_table));
	rx_mem_alloc = 0;
	tx_mem_alloc = 0;
}

void THwUsbCtrl_atsam_v2::SetPullUp(bool aenable)
{
	if (aenable)
	{
		regs->DEVICE.CTRLB.bit.DETACH = 0;
	}
	else
	{
		regs->DEVICE.CTRLB.bit.DETACH = 1;
	}
}

void THwUsbCtrl_atsam_v2::DisableIrq()
{
	//{  /* regs->CNTR &= ~irq_mask; */ }
}

void THwUsbCtrl_atsam_v2::EnableIrq()
{
	//{  /* regs->CNTR |=  irq_mask; */ }
}

void THwUsbCtrl_atsam_v2::SetDeviceAddress(uint8_t aaddr)
{
	regs->DEVICE.DADD.reg = ((aaddr & 0x7F) | USB_DEVICE_DADD_ADDEN);
}

void THwUsbCtrl_atsam_v2::HandleIrq()
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

#endif
}

#endif
