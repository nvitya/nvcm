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
 *  file:     hwusbctrl_stm32.h
 *  brief:    STM32 USB Controller
 *  version:  1.00
 *  date:     2018-05-18
 *  authors:  nvitya
*/

#ifndef HWUSBCTRL_STM32_H_
#define HWUSBCTRL_STM32_H_

#include "platform.h"

#if defined(USB_PMAADDR) //defined(MCUSF_F1) || defined(MCUSF_F0)

#define HWUSBCTRL_PRE_ONLY
#include "hwusbctrl.h"

#define USB_MAX_ENDPOINTS       8

#define PACKET_MEMORY_SIZE    512

#if defined(MCUSF_F1)

typedef struct
{
  __IO uint16_t ADDR_TX;
  __IO uint16_t _RESERVED0;
  __IO uint16_t COUNT_TX;
  __IO uint16_t _RESERVED1;
  __IO uint16_t ADDR_RX;
  __IO uint16_t _RESERVED2;
  __IO uint16_t COUNT_RX;
  __IO uint16_t _RESERVED3;
//
} TUsbPmaDescriptor;

#elif defined(MCUSF_F0)

typedef struct
{
  __IO uint16_t ADDR_TX;
  __IO uint16_t COUNT_TX;
  __IO uint16_t ADDR_RX;
  __IO uint16_t COUNT_RX;
//
} TUsbPmaDescriptor;

#else

#error "unimplemented MCU sub-family"

#endif

typedef TUsbPmaDescriptor *  PUsbPmaDescriptor;

#define HWUSBCTRL_REGS  USB_TypeDef

class THwUsbEndpoint_stm32 : public THwUsbEndpoint_pre
{
public:
	uint16_t             rxbufoffs;
	uint16_t             txbufoffs;

	__IO uint16_t *      preg;
	PUsbPmaDescriptor    pdesc;

	virtual ~THwUsbEndpoint_stm32() { }

	bool ConfigureHwEp();
	int  SendRemaining();
	void SendAck();
  int  ReadRecvData(void * buf, uint32_t buflen);

  void FinishRecv(bool reenable);
  void EnableRecv();
  void DisableRecv();
  void StopSend();
  void FinishSend();
  void Stall();
};

class THwUsbCtrl_stm32 : public THwUsbCtrl_pre
{
public:
	HWUSBCTRL_REGS *    regs = nullptr;
	uint32_t            irq_mask;
	uint16_t       			pma_mem_end;

	bool InitHw();

	void HandleIrq();

	inline void DisableIrq() {  regs->CNTR &= ~irq_mask; }
	inline void EnableIrq()  {  regs->CNTR |=  irq_mask; }
	inline void SetDeviceAddress(uint8_t aaddr) { regs->DADDR = (USB_DADDR_EF | (aaddr & 0x7F)); }

	void ResetEndpoints();

public:

#if 0
	bool 		ep_add(uint8_t aid, uint16_t atxbufsize, uint16_t arxbufsize, uint16_t aflags);

	int     ep_recv(int epid, void * buf, unsigned len, unsigned flags);
	int     ep_send(int epid, void * buf, unsigned len, unsigned flags);
	int     ep_send_remaining(int epid);

  void    set_device_address(uint8_t aaddr);
#endif
};

#define HWUSBENDPOINT_IMPL   THwUsbEndpoint_stm32
#define HWUSBCTRL_IMPL       THwUsbCtrl_stm32

#endif

#endif // def HWUSBCTRL_STM32_H_
