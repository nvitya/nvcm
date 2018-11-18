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
 *  file:     hwusbctrl_stm32.cpp
 *  brief:    STM32 USB Controller for F0 and F1 devices
 *  version:  1.00
 *  date:     2018-05-18
 *  authors:  nvitya
*/

#include "platform.h"

#if defined(USB_PMAADDR)

#include "string.h"
#include <stdio.h>
#include <stdarg.h>
#include <hwusbctrl.h>
#include "traces.h"

#if defined(USB_CNTR_LPMODE) && !defined(USB_CNTR_LP_MODE)
  #define USB_CNTR_LP_MODE USB_CNTR_LPMODE
#endif


#define EPREG_INVARIANT_CLEAR_MASK  (~((1 << 14) | (3 << 12) | (1 << 6) | (3 << 4)))
#define EPREG_INVARIANT_SET_MASK    ((1 << 15) | (1 << 7))

static void set_epreg_rx_status(__IO uint16_t * preg, uint16_t astate)
{
  // the bits can not be overridden, so we must use toggling

	uint16_t regval = *preg;
	uint16_t toggling = ((regval >> 12) & 3) ^ (astate & 3);

	regval &= EPREG_INVARIANT_CLEAR_MASK;
	regval |= EPREG_INVARIANT_SET_MASK; // KEEP RX TX Complete flags

	*preg = (regval | (toggling << 12));
}

static void set_epreg_tx_status(__IO uint16_t * preg, uint16_t astate)
{
  // the bits can not be overridden, so we must use toggling

	uint16_t regval = *preg;
	uint16_t toggling = ((regval >> 4) & 3) ^ (astate & 3);

	regval &= EPREG_INVARIANT_CLEAR_MASK;
	regval |= EPREG_INVARIANT_SET_MASK;

	*preg = (regval | (toggling << 4));
}

static void set_epreg_static_content(__IO uint16_t * preg, uint16_t avalue)
{
	uint16_t regval = avalue;

	regval &= ~((1 << 14) | (3 << 12) | (1 << 6) | (3 << 4));
	regval |= ((1 << 15) | (1 << 7));

	*preg = regval;
}

static void clear_epreg_ctr_tx(__IO uint16_t * preg)
{
	uint16_t regval = *preg;

	regval &= (EPREG_INVARIANT_CLEAR_MASK & 0xFF7F);  // delete TX complete
	regval |= (EPREG_INVARIANT_SET_MASK & 0xFF7F);  // keep the RX complete

	*preg = regval;
}

static void clear_epreg_ctr_rx(__IO uint16_t * preg)
{
	uint16_t regval = *preg;

	regval &= (EPREG_INVARIANT_CLEAR_MASK & 0x7FFF);  // delete RX complete
	regval |= (EPREG_INVARIANT_SET_MASK & 0x7FFF);  // keep the TX complete

	*preg = regval;
}

