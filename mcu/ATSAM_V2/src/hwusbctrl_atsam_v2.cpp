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
#include "atsam_v2_utils.h"
#include "traces.h"
#include "clockcnt.h"

#if defined(USB)

// Warning: inverse bank-ready logic at Bank0 and Bank1 !

#define HWUSB_INTFLAG_TRCPT0   (1 << 0)
#define HWUSB_INTFLAG_TRCPT1   (1 << 1)
#define HWUSB_INTFLAG_TRFAIL0  (1 << 2)
#define HWUSB_INTFLAG_TRFAIL1  (1 << 3)
#define HWUSB_INTFLAG_RXSTP    (1 << 4)
#define HWUSB_INTFLAG_STALL0   (1 << 5)
#define HWUSB_INTFLAG_STALL1   (1 << 6)

UsbDeviceDescriptor hwusb_desc_table[USB_MAX_ENDPOINTS]  __attribute__((aligned(32)));

uint8_t hwusb_rx_buffer[USB_RX_BUFFER_SIZE]  __attribute__((aligned(4)));
uint8_t hwusb_tx_buffer[USB_TX_BUFFER_SIZE]  __attribute__((aligned(4)));

bool THwUsbEndpoint_atsam_v2::ConfigureHwEp()
{
	uint32_t tmp;

	if (!usbctrl)
	{
		return false;
	}

	regs = &(usbctrl->regs->DeviceEndpoint[index]);
	rxdesc = &(hwusb_desc_table[index].DeviceDescBank[0]);
	txdesc = &(hwusb_desc_table[index].DeviceDescBank[1]);

	// correct the maxlen

	uint32_t lencode;

	if      (maxlen > 512)  lencode = 7;  // 513 .. 1023
	else if (maxlen > 256)  lencode = 6;  // 257 ..  512
	else if (maxlen > 128)  lencode = 5;  // 129 ..  256
	else if (maxlen >  64)  lencode = 4;  //  65 ..  128
	else if (maxlen >  32)  lencode = 3;  //  33 ..   64
	else if (maxlen >  16)  lencode = 2;  //  17 ..   32
	else if (maxlen >   8)  lencode = 1;  //   9 ..   16
	else                    lencode = 0;  //   0 ..    8

	maxlen = (8 << lencode);


	uint16_t htod_len;
	uint16_t dtoh_len;

	if (dir_htod)
	{
		htod_len = maxlen;
		dtoh_len = 0;
	}
	else
	{
		htod_len = 0;
		dtoh_len = maxlen;
	}

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
	else // else control by default
	{
		htod_len = maxlen;
		dtoh_len = maxlen;
		tmp = 0x11;  // both directions are used!
	}

	if (!iscontrol && !dir_htod)
	{
		tmp <<= 4;
	}
	regs->EPCFG.reg = tmp;

	// allocate memory

	if (htod_len + usbctrl->rx_mem_alloc > USB_RX_BUFFER_SIZE)
	{
		// does not fit into the memory!
		return false;
	}

	if (dtoh_len + usbctrl->tx_mem_alloc > USB_TX_BUFFER_SIZE)
	{
		// does not fit into the memory!
		return false;
	}

	// ok, prepare the descriptors
	memset(rxdesc, 0, sizeof(UsbDeviceDescBank));
	memset(txdesc, 0, sizeof(UsbDeviceDescBank));
	rxmem = nullptr;
	txmem = nullptr;

	regs->EPSTATUSCLR.reg = 0xFF;

	if (htod_len > 0)
	{
		rxmem = &hwusb_rx_buffer[usbctrl->rx_mem_alloc];
		rxdesc->ADDR.reg = (uint32_t)rxmem;
		rxdesc->PCKSIZE.reg = 0;
		rxdesc->PCKSIZE.bit.SIZE = lencode;
		rxdesc->PCKSIZE.bit.MULTI_PACKET_SIZE = (maxlen << 14);
		usbctrl->rx_mem_alloc += htod_len;
	}

	if (dtoh_len > 0)
	{
		txmem = &hwusb_tx_buffer[usbctrl->tx_mem_alloc];
		txdesc->ADDR.reg = (uint32_t)txmem;
		txdesc->PCKSIZE.reg = 0;
		txdesc->PCKSIZE.bit.SIZE = lencode;
		usbctrl->tx_mem_alloc += dtoh_len;
	}


	uint8_t intenmask = (HWUSB_INTFLAG_RXSTP | HWUSB_INTFLAG_TRCPT0 | HWUSB_INTFLAG_TRCPT1);
	regs->EPINTENCLR.reg = ~intenmask;
	regs->EPINTENSET.reg = intenmask;

	return true;
}

