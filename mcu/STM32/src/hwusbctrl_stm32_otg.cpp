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

	maxlen = ((maxlen + 3) & 0xFFC); // round up to 4 bytes

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

	if (htod_len + dtoh_len + usbctrl->fifomem_end > HWUSB_MEMORY_SIZE)
	{
		// does not fit into the memory!
		return false;
	}

	// allocate the packet memory

	txbufoffs = usbctrl->fifomem_end;
	if (dtoh_len)
	{
		usbctrl->fifomem_end += dtoh_len + 16; // use some gaps, the reference manual is not so clear if it is required
	}

	// setup the pointers

	txregs  = &(usbctrl->inepregs[index]);
	rxregs = &(usbctrl->outepregs[index]);
	rxfifo = usbctrl->rxfifo; // common rxfifo

	// set EPxR base configuration

	uint32_t typecode;
	if ((attr & HWUSB_EP_TYPE_MASK) == HWUSB_EP_TYPE_CONTROL)
	{
		typecode = 0;
	}
	else if ((attr & HWUSB_EP_TYPE_MASK) == HWUSB_EP_TYPE_BULK)
	{
		typecode = 2;
	}
	else if ((attr & HWUSB_EP_TYPE_MASK) == HWUSB_EP_TYPE_ISO)
	{
		typecode = 1;
	}
	else // interrupt type
	{
		typecode = 3;
	}

	if (dtoh_len)
	{
		// configure TX (IN)

		// allocate the TX FIFO
		uint32_t txfifocfg = ((dtoh_len >> 2) << 16) | (txbufoffs << 0);
		if (0 == index)
		{
			usbctrl->gregs->DIEPTXF0_HNPTXFSIZ = txfifocfg;
		}
		else
		{
			usbctrl->gregs->DIEPTXF[index] = txfifocfg;
		}

		uint32_t epcfg = 0
			| (0         << 31)  // EPENA
			| (0         << 30)  // EPDIS
			| (0         << 29)  // SODDFRM
			| (1         << 28)  // SD0PID
			| (0         << 27)  // SNAK: Set NAK
			| (0         << 26)  // CNAK: Clear NAK
			| (index     << 22)  // TXFNUM(4): TX FIFO id
			| (0         << 21)  // STALL
			| (typecode  << 18)  // EPTYP(2): EP Type
			| (0         << 17)  // NAKSTS: NAK status
			| (0         << 16)  // DPID: data packet id
			| (1         << 15)  // USBAEP: 1 = endpoint active
			| (dtoh_len  <<  0)  // MPSIZ(11): maximum packet size (will be filled later)
		;

		// for the EP0 the size (bit0..1) must be 0, but it is fulfilled because the maxsize rounded to 32 bits

		txregs->CTL = epcfg;

		txfifo = (volatile uint32_t *)(usbctrl->periph_address + USB_OTG_FIFO_BASE + index * USB_OTG_FIFO_SIZE);

		usbctrl->irq_mask |= (1 << index);
	}

	if (htod_len)
	{
		// configure RX (OUT)

		uint32_t epcfg = 0
			| (0         << 31)  // EPENA
			| (0         << 30)  // EPDIS
			| (0         << 29)  // SODDFRM
			| (1         << 28)  // SD0PID
			| (0         << 27)  // SNAK: Set NAK
			| (0         << 26)  // CNAK: Clear NAK
			| (0         << 20)  // STALL
			| (0         << 20)  // SNMP: Snoop Mode
			| (typecode  << 18)  // EPTYP(2): EP Type
			| (0         << 17)  // NAKSTS: NAK status
			| (0         << 16)  // DPID: data packet id
			| (1         << 15)  // USBAEP: 1 = endpoint active
			| (htod_len  <<  0)  // MPSIZ(11): maximum packet size (will be filled later)
		;

		// for the EP0 the size (bit0..1) must be 0, but it is fulfilled because the maxsize rounded to 32 bits

		rxregs->CTL = epcfg;

		usbctrl->irq_mask |= (1 << (16 + index));
	}

  usbctrl->regs->DAINTMSK = usbctrl->irq_mask;

	return true;
}

