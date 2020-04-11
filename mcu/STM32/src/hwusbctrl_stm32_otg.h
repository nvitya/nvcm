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

#define USB_MAX_ENDPOINTS     6
#define PACKET_MEMORY_SIZE    1024

#define HWUSBCTRL_REGS        USB_OTG_GlobalTypeDef

class THwUsbEndpoint_stm32_otg : public THwUsbEndpoint_pre
{
public:
	uint16_t             rxbufoffs;
	uint16_t             txbufoffs;

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

	USB_OTG_INEndpointTypeDef *   inepregs = nullptr;
	USB_OTG_OUTEndpointTypeDef *  outepregs = nullptr;

	uint32_t                   irq_mask;
	uint16_t       			       pma_mem_end;

	bool InitHw();

	void HandleIrq();

	inline void DisableIrq() {  gregs->GAHBCFG &= ~USB_OTG_GAHBCFG_GINT; }
	inline void EnableIrq()  {  gregs->GAHBCFG |=  USB_OTG_GAHBCFG_GINT; }
	inline void SetDeviceAddress(uint8_t aaddr) {} //{ regs->DADDR = (USB_DADDR_EF | (aaddr & 0x7F)); }
	virtual void SetPullUp(bool aenable);

	void ResetEndpoints();
};

#define HWUSBENDPOINT_IMPL   THwUsbEndpoint_stm32_otg
#define HWUSBCTRL_IMPL       THwUsbCtrl_stm32_otg

#endif

#endif // def HWUSBCTRL_STM32_OTG_H_