int THwUsbEndpoint_atsam_v2::ReadRecvData(void * buf, uint32_t buflen)
{
	regs->EPINTFLAG.reg = (HWUSB_INTFLAG_RXSTP | HWUSB_INTFLAG_TRCPT0);

	uint32_t cnt = rxdesc->PCKSIZE.bit.BYTE_COUNT;
	if (cnt)
	{
		if (buflen < cnt)
		{
			return USBERR_BUFFER_TOO_SMALL;
		}

		// optimized copy

		uint32_t cnt32 = (cnt >> 2);
		uint32_t * psrc32 = (uint32_t *)rxmem;
		uint32_t * pdst32 = (uint32_t *)buf;
		uint32_t * pend32 = pdst32 + cnt32;
		while (pdst32 < pend32)
		{
			*pdst32++ = *psrc32++;
		}

		uint32_t cnt8 = cnt & 3;
		uint8_t * psrc8 = (uint8_t *)psrc32;
		uint8_t * pdst8 = (uint8_t *)pdst32;
		uint8_t * pend8 = pdst8 + cnt8;
		while (pdst8 < pend8)
		{
			*pdst8++ = *psrc8++;
		}
	}

	rxdesc->STATUS_BK.reg = 0;
	rxdesc->ADDR.reg = (uint32_t)rxmem;
	rxdesc->PCKSIZE.bit.BYTE_COUNT = 0;
	rxdesc->PCKSIZE.bit.MULTI_PACKET_SIZE = maxlen;

	// BK0RDY = 0: ready to receive
	regs->EPSTATUSCLR.reg = USB_DEVICE_EPSTATUS_BK0RDY | USB_DEVICE_EPSTATUS_STALLRQ(0x3);

	return cnt;
}

// the answer of the device descriptor

int THwUsbEndpoint_atsam_v2::StartSendData(void * buf, unsigned len)
{
	regs->EPSTATUSCLR.reg = USB_DEVICE_EPSTATUS_BK1RDY;
	regs->EPINTFLAG.reg = (HWUSB_INTFLAG_TRCPT1 | HWUSB_INTFLAG_TRFAIL1);

	txdesc->STATUS_BK.reg = 0;

	int sendlen = len;
	if (sendlen > maxlen)  sendlen = maxlen;

	// optimized copy

	uint32_t cnt32 = (sendlen >> 2);
	uint32_t * psrc32 = (uint32_t *)buf;
	uint32_t * pdst32 = (uint32_t *)txmem;
	uint32_t * pend32 = pdst32 + cnt32;
	while (pdst32 < pend32)
	{
		*pdst32++ = *psrc32++;
	}

	uint32_t cnt8 = sendlen & 3;
	uint8_t * psrc8 = (uint8_t *)psrc32;
	uint8_t * pdst8 = (uint8_t *)pdst32;
	uint8_t * pend8 = pdst8 + cnt8;
	while (pdst8 < pend8)
	{
		*pdst8++ = *psrc8++;
	}

	txdesc->ADDR.reg = (uint32_t)txmem;

	txdesc->PCKSIZE.bit.BYTE_COUNT = sendlen;
	txdesc->PCKSIZE.bit.MULTI_PACKET_SIZE = 0;

	regs->EPSTATUSCLR.reg = USB_DEVICE_EPSTATUS_STALLRQ(0x3);

	// BK1RDY = 1: send prepared data
	regs->EPSTATUSSET.reg = USB_DEVICE_EPSTATUS_BK1RDY;

	return sendlen;
}

void THwUsbEndpoint_atsam_v2::SendAck()
{
	txdesc->PCKSIZE.bit.MULTI_PACKET_SIZE = 0;
	txdesc->PCKSIZE.bit.BYTE_COUNT = 0;

	regs->EPSTATUSCLR.reg = USB_DEVICE_EPSTATUS_STALLRQ(0x3);
	regs->EPSTATUSSET.reg = USB_DEVICE_EPSTATUS_BK1RDY;
}

