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

#define HWUSB_INTFLAG_TRCPT0   (1 << 0)
#define HWUSB_INTFLAG_TRCPT1   (1 << 1)
#define HWUSB_INTFLAG_TRFAIL0  (1 << 2)
#define HWUSB_INTFLAG_TRFAIL1  (1 << 3)
#define HWUSB_INTFLAG_RXSTP    (1 << 4)
#define HWUSB_INTFLAG_STALL0   (1 << 5)
#define HWUSB_INTFLAG_STALL1   (1 << 6)

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

	regs = &usbctrl->regs->DeviceEndpoint[index];
	rxdesc = &hwusb_desc_table[index].DeviceDescBank[0];
	txdesc = &hwusb_desc_table[index].DeviceDescBank[1];

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

	if (htod_len > 0)
	{
		rxmem = &hwusb_rx_buffer[usbctrl->rx_mem_alloc];
		rxdesc->ADDR.reg = (uint32_t)rxmem;
		rxdesc->PCKSIZE.reg = (lencode << 28) | (maxlen << 14);
		usbctrl->rx_mem_alloc += htod_len;
	}

	if (dtoh_len > 0)
	{
		txmem = &hwusb_rx_buffer[usbctrl->tx_mem_alloc];
		txdesc->ADDR.reg = (uint32_t)txmem;
		txdesc->PCKSIZE.reg = (lencode << 28);
		usbctrl->tx_mem_alloc += dtoh_len;
	}

	regs->EPSTATUSCLR.reg = 0xFF;

	regs->EPINTENCLR.reg = 0xFF;
	regs->EPINTENSET.reg = (HWUSB_INTFLAG_RXSTP | HWUSB_INTFLAG_TRCPT0 | HWUSB_INTFLAG_TRCPT1);

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
	return 0;
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
	regs = &USB->DEVICE;

	// setup peripheral clock
	GCLK->PCHCTRL[USB_GCLK_ID].reg = ((1 << 6) | (0 << 0));   // select main clock frequency + enable

	MCLK->AHBMASK.bit.USB_ = 1;
	MCLK->APBBMASK.bit.USB_ = 1;

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
	irq_mask = USB_DEVICE_INTENSET_EORST;
	  //(USB_DEVICE_INTENSET_SOF | USB_DEVICE_INTENSET_EORST | USB_DEVICE_INTENSET_RAMACER
	  //| USB_DEVICE_INTFLAG_LPMSUSP | USB_DEVICE_INTFLAG_SUSPEND); // suspend state IRQ flags

	regs->INTENSET.reg = irq_mask;

	// enable the device
	regs->CTRLA.bit.ENABLE = 1;

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
	regs->DADD.reg = ((aaddr & 0x7F) | USB_DEVICE_DADD_ADDEN);
}

void THwUsbCtrl_atsam_v2::HandleIrq()
{
	uint32_t intflag = regs->INTFLAG.reg;

	if (intflag & USB_DEVICE_INTFLAG_EORST)
	{
		TRACE("USB RESET, INTFLAG=%04X\r\n", intflag);

		// disable address, configured state
		SetDeviceAddress(0);

		HandleReset();

		regs->INTFLAG.reg = USB_DEVICE_INTFLAG_EORST; // ack reset

	}

	uint32_t rev_epirq = __RBIT(regs->EPINTSMRY.reg);
	while (true)
	{
		uint32_t epid = __CLZ(rev_epirq); // returns leading zeros, 32 when the argument = 0
		if (epid >= USB_MAX_ENDPOINTS)  break; // -->

		UsbDeviceEndpoint * pep = &regs->DeviceEndpoint[epid];

		uint8_t epreg = pep->EPINTFLAG.reg;
		TRACE("[EP(%i)=%02X]\r\n", epid, epreg);

		__BKPT();

		rev_epirq &= ~(1 << (31-epid));
	}
}

#endif