int THwUsbEndpoint_stm32_otg::ReadRecvData(void * buf, uint32_t buflen)
{
  uint32_t bcnt = ((usbctrl->rxstatus >> 4) & 0x7FF);

	if (bcnt > buflen)
	{
		// buffer is too small
		return USBERR_BUFFER_TOO_SMALL;
	}

  uint32_t dwcnt = ((bcnt + 3) >> 2);

	uint32_t * pdst = (uint32_t *)buf;
	uint32_t * pend = pdst + dwcnt;

	while (pdst < pend)
	{
		*pdst++ = *rxfifo;
	}

	return bcnt;
}

int THwUsbEndpoint_stm32_otg::StartSendData(void * buf, unsigned len)
{
	uint32_t  sendlen = len;
	if (sendlen > maxlen)  sendlen = maxlen;

	txregs->TSIZ = 0;

	txregs->TSIZ = (0
	  | (0       << 29)  // MCNT(2): multi count
	  | (1       << 19)  // PKTCNT(10): packet count
	  | (sendlen <<  0)  // XFRSIZ(19)
	);

	txregs->INT = 0xFF;

	uint32_t ctl = txregs->CTL;
	ctl |= (USB_OTG_DOEPCTL_CNAK | USB_OTG_DOEPCTL_EPENA);
	txregs->CTL = ctl; // start the transfer

  uint32_t dwcnt = ((sendlen + 3) >> 2);
	uint32_t * psrc = (uint32_t *)buf;
	uint32_t * pend = psrc + dwcnt;

	while (psrc < pend)
	{
		*txfifo = *psrc++;
	}

	return sendlen;
}

void THwUsbEndpoint_stm32_otg::SendAck()
{
	txregs->TSIZ = 0;

	txregs->TSIZ = 0
	  | (0       << 29)  // MCNT(2): multi count
	  | (1       << 19)  // PKTCNT(10): packet count
	  | (0       <<  0)  // XFRSIZ(19)
	;

	uint32_t ctl = txregs->CTL;
	ctl |= (USB_OTG_DOEPCTL_CNAK | USB_OTG_DOEPCTL_EPENA);
	txregs->CTL = ctl; // start the transfer
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
	rxregs->TSIZ = 0
		| (1      << 29)  // STUPCNT(2)
		| (1      << 19)  // PKTCNT
		| (maxlen <<  0)  // XFRSIZ(7)
	;

	uint32_t ctl = rxregs->CTL;
	ctl |= (USB_OTG_DOEPCTL_CNAK | USB_OTG_DOEPCTL_EPENA);
	rxregs->CTL = ctl;
}

void THwUsbEndpoint_stm32_otg::DisableRecv()
{
	uint32_t ctl = rxregs->CTL;
	ctl |= (USB_OTG_DOEPCTL_SNAK | USB_OTG_DOEPCTL_EPENA);
	rxregs->CTL = ctl;
}

void THwUsbEndpoint_stm32_otg::StopSend()
{
	uint32_t ctl = txregs->CTL;
	ctl |= USB_OTG_DOEPCTL_EPDIS;
	txregs->CTL = ctl;
}

void THwUsbEndpoint_stm32_otg::FinishSend()
{
	//clear_epreg_ctr_tx(preg);
}

void THwUsbEndpoint_stm32_otg::Stall()
{
	if (iscontrol || dir_htod)  rxregs->CTL |= USB_OTG_DOEPCTL_STALL;
	if (iscontrol || !dir_htod) txregs->CTL |= USB_OTG_DIEPCTL_STALL;
}

