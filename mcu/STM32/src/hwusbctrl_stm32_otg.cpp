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
 *  file:     hwusbctrl_stm32_otg.cpp
 *  brief:    STM32 USB Controller for devices with USB OTG
 *  version:  1.00
 *  date:     2020-04-11
 *  authors:  nvitya
*/

#include "platform.h"

#if defined(USB_OTG_FS) || defined(USB_OTG_HS)

#include "string.h"
#include <stdio.h>
#include <stdarg.h>
#include <hwusbctrl.h>
#include "clockcnt.h"

#define LTRACES
#include "traces.h"

bool THwUsbEndpoint_stm32_otg::ConfigureHwEp()
{
	if (!usbctrl)
	{
		return false;
	}

#if 0

	if (maxlen > 62)
	{
		// in 32 byte granularity
		maxlen = ((maxlen + 0x1F) & 0xFE0);
	}
	else
	{
		// in 2 byte granularity
		maxlen = ((maxlen + 0x01) & 0x3E);
	}

	uint16_t htod_len;
	uint16_t dtoh_len;
	if ((attr & HWUSB_EP_TYPE_MASK) == HWUSB_EP_TYPE_CONTROL)
	{
		htod_len = maxlen;
		dtoh_len = maxlen;
	}
	else
	{
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
	}

	if (htod_len + dtoh_len + usbctrl->pma_mem_end > PACKET_MEMORY_SIZE)
	{
		// does not fit into the memory!
		return false;
	}

	// allocate the packet memory

	txbufoffs = usbctrl->pma_mem_end;
	usbctrl->pma_mem_end += dtoh_len;

	rxbufoffs = usbctrl->pma_mem_end;
	usbctrl->pma_mem_end += htod_len;

	// setup the pointers

	preg = &usbctrl->regs->EP0R;
	preg += (index * 2);  // the register distance is always 32 bit

	pdesc = PUsbPmaDescriptor(USB_PMAADDR);
	pdesc += index;

	// setup the descriptor table

	pdesc->ADDR_TX   = txbufoffs;
	pdesc->COUNT_TX  = 0;

	// set the buffer size:
	pdesc->ADDR_RX   = rxbufoffs;
	if (maxlen >= 32)
	{
		pdesc->COUNT_RX = ((maxlen >> 5) << 10) | 0x8000;  // the size is presented in 32 byte blocks
	}
	else
	{
		pdesc->COUNT_RX = ((maxlen >> 1) << 10);  // the size is presented in 2 byte blocks
	}

	// set EPxR base configuration

	uint16_t epconf =	0;

	epconf |= (index & 0xF);  // add the endpoint address

	if ((attr & HWUSB_EP_TYPE_MASK) == HWUSB_EP_TYPE_CONTROL)
	{
		epconf |= (1 << 9);
	}
	else if ((attr & HWUSB_EP_TYPE_MASK) == HWUSB_EP_TYPE_BULK)
	{
		epconf |= (0 << 9);
	}
	else if ((attr & HWUSB_EP_TYPE_MASK) == HWUSB_EP_TYPE_ISO)
	{
		epconf |= (2 << 9);
	}
	else // interrupt type
	{
		epconf |= (3 << 9);
	}

	set_epreg_static_content(preg, epconf);

	// set the default state to NAK, otherwise it stays disabled
	set_epreg_tx_status(preg, 2);  // NAK
	set_epreg_rx_status(preg, 2);  // NAK

#endif

	return true;
}

int THwUsbEndpoint_stm32_otg::ReadRecvData(void * buf, uint32_t buflen)
{
#if 0

	if (buflen & 1)
	{
		// invalid buffer size
		return USBERR_INVALID_BUFFER_SIZE;
	}

	unsigned cnt = (pdesc->COUNT_RX & 0x1FF);
	if (cnt > buflen)
	{
		// buffer is too small
		return USBERR_BUFFER_TOO_SMALL;
	}

	uint16_t * psrc = (uint16_t *)(USB_PMAADDR); // the PMA allows only 16 bit accesses !

	// do the copiing bytewise (to handle unaligned buffers on Cortex-M0 processors):

	psrc += (pdesc->ADDR_RX / 2);
	uint8_t * pdst = (uint8_t *)(buf);

	unsigned ccnt = ((cnt + 1) >> 1);
	for (unsigned i = 0; i < ccnt; ++i)
	{
		uint16_t tmp16 = *psrc++;
		*pdst++ = tmp16;
		*pdst++ = (tmp16 >> 8);
	}

	return cnt;
#else
	return 0;
#endif
}