void THwUsbEndpoint_atsam_v2::Nak()
{
}

void THwUsbEndpoint_atsam_v2::EnableRecv()
{
	rxdesc->ADDR.reg = (uint32_t)rxmem;
	rxdesc->PCKSIZE.bit.BYTE_COUNT = 0;
	rxdesc->PCKSIZE.bit.MULTI_PACKET_SIZE = maxlen;

	regs->EPSTATUSCLR.reg = USB_DEVICE_EPSTATUS_BK0RDY | USB_DEVICE_EPSTATUS_STALLRQ(0x3);
}

void THwUsbEndpoint_atsam_v2::DisableRecv()
{
	regs->EPSTATUSSET.reg = USB_DEVICE_EPSTATUS_BK0RDY;
}

void THwUsbEndpoint_atsam_v2::StopSend()
{
	//udp_ep_csreg_bit_clear(csreg, UDP_CSR_TXCOMP | UDP_CSR_TXPKTRDY);
}

void THwUsbEndpoint_atsam_v2::FinishSend()
{
	regs->EPINTFLAG.bit.TRCPT1 = 1;
}

void THwUsbEndpoint_atsam_v2::Stall()
{
	regs->EPSTATUSSET.reg = USB_DEVICE_EPSTATUS_BK0RDY | USB_DEVICE_EPSTATUS_STALLRQ(1);
}

/************************************************************************************************************
 * THwUsbCtrl_atsam_v2
 ************************************************************************************************************/

#define USB_WAKEUP_INT_FLAGS    (USB_DEVICE_INTFLAG_UPRSM | USB_DEVICE_INTFLAG_EORSM | USB_DEVICE_INTFLAG_WAKEUP)
#define USB_SUSPEND_INT_FLAGS   (USB_DEVICE_INTFLAG_SUSPEND)

bool THwUsbCtrl_atsam_v2::InitHw()
{
	regs = &USB->DEVICE;

	// enable USB AHB clock (for the peripheral access)
	MCLK->AHBMASK.bit.USB_ = 1;
	MCLK->APBBMASK.bit.USB_ = 1;

	// setup 48 MHz clock
	atsam2_set_periph_gclk(USB_GCLK_ID, 4); // the GCLK 4 is prepared in the hwclkctrl.cpp to 48 MHz

	// reset
	if (!regs->SYNCBUSY.bit.SWRST)
	{
		if (regs->CTRLA.bit.ENABLE)
		{
			regs->CTRLA.bit.ENABLE = 1;
			while (regs->SYNCBUSY.bit.ENABLE)
			{
				// wait
			}
		}
		regs->CTRLA.bit.SWRST = 1;
	}

	while (regs->SYNCBUSY.bit.SWRST)
	{
		// wait for reset end
	}

	LoadCalibration();

	regs->CTRLA.bit.RUNSTDBY = 1;
	regs->CTRLB.bit.SPDCONF = 0; // 0 = full speed, 1 = low speed
	regs->CTRLB.bit.DETACH = 1;

	// reset endpoints
	regs->DESCADD.reg = (uint32_t)&hwusb_desc_table[0];

	ResetEndpoints();

	// enable the IRQs:
	irq_mask = USB_DEVICE_INTENSET_EORST | USB_WAKEUP_INT_FLAGS | USB_SUSPEND_INT_FLAGS;
	//  | USB_DEVICE_INTENSET_SOF | USB_DEVICE_INTENSET_RAMACER
	//  | USB_DEVICE_INTFLAG_LPMSUSP | USB_DEVICE_INTFLAG_SUSPEND // suspend state IRQ flags
	;

	regs->INTENSET.reg = irq_mask;

	// enable the device
	regs->CTRLA.bit.ENABLE = 1;
	while (regs->SYNCBUSY.bit.ENABLE)
	{
		// wait
	}

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

	regs->PADCAL.reg = USB_PADCAL_TRANSN(pad_transn) | USB_PADCAL_TRANSP(pad_transp) | USB_PADCAL_TRIM(pad_trim);

	regs->QOSCTRL.bit.CQOS = 3;
	regs->QOSCTRL.bit.DQOS = 3;
}