void THwUsbEndpoint_stm32_otg::Nak()
{
	uint32_t ctl;
	if (iscontrol || dir_htod)
	{
		ctl = rxregs->CTL;
		ctl |= USB_OTG_DOEPCTL_SNAK;
		rxregs->CTL = ctl;
	}

	if (iscontrol || !dir_htod)
	{
		ctl = txregs->CTL;
		ctl |= USB_OTG_DOEPCTL_SNAK;
		txregs->CTL = ctl;
	}
}

bool THwUsbEndpoint_stm32_otg::IsSetupRequest()
{
	uint32_t pktsts = ((usbctrl->rxstatus >> 17) & 0x0F);
	return (6 == pktsts);
}

/************************************************************************************************************
 * THwUsbCtrl_stm32_otg
 ************************************************************************************************************/

bool THwUsbCtrl_stm32_otg::InitHw()
{
	periph_address = 0;

	#if defined (RCC_AHB1ENR_USB1OTGHSEN) // H7
		if ((1 == devnum) || (0 == devnum))
		{
			periph_address = USB1_OTG_HS_PERIPH_BASE;
			RCC->AHB1ENR |= RCC_AHB1ENR_USB1OTGHSEN;
		}
		else // 2 = devnum
		{
			periph_address = USB2_OTG_FS_PERIPH_BASE;
			RCC->AHB1ENR |= RCC_AHB1ENR_USB2OTGHSEN;
		}
	#else
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
				RCC->AHB2ENR |= RCC_AHB2ENR_OTGFSEN;
			}
	#endif

	if (!periph_address)
	{
		return false;
	}

	gregs = (USB_OTG_GlobalTypeDef *)(periph_address + USB_OTG_GLOBAL_BASE);
	regs = (USB_OTG_DeviceTypeDef *)(periph_address + USB_OTG_DEVICE_BASE);
	pcgctrl = (volatile uint32_t *)(periph_address + USB_OTG_PCGCCTL_BASE);
	rxfifo = (volatile uint32_t *)(periph_address + USB_OTG_FIFO_BASE);
	inepregs  = (THwOtgEndpointRegs *)(periph_address + USB_OTG_IN_ENDPOINT_BASE);
	outepregs = (THwOtgEndpointRegs *)(periph_address + USB_OTG_OUT_ENDPOINT_BASE);

	DisableIrq();

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

  // deactivate OTG / force device

  // Select FS device mode
  gregs->GUSBCFG &= ~USB_OTG_GUSBCFG_FHMOD;
  gregs->GUSBCFG |=  USB_OTG_GUSBCFG_FDMOD;

  delay_us(50000);  // delay 50 ms


#if defined(USB_OTG_GCCFG_VBDEN)
	gregs->GCCFG &= ~USB_OTG_GCCFG_VBDEN;  // Deactivate VBUS Sensing B

	// B-peripheral session valid override enable
	gregs->GOTGCTL |= USB_OTG_GOTGCTL_BVALOEN;
	gregs->GOTGCTL |= USB_OTG_GOTGCTL_BVALOVAL;
#else

	gregs->GCCFG |= (USB_OTG_GCCFG_NOVBUSSENS);

#endif

  // Restart the Phy Clock
  *pcgctrl = 0;

  /* Device mode configuration */
  regs->DCFG = 0x02200000 // reset value at the upper part
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

  // set the shared RX FIFO size (in 32 bit words)
  gregs->GRXFSIZ = (HWUSB_RX_FIFO_SIZE >> 2);

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
  gregs->GINTMSK |= (0
  	//|	USB_OTG_GINTMSK_USBSUSPM
  	| USB_OTG_GINTMSK_USBRST
  	| USB_OTG_GINTMSK_ENUMDNEM
  	| USB_OTG_GINTMSK_IEPINT
  	| USB_OTG_GINTMSK_OEPINT
  	//| USB_OTG_GINTMSK_IISOIXFRM
  	//| USB_OTG_GINTMSK_PXFRM_IISOOXFRM
  	//| USB_OTG_GINTMSK_WUIM
    | USB_OTG_GINTMSK_RXFLVLM
  );

	return true;
}