int THwUsbEndpoint_stm32_otg::StartSendData(void * buf, unsigned len)
{
	// copy words

	uint16_t  sendlen = len;
	if (sendlen > maxlen)  sendlen = maxlen;

#if 0

	uint16_t remaining = sendlen;

	//strace("  sending %i bytes...\r\n", remaining);

	uint16_t * pdst = (uint16_t *)(USB_PMAADDR); // the PMA allows only 16 bit accesses !

	pdst += (pdesc->ADDR_TX / 2);
	uint8_t * psrc = (uint8_t *)(buf);

	while (remaining >= 2)
	{
		uint16_t tmp16;
		tmp16 = *psrc++;  // do the copiing bytewise (to handle unaligned buffers on Cortex-M0 processors):
		tmp16 |= ((*psrc++) << 8);
		*pdst++ = tmp16;
		remaining -= 2;
  }

	if (remaining == 1)
	{
		// the last byte
		*pdst = *psrc;
	}

	// signalize count
	pdesc->COUNT_TX = sendlen;
	// start the transfer
	set_epreg_tx_status(preg, 3);

#endif

	return sendlen;
}

void THwUsbEndpoint_stm32_otg::SendAck()
{
#if 0
	pdesc->COUNT_TX = 0;
	set_epreg_tx_status(preg, 3);
#endif
}

void THwUsbEndpoint_stm32_otg::FinishRecv(bool reenable)
{
	//clear_epreg_ctr_rx(preg);
	if (reenable)
	{
		EnableRecv();
	}
}

void THwUsbEndpoint_stm32_otg::EnableRecv()
{
	//set_epreg_rx_status(preg, 3);  // restart read
}

void THwUsbEndpoint_stm32_otg::DisableRecv()
{
	//set_epreg_rx_status(preg, 0);
}

void THwUsbEndpoint_stm32_otg::StopSend()
{
	//set_epreg_tx_status(preg, 0);
}

void THwUsbEndpoint_stm32_otg::FinishSend()
{
	//clear_epreg_ctr_tx(preg);
}

void THwUsbEndpoint_stm32_otg::Stall()
{
	//if (iscontrol || dir_htod)  set_epreg_rx_status(preg, 1);
	//if (iscontrol || !dir_htod) set_epreg_tx_status(preg, 1);
}

void THwUsbEndpoint_stm32_otg::Nak()
{
	//if (iscontrol || dir_htod)  set_epreg_rx_status(preg, 2);
	//if (iscontrol || !dir_htod) set_epreg_tx_status(preg, 2);
}

bool THwUsbEndpoint_stm32_otg::IsSetupRequest()
{
	//return (*preg & USB_EP_SETUP);

	return false;
}

/************************************************************************************************************
 * THwUsbCtrl_stm32_otg
 ************************************************************************************************************/