bool THwUsbEndpoint_stm32::ConfigureHwEp()
{
	if (!usbctrl)
	{
		return false;
	}

	if (htod_len > 62)
	{
		// in 32 byte granularity
		htod_len = ((htod_len + 0x1F) & 0xFE0);
	}
	else
	{
		// in 2 byte granularity
		htod_len = ((htod_len + 0x01) & 0x3E);
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
	preg += (index * 2);

	pdesc = PUsbPmaDescriptor(USB_PMAADDR);
	pdesc += index;

	// setup the descriptor table

	pdesc->ADDR_TX   = txbufoffs;
	pdesc->COUNT_TX  = 0;

	// set the buffer size:
	pdesc->ADDR_RX   = rxbufoffs;
	if (htod_len >= 32)
	{
		pdesc->COUNT_RX = ((htod_len >> 5) << 10) | 0x8000;  // the size is presented in 32 byte blocks
	}
	else
	{
		pdesc->COUNT_RX = ((htod_len >> 1) << 10);  // the size is presented in 2 byte blocks
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

	set_epreg_tx_status(preg, 0);
	set_epreg_rx_status(preg, 0);

	return true;
}

int THwUsbEndpoint_stm32::ReadRecvData(void * buf, uint32_t buflen)
{
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

	uint16_t * psrc = (uint16_t *)(USB_PMAADDR);
#ifdef HWUSB_16_32
	psrc += pdesc->ADDR_RX; // ADDR_RX in bytes but this will increment words
#else
	psrc += pdesc->ADDR_RX / 2; // increment in bytes
#endif

	uint16_t * pdst = (uint16_t *)(buf);

	unsigned ccnt = ((cnt + 1) >> 1);
	for (unsigned i = 0; i < ccnt; ++i)
	{
		*pdst = *psrc;
#ifdef HWUSB_16_32
		psrc += 2;
#else
		psrc += 1;
#endif
		pdst += 1;
	}

	return cnt;
}

int THwUsbEndpoint_stm32::SendRemaining()
{
	// copy words

	uint16_t * pdst = (uint16_t *)(USB_PMAADDR);
#ifdef HWUSB_16_32
	pdst += pdesc->ADDR_TX;  // ADDR_TX in bytes but this will increment words !
#else
	pdst += pdesc->ADDR_TX / 2;  // increment in bytes
#endif

	uint16_t * psrc = (uint16_t *)(tx_remaining_dataptr);

	uint16_t  sendlen = tx_remaining_len;
	if (sendlen > dtoh_len)  sendlen = dtoh_len;

	uint16_t remaining = sendlen;

	//strace("  sending %i bytes...\r\n", remaining);

	// do the copiing:
	while (remaining >= 2)
	{
		*pdst = *psrc;
		psrc += 1;
#ifdef HWUSB_16_32
		pdst += 2;
#else
		pdst += 1;
#endif
		remaining -= 2;
  }

	if (remaining == 1)
	{
		// the last byte
		*(uint8_t *)pdst = *(uint8_t *)psrc;
	}

	tx_remaining_dataptr += sendlen;
	tx_remaining_len -= sendlen;

	// signalize count
	pdesc->COUNT_TX = sendlen;
	// start the transfer
	set_epreg_tx_status(preg, 3);

	return sendlen;
}

void THwUsbEndpoint_stm32::SendAck()
{
	tx_remaining_len = 0;
	pdesc->COUNT_TX = 0;
	set_epreg_tx_status(preg, 3);
}

void THwUsbEndpoint_stm32::FinishRecv(bool reenable)
{
	clear_epreg_ctr_rx(preg);
	if (reenable)
	{
		EnableRecv();
	}
}

void THwUsbEndpoint_stm32::EnableRecv()
{
	set_epreg_rx_status(preg, 3);  // restart read
}

void THwUsbEndpoint_stm32::DisableRecv()
{
	set_epreg_rx_status(preg, 0);
}

void THwUsbEndpoint_stm32::StopSend()
{
	set_epreg_tx_status(preg, 0);
	tx_remaining_len = 0;
}

void THwUsbEndpoint_stm32::FinishSend()
{
	clear_epreg_ctr_tx(preg);
}

void THwUsbEndpoint_stm32::Stall()
{
	if (htod_len) set_epreg_rx_status(preg, 1);
	if (dtoh_len) set_epreg_tx_status(preg, 1);
}

/************************************************************************************************************
 * THwUsbCtrl_stm32
 ************************************************************************************************************/

bool THwUsbCtrl_stm32::InitHw()
{
	regs = nullptr;

	regs = (HWUSBCTRL_REGS *)USB_BASE;
	RCC->APB1ENR |= RCC_APB1ENR_USBEN;

	#if defined(MCUSF_F0)
			// for STM32F070F6 remap the PA11 + PA12 from UART to USB
			RCC->APB2ENR |= RCC_APB2ENR_SYSCFGEN;
			SYSCFG->CFGR1 |= (1 << 4); // select PA11 + PA12 on pins 17,18 (USBDP, USBDM)

			RCC->CFGR3 |= RCC_CFGR3_USBSW; // enable USB clock !
	#endif

	if (!regs)
	{
		return false;
	}

  // irq_mask = USB_CNTR_CTRM | USB_CNTR_WKUPM | USB_CNTR_SUSPM | USB_CNTR_ERRM | USB_CNTR_ESOFM | USB_CNTR_RESETM;
  irq_mask = USB_CNTR_CTRM | USB_CNTR_WKUPM | USB_CNTR_SUSPM | USB_CNTR_ERRM | USB_CNTR_RESETM;

	DisableIrq();

  // USB_DevInit(hpcd->Instance, hpcd->Init) ->
  regs->CNTR = USB_CNTR_FRES; // reset USB registers
  if (regs->CNTR) { } // synchronize HW
  regs->CNTR = 0; // remove reset
  regs->ISTR = 0;
  regs->BTABLE = 0;  // the descriptor table is at the beginning

  memset((uint8_t *)USB_PMAADDR, 0, 2 * 64); // clear USB PMA descriptor buffer

#if defined(MCUSF_F0)
  regs->LPMCSR = 0;
  regs->BCDR = 0;
#endif

  // clear device address:
  regs->DADDR = USB_DADDR_EF;

  ResetEndpoints();

	return true;
}

void THwUsbCtrl_stm32::HandleIrq()
{
	//source:  HAL_PCD_IRQHandler

	uint32_t istr = regs->ISTR;

	if (istr & USB_ISTR_CTR)
	{
		// Endpoint transfer finished.
		//PCD_EP_ISR_Handler(hpcd);

		int epid = (istr & 7);
		bool htod = ((istr & 0x10) != 0);

		//TRACE("EP(%03X) event\r\n", istr & 0x17);

		if (!HandleEpTransferEvent(epid, htod))
		{
			TRACE("Unhandled endpoint: %u, htod=%u\r\n", epid, htod);
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
		TRACE("USB RESET, ISTR=%04X\r\n", regs->ISTR);

		// the USB registers were already cleared by the hw

		//HAL_PCD_SetAddress(hpcd, 0):
		regs->DADDR = USB_DADDR_EF;

		HandleReset();

		regs->ISTR &= ~USB_ISTR_RESET;
	}

	if (regs->ISTR & USB_ISTR_PMAOVR)
	{
		TRACE("USB PMAOVR, ISTR=%04X\r\n", regs->ISTR);
		regs->ISTR &= ~USB_ISTR_PMAOVR;
	}

	if (regs->ISTR & USB_ISTR_ERR)
	{
		TRACE("USB ERR, ISTR=%04X\r\n", regs->ISTR);
		regs->ISTR &= ~USB_ISTR_ERR;
	}

	if (regs->ISTR & USB_ISTR_WKUP)
	{
		TRACE("USB WKUP, ISTR=%04X\r\n", regs->ISTR);

		regs->CNTR &= ~(USB_CNTR_LP_MODE);

		/*Set interrupt mask*/
		regs->CNTR = irq_mask;

		//HAL_PCD_ResumeCallback(hpcd);

		regs->ISTR &= ~USB_ISTR_WKUP;
	}

	if (regs->ISTR & USB_ISTR_SUSP)
	{
		TRACE("USB SUSP, ISTR=%04X\r\n", regs->ISTR);

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

	if (regs->ISTR & USB_ISTR_SOF)
	{
		//strace("USB SOF, ISTR=%04X\r\n", regs->ISTR);

		regs->ISTR &= ~USB_ISTR_SOF;
		//HAL_PCD_SOFCallback(hpcd);
	}

/*
	if (regs->ISTR & USB_ISTR_ESOF)
	{
		strace("USB ESOF, ISTR=%04X\r\n", regs->ISTR);
		regs->ISTR &= ~USB_ISTR_ESOF;
	}
*/
}

void THwUsbCtrl_stm32::ResetEndpoints()
{
	pma_mem_end = 8 * 8; // skip the descriptor table

	int i;
	__IO uint16_t * eprptr = &regs->EP0R;

	for (i = 0; i < USB_MAX_ENDPOINTS; ++i)
	{
		set_epreg_rx_status(eprptr, 0);
		set_epreg_tx_status(eprptr, 0);

		eprptr += 2;
	}
}

#endif