void THwUsbCtrl_atsam_v2::ResetEndpoints()
{
	for (int i = 0; i < USB_MAX_ENDPOINTS; ++i)
	{
		regs->DeviceEndpoint[i].EPCFG.reg = 0;
	}

	memset(&hwusb_desc_table, 0, sizeof(hwusb_desc_table));
	rx_mem_alloc = 0;
	tx_mem_alloc = 0;
}

void THwUsbCtrl_atsam_v2::SetPullUp(bool aenable)
{
	if (aenable)
	{
		regs->CTRLB.bit.DETACH = 0;
	}
	else
	{
		regs->CTRLB.bit.DETACH = 1;
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
	regs->DADD.bit.ADDEN = 0;
	regs->DADD.reg = (aaddr & 0x7F);

	//if (aaddr)  regs->DADD.bit.ADDEN = 1;
	regs->DADD.bit.ADDEN = 1;
}

void THwUsbCtrl_atsam_v2::HandleIrq()
{
	uint32_t epintsum = regs->EPINTSMRY.reg;
	if (epintsum)
	{
		uint32_t rev_epirq = __RBIT(epintsum);
		while (true)
		{
			uint32_t epid = __CLZ(rev_epirq); // returns leading zeros, 32 when the argument = 0
			if (epid >= USB_MAX_ENDPOINTS)  break; // -->

			UsbDeviceEndpoint * pep = &regs->DeviceEndpoint[epid];

			uint8_t epreg = pep->EPINTFLAG.reg;
			//TRACE("[EP(%i)=%02X, EPS=%02X, USB=%04X]\r\n", epid, epreg, pep->EPSTATUS.reg, regs->INTFLAG.reg);

			// the transfer complete must be served for the case it comes togeter with SETUP
			if (epreg & HWUSB_INTFLAG_TRCPT1)
			{
				if (!HandleEpTransferEvent(epid, false))
				{
					// todo: handle error
				}

				if (pep->EPINTFLAG.bit.TRCPT1)		pep->EPINTFLAG.bit.TRCPT1 = 1;
			}

			if (epreg & HWUSB_INTFLAG_RXSTP)
			{
				//test_send_desc();

				if (!HandleEpTransferEvent(epid, true))
				{
					// todo: handle error
				}

				if (pep->EPINTFLAG.bit.RXSTP)		pep->EPINTFLAG.bit.RXSTP = 1;
			}
			else if (epreg & HWUSB_INTFLAG_TRCPT0)
			{
				if (!HandleEpTransferEvent(epid, true))
				{
					// todo: handle error
				}

				if (pep->EPINTFLAG.bit.TRCPT0)		pep->EPINTFLAG.bit.TRCPT0 = 1;
			}

			rev_epirq &= ~(1 << (31-epid));
		}
	}

	uint32_t intflag = (regs->INTFLAG.reg & regs->INTENSET.reg);
	if (intflag & USB_DEVICE_INTFLAG_EORST)
	{
		TRACE("USB RESET %04X\r\n", regs->INTFLAG.reg);

		regs->DeviceEndpoint[0].EPCFG.reg = 0; // disable endpoint 0

		regs->INTFLAG.reg = USB_DEVICE_INTFLAG_EORST; // ack reset
		regs->INTENCLR.reg = USB_WAKEUP_INT_FLAGS;
		regs->INTENSET.reg = USB_SUSPEND_INT_FLAGS;

		// disable address, configured state
	  SetDeviceAddress(0);

		HandleReset();
	}
	else if (intflag & USB_WAKEUP_INT_FLAGS)
	{
		TRACE("USB wakeup\r\n");
		// wakeup
		regs->INTFLAG.reg = USB_WAKEUP_INT_FLAGS;
		regs->INTENCLR.reg = USB_WAKEUP_INT_FLAGS;
		regs->INTENSET.reg = USB_SUSPEND_INT_FLAGS;
	}
	else if (intflag & USB_SUSPEND_INT_FLAGS)
	{
		TRACE("USB suspend\r\n");
		// suspend
		regs->INTFLAG.reg = USB_SUSPEND_INT_FLAGS;
		regs->INTENCLR.reg = USB_SUSPEND_INT_FLAGS;
		regs->INTENSET.reg = USB_WAKEUP_INT_FLAGS;
	}
}

#endif