bool THwUsbCtrl_stm32_otg::InitHw()
{
	periph_address = 0;

#if defined(USB_OTG_HS)
	if (1 == devnum)
	{
		periph_address = USB_OTG_HS_PERIPH_BASE;
		RCC->AHB1ENR |= RCC_AHB1ENR_OTGHSEN;
	}
	else
#endif
	{
		periph_address = USB_OTG_FS_PERIPH_BASE;
		gregs = USB_OTG_FS;
		pcgctrl = (volatile uint32_t *)(USB_OTG_FS_PERIPH_BASE + USB_OTG_PCGCCTL_BASE);
		RCC->AHB2ENR |= RCC_AHB2ENR_OTGFSEN;
	}

	if (!periph_address)
	{
		return false;
	}

	gregs = (USB_OTG_GlobalTypeDef *)(periph_address + USB_OTG_GLOBAL_BASE);
	regs = (USB_OTG_DeviceTypeDef *)(periph_address + USB_OTG_DEVICE_BASE);
	pcgctrl = (volatile uint32_t *)(periph_address + USB_OTG_PCGCCTL_BASE);
	inepregs = (USB_OTG_INEndpointTypeDef *)(periph_address + USB_OTG_IN_ENDPOINT_BASE);
	outepregs = (USB_OTG_OUTEndpointTypeDef *)(periph_address + USB_OTG_OUT_ENDPOINT_BASE);

	// The 48 MHz clock from the main PLL is selected by default, should be running

  gregs->GAHBCFG &= ~USB_OTG_GAHBCFG_GINT;  // disable the USB global interrupt

  // FS interface (embedded Phy)

	gregs->GUSBCFG |= USB_OTG_GUSBCFG_PHYSEL;  // Select FS Embedded PHY

  // Reset after a PHY select

	// Wait for AHB master IDLE state
	while ((gregs->GRSTCTL & USB_OTG_GRSTCTL_AHBIDL) == 0)
	{
		// wait
	}

	gregs->GRSTCTL |= USB_OTG_GRSTCTL_CSRST;  // Core Soft Reset
	while ((gregs->GRSTCTL & USB_OTG_GRSTCTL_CSRST) == USB_OTG_GRSTCTL_CSRST)
	{
		// wait
	}

  gregs->GCCFG = USB_OTG_GCCFG_PWRDWN;  // Deactivate the power down

#if 0
  // enable DMA
  regs->GAHBCFG |= (USB_OTG_GAHBCFG_HBSTLEN_1 | USB_OTG_GAHBCFG_HBSTLEN_2);
  regs->GAHBCFG |= USB_OTG_GAHBCFG_DMAEN;
#endif

  // Select FS device mode
  gregs->GUSBCFG &= ~USB_OTG_GUSBCFG_FHMOD;
  gregs->GUSBCFG |=  USB_OTG_GUSBCFG_FDMOD;

  delay_us(50000);  // delay 50 ms

  // deactivate OTG / force device
	gregs->GCCFG &= ~USB_OTG_GCCFG_VBDEN;  // Deactivate VBUS Sensing B

	// B-peripheral session valid override enable
	gregs->GOTGCTL |= USB_OTG_GOTGCTL_BVALOEN;
	gregs->GOTGCTL |= USB_OTG_GOTGCTL_BVALOVAL;

  // Restart the Phy Clock
  *pcgctrl = 0;

  /* Device mode configuration */
  regs->DCFG = 0x02200000 // reset value
    | (0  << 15)  // ERRATIM
    | (0  << 14)  // XCVRDLY: 0 = disable delay
    | (0  << 11)  // PFIVL(2): periodic frame interval, 0 = 80% of the frame interval
    | (0  <<  4)  // DAD(7): device address
    | (0  <<  0)  // NZLSOHSK: Non-zero-length status OUT handshake
    | (3  <<  0)  // DSPD(2): device speed, 3 = full speed with internal PHY, 0 = high speed
  ;

  // Flush FIFOs

  gregs->GRSTCTL = (USB_OTG_GRSTCTL_TXFFLSH | (0x10 << 6));  // flush all TX FIFOs
  while ((gregs->GRSTCTL & USB_OTG_GRSTCTL_TXFFLSH) == USB_OTG_GRSTCTL_TXFFLSH)
  {
  	// wait until finishes
  }

  gregs->GRSTCTL = USB_OTG_GRSTCTL_RXFFLSH; // flush all RX FIFOs
  while ((gregs->GRSTCTL & USB_OTG_GRSTCTL_RXFFLSH) == USB_OTG_GRSTCTL_RXFFLSH)
  {
  	// wait until finishes
  }

  // Clear all pending Device Interrupts
  regs->DIEPMSK = 0;
  regs->DOEPMSK = 0;
  regs->DAINT = 0xFFFFFFFF;
  regs->DAINTMSK = 0;

  ResetEndpoints();

  regs->DIEPMSK &= ~(USB_OTG_DIEPMSK_TXFURM);

#if 0
  if (cfg.dma_enable == 1)
  {
    /*Set threshold parameters */
    USBx_DEVICE->DTHRCTL = (USB_OTG_DTHRCTL_TXTHRLEN_6 | USB_OTG_DTHRCTL_RXTHRLEN_6);
    USBx_DEVICE->DTHRCTL |= (USB_OTG_DTHRCTL_RXTHREN | USB_OTG_DTHRCTL_ISOTHREN | USB_OTG_DTHRCTL_NONISOTHREN);

    if (USBx_DEVICE->DTHRCTL) { }
  }
#endif

  /* Disable all interrupts. */
  gregs->GINTMSK = 0;

  /* Clear any pending interrupts */
  gregs->GINTSTS = 0xBFFFFFFF;

  /* Enable interrupts matching to the Device mode ONLY */
  gregs->GINTMSK |= (USB_OTG_GINTMSK_USBSUSPM | USB_OTG_GINTMSK_USBRST |\
                    USB_OTG_GINTMSK_ENUMDNEM | USB_OTG_GINTMSK_IEPINT |\
                    USB_OTG_GINTMSK_OEPINT   | USB_OTG_GINTMSK_IISOIXFRM|\
                    USB_OTG_GINTMSK_PXFRM_IISOOXFRM | USB_OTG_GINTMSK_WUIM);
#if 0
  if (cfg.dma_enable == DISABLE)
  {
    gregs->GINTMSK |= USB_OTG_GINTMSK_RXFLVLM;
  }
#endif

  gregs->GAHBCFG |= USB_OTG_GAHBCFG_GINT;  // disable the USB global interrupt

	return true;
}

