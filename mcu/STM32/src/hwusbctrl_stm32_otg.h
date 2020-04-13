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
 *  file:     hwusbctrl_stm32_otg.h
 *  brief:    STM32 USB Controller for devices with USB OTG
 *  version:  1.00
 *  date:     2020-04-11
 *  authors:  nvitya
*/

#ifndef HWUSBCTRL_STM32_OTG_H_
#define HWUSBCTRL_STM32_OTG_H_

#include "platform.h"

#if defined(USB_OTG_FS) || defined(USB_OTG_HS)

#define HWUSBCTRL_PRE_ONLY
#include "hwusbctrl.h"

#define HWUSB_MAX_ENDPOINTS     6

#define HWUSB_MEMORY_SIZE    1280
#define HWUSB_RX_FIFO_SIZE    640  // for all endpoints, multiple packets

#define HWUSB_SET_DADDR_BEFORE_ACK  // set the device address before sending the status (ACK)

// Unified IN/OUT endpoint register definition

typedef struct
{
  __IO uint32_t   CTL;           // IN/OUT Endpoint Control Reg
  uint32_t        _res04;
  __IO uint32_t   INT;           // IN/OUT Endpoint Interrupt Reg
  uint32_t        _res0C;
  __IO uint32_t   TSIZ;          // IN/OUT Endpoint Transfer Size
  __IO uint32_t   DMA;           // IN/OUT Endpoint DMA Address Reg
  __IO uint32_t   TXFSTS;        // IN Endpoint Tx FIFO Status
  uint32_t        _res1C;
//
} THwOtgEndpointRegs;

class THwUsbEndpoint_stm32_otg : public THwUsbEndpoint_pre
{
public:
	uint16_t               txbufoffs = 0;
	volatile uint32_t *    txfifo = nullptr;
	volatile uint32_t *    rxfifo = nullptr;

	THwOtgEndpointRegs *   txregs = nullptr;
	THwOtgEndpointRegs *   rxregs = nullptr;

	//__IO uint16_t *      preg;
	//PUsbPmaDescriptor    pdesc;

	virtual ~THwUsbEndpoint_stm32_otg() { }

	bool ConfigureHwEp();
  int  ReadRecvData(void * buf, uint32_t buflen);
	int  StartSendData(void * buf, unsigned len);

	void SendAck();
  void Stall();
  void Nak();

  bool IsSetupRequest();

  void FinishRecv(bool reenable);
  void EnableRecv();
  void DisableRecv();
  void StopSend();
  void FinishSend();
};

class THwUsbCtrl_stm32_otg : public THwUsbCtrl_pre
{
public:
	uint32_t                   periph_address = 0;
	USB_OTG_DeviceTypeDef *    regs = nullptr;
	USB_OTG_GlobalTypeDef *    gregs = nullptr;
	volatile uint32_t *        pcgctrl = nullptr;
	volatile uint32_t *        rxfifo = nullptr;

	THwOtgEndpointRegs *       inepregs = nullptr;
	THwOtgEndpointRegs *       outepregs = nullptr;

	uint32_t                   rxstatus = 0;  // cached GRXSTSP content

	uint32_t                   irq_mask = 0;
	uint16_t       			       fifomem_end = 0; // used FIFO memory (shared RX + every TX)

	bool InitHw();

	void HandleIrq();

	inline void DisableIrq() {  gregs->GAHBCFG &= ~USB_OTG_GAHBCFG_GINT; }
	inline void EnableIrq()  {  gregs->GAHBCFG |=  USB_OTG_GAHBCFG_GINT; }
	void SetDeviceAddress(uint8_t aaddr);
	virtual void SetPullUp(bool aenable);

	void ResetEndpoints();
};

#define HWUSBENDPOINT_IMPL   THwUsbEndpoint_stm32_otg
#define HWUSBCTRL_IMPL       THwUsbCtrl_stm32_otg

#endif

#endif // def HWUSBCTRL_STM32_OTG_H_