void THwUsbCtrl_stm32_otg::ResetEndpoints()
{
	for (int i = 0; i < HWUSB_MAX_ENDPOINTS; ++i)
	{
		THwOtgEndpointRegs *  inep  = &inepregs[i];
		THwOtgEndpointRegs *  outep = &outepregs[i];

    if ((inep->CTL & USB_OTG_DIEPCTL_EPENA) == USB_OTG_DIEPCTL_EPENA)
    {
      inep->CTL = (USB_OTG_DIEPCTL_EPDIS | USB_OTG_DIEPCTL_SNAK);
    }
    else
    {
      inep->CTL = 0;
    }

    inep->TSIZ = 0;
    inep->INT  = 0xFF;

    if ((outep->CTL & USB_OTG_DOEPCTL_EPENA) == USB_OTG_DOEPCTL_EPENA)
    {
    	outep->CTL = (USB_OTG_DOEPCTL_EPDIS | USB_OTG_DOEPCTL_SNAK);
    }
    else
    {
    	outep->CTL = 0;
    }

    outep->TSIZ = 0;
    outep->INT  = 0xFF;
  }

  for (int i = 0; i < 15; ++i)
  {
  	gregs->DIEPTXF[i] = 0; // clear transmit FIFOs
  }

  fifomem_end = HWUSB_RX_FIFO_SIZE;
  irq_mask = 0;
  regs->DAINTMSK = irq_mask;
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

void THwUsbCtrl_stm32_otg::SetDeviceAddress(uint8_t aaddr)
{
	uint32_t tmp = regs->DCFG;
	tmp &= ~USB_OTG_DCFG_DAD;
	tmp |= ((aaddr & 0x7F) << USB_OTG_DCFG_DAD_Pos);
	regs->DCFG = tmp;
}

void THwUsbCtrl_stm32_otg::HandleIrq()
{
	uint32_t istr = (gregs->GINTSTS & gregs->GINTMSK);

	if (istr & USB_OTG_GINTSTS_RXFLVL)  // common receive (htod)
	{
		// warning: the GRXSTSP (receive status) can be read only once !
		rxstatus = gregs->GRXSTSP;

		uint32_t epid = (rxstatus & 0x0F);
		uint32_t pktsts = ((rxstatus >> 17) & 0x0F);

		//TRACE("RXFLVL %08X\r\n", rxstatus);

		if ((6 == pktsts) || (2 == pktsts))  // setup or non-setup packet received
		{
			//TRACE("RXFLVL %08X\r\n", rxstatus);

			if (!HandleEpTransferEvent(epid, true))
			{
				TRACE("Unhandled RX at EP(%u)!\r\n", epid);
			}
		}
		else
		{
			// other events
			//  1 = global OUT NAK
			//  3 = OUT transfer completed
			//  4 = setup transaction completed
		}

		gregs->GINTSTS = USB_OTG_GINTSTS_RXFLVL;
	}

	if (istr & USB_OTG_GINTSTS_OEPINT)  // output (htod, RX) endpoint interrupt
	{
		// this event is turned off

		rxstatus = 0; // zeroing for IsSetupRequest()

	  uint32_t epirq = ((regs->DAINT & regs->DAINTMSK) >> 16);
		uint32_t rev_epirq = __RBIT(epirq);  // prepare for CLZ

		while (true)
		{
			uint32_t epid = __CLZ(rev_epirq); // returns leading zeros, 32 when the argument = 0
			if (epid >= HWUSB_MAX_ENDPOINTS)  break; // -->

			THwOtgEndpointRegs * pepregs = &outepregs[epid];

			TRACE("RXCOMP(%i) %02X\r\n", epid, pepregs->INT);

#if 0
			if (!HandleEpTransferEvent(epid, true))
			{
				// todo: handle error
			}
#endif

			if (pepregs->INT)
			{
				pepregs->INT = 0xFF;  // acknowledge interrupts
			}

			rev_epirq &= ~(1 << (31-epid));
		}

		gregs->GINTSTS = USB_OTG_GINTSTS_OEPINT;
	}

	if (istr & USB_OTG_GINTSTS_IEPINT)  // input (dtoh, TX) endpoint interrupt
	{
		rxstatus = 0; // zeroing for IsSetupRequest()

	  uint32_t epirq = ((regs->DAINT & regs->DAINTMSK) & 0xFFFF);
		uint32_t rev_epirq = __RBIT(epirq);  // prepare for CLZ

		while (true)
		{
			uint32_t epid = __CLZ(rev_epirq); // returns leading zeros, 32 when the argument = 0
			if (epid >= HWUSB_MAX_ENDPOINTS)  break; // -->

			THwOtgEndpointRegs * pepregs = &inepregs[epid];

			//TRACE("TXCOMP(%i) %02X\r\n", epid, pepregs->INT);

			if (!HandleEpTransferEvent(epid, false))
			{
				// todo: handle error
			}

			if (pepregs->INT)
			{
				pepregs->INT = 0xFF;  // acknowledge interrupts
			}

			rev_epirq &= ~(1 << (31-epid));
		}

		gregs->GINTSTS = USB_OTG_GINTSTS_IEPINT;
	}

	if (istr & USB_OTG_GINTSTS_USBRST)
	{
		LTRACE("USB RESET, ISTR=%04X\r\n", istr);

		// the USB registers were already cleared by the hw

		// enable only transfer complete endpoint interrupts
    regs->DOEPMSK = 0; // RX handled at the common RX fifo
    regs->DIEPMSK = USB_OTG_DIEPMSK_XFRCM;
    regs->DIEPEMPMSK = 0; // disable FIFO empty interrupts

    regs->DCFG &= ~USB_OTG_DCFG_DAD;  // set device address to 0

		HandleReset();  // resets, reconfigures end-points, enables EP0

		gregs->GINTSTS = USB_OTG_GINTSTS_USBRST;
	}

	if (istr & USB_OTG_GINTSTS_WKUINT)
	{
		LTRACE("USB WKUP, ISTR=%04X\r\n", istr);

    /* Clear the Remote Wake-up Signaling */
    regs->DCTL &= ~USB_OTG_DCTL_RWUSIG;

		//HAL_PCD_ResumeCallback(hpcd);

		gregs->GINTSTS = USB_OTG_GINTSTS_WKUINT;
	}

	if (istr & USB_OTG_GINTSTS_USBSUSP)
	{
		LTRACE("USB SUSP, ISTR=%04X\r\n", istr);

		//HAL_PCD_SuspendCallback(hpcd);

		gregs->GINTSTS = USB_OTG_GINTSTS_USBSUSP;
	}

	if (istr & USB_OTG_GINTSTS_MMIS)
	{
		// incorrect mode interrupt
		TRACE("USB INCORRECT\r\n");
		gregs->GINTSTS = USB_OTG_GINTSTS_MMIS;
	}

	if (istr & USB_OTG_GINTSTS_ENUMDNE)  // enumertation done
	{
		uint32_t dsts = regs->DSTS;  // read the enumerated speed (clears the interrupt)
		TRACE("ENUMDNE, DSTS=%02X\r\n", dsts & 15);

    /* hclk Clock Range between 32-200 MHz */
    gregs->GUSBCFG |= (uint32_t)((0x6 << 10) & USB_OTG_GUSBCFG_TRDT);
    regs->DCTL |= (USB_OTG_DCTL_CGINAK | USB_OTG_DCTL_CGONAK);

		gregs->GINTSTS = USB_OTG_GINTSTS_ENUMDNE;
	}
}

#endif