void THwUsbCtrl_stm32_otg::ResetEndpoints()
{
	for (int i = 0; i < USB_MAX_ENDPOINTS; ++i)
	{
		USB_OTG_INEndpointTypeDef *   inep = &inepregs[i];
		USB_OTG_OUTEndpointTypeDef *  outep = &outepregs[i];

    if ((inep->DIEPCTL & USB_OTG_DIEPCTL_EPENA) == USB_OTG_DIEPCTL_EPENA)
    {
      inep->DIEPCTL = (USB_OTG_DIEPCTL_EPDIS | USB_OTG_DIEPCTL_SNAK);
    }
    else
    {
      inep->DIEPCTL = 0;
    }

    inep->DIEPTSIZ = 0;
    inep->DIEPINT  = 0xFF;

    if ((outep->DOEPCTL & USB_OTG_DOEPCTL_EPENA) == USB_OTG_DOEPCTL_EPENA)
    {
    	outep->DOEPCTL = (USB_OTG_DOEPCTL_EPDIS | USB_OTG_DOEPCTL_SNAK);
    }
    else
    {
    	outep->DOEPCTL = 0;
    }

    outep->DOEPTSIZ = 0;
    outep->DOEPINT  = 0xFF;
  }
}

void THwUsbCtrl_stm32_otg::SetPullUp(bool aenable)
{
	if (aenable)
	{
	  regs->DCTL &= ~USB_OTG_DCTL_SDIS ;
	}
	else
	{
	  regs->DCTL |= USB_OTG_DCTL_SDIS ;
	}
}

void THwUsbCtrl_stm32_otg::HandleIrq()
{
	//source:  HAL_PCD_IRQHandler

#if 0

	uint32_t istr = regs->ISTR;

	if (istr & USB_ISTR_CTR)
	{
		// Endpoint transfer finished.
		//PCD_EP_ISR_Handler(hpcd);

		int epid = (istr & 7);
		bool htod = ((istr & 0x10) != 0);

		volatile uint16_t * epreg = &regs->EP0R;
		epreg += (epid * 2);  // the register distance is always 32 bit

		if (htod)
		{
			clear_epreg_ctr_rx(epreg);
		}
		else
		{
			clear_epreg_ctr_tx(epreg);
		}

		//LTRACE("[EP-%i CTR %i]\r\n", epid, htod);

		if (!HandleEpTransferEvent(epid, htod))
		{
			LTRACE("Unhandled endpoint: %u, htod=%u\r\n", epid, htod);
		}

		//strace("CTR on EP(%i), ISRX=%i, EPREG=%04X\r\n", epid, isrx, *epdef->preg);

		//uartx.printf("EPR0 = %04X\r\n", regs->EP0R);

		//uartx.printf("  EP.COUNT_RX = %04X\r\n", epdef->pdesc->COUNT_RX);

		// EPR0 = EA60
		//  EP.COUNT_RX = 8808

		//show_ep_rxdata(0);

//  	__BKPT(0);

		regs->ISTR &= ~USB_ISTR_CTR;
	}

	if (regs->ISTR & USB_ISTR_RESET)
	{
		LTRACE("USB RESET, ISTR=%04X\r\n", regs->ISTR);

		// the USB registers were already cleared by the hw

		//HAL_PCD_SetAddress(hpcd, 0):
		regs->DADDR = USB_DADDR_EF;

		HandleReset();

		regs->ISTR &= ~USB_ISTR_RESET;
	}

	if (regs->ISTR & USB_ISTR_PMAOVR)
	{
		//LTRACE("USB PMAOVR, ISTR=%04X\r\n", regs->ISTR);
		regs->ISTR &= ~USB_ISTR_PMAOVR;
	}

	if (regs->ISTR & USB_ISTR_ERR)
	{
		//LTRACE("USB ERR, ISTR=%04X\r\n", regs->ISTR);
		regs->ISTR &= ~USB_ISTR_ERR;
	}

	if (regs->ISTR & USB_ISTR_WKUP)
	{
		//LTRACE("USB WKUP, ISTR=%04X\r\n", regs->ISTR);

		regs->CNTR &= ~(USB_CNTR_LP_MODE);

		/*Set interrupt mask*/
		regs->CNTR = irq_mask;

		//HAL_PCD_ResumeCallback(hpcd);

		regs->ISTR &= ~USB_ISTR_WKUP;
	}

	if (regs->ISTR & USB_ISTR_SUSP)
	{
		//LTRACE("USB SUSP, ISTR=%04X\r\n", regs->ISTR);

		// Force low-power mode in the macrocell
		regs->CNTR |= USB_CNTR_FSUSP;
		regs->CNTR |= USB_CNTR_LP_MODE;

		if ((regs->ISTR & USB_ISTR_WKUP) == 0)
		{
			//HAL_PCD_SuspendCallback(hpcd);
		}

		/* clear of the ISTR bit must be done after setting of CNTR_FSUSP */
		regs->ISTR &= ~USB_ISTR_SUSP;
	}

#endif
}

#endif
