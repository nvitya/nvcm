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
 *  file:     hwusbctrl_atsam.h
 *  brief:    ATSAM USB Controller
 *  version:  1.00
 *  date:     2018-12-11
 *  authors:  nvitya
*/

#ifndef HWUSBCTRL_ATSAM_H_
#define HWUSBCTRL_ATSAM_H_

#include "platform.h"

#define HWUSBCTRL_PRE_ONLY
#include "hwusbctrl.h"

#define USB_MAX_ENDPOINTS       8

#define PACKET_MEMORY_SIZE    512

#define HWUSBCTRL_REGS  Udp

class THwUsbEndpoint_atsam : public THwUsbEndpoint_pre
{
public:

	__IO uint8_t *       fiforeg;
	__IO uint32_t *      csreg;

	virtual ~THwUsbEndpoint_atsam() { }

	bool ConfigureHwEp();
  int  ReadRecvData(void * buf, uint32_t buflen);
	int  StartSendData(void * buf, unsigned len);

	void SendAck();
  void Stall();
  void Nak();

  inline bool IsSetupRequest()  { return (*csreg & UDP_CSR_RXSETUP); }

  void EnableRecv();
  void DisableRecv();
  void StopSend();
  void FinishSend();
};

class THwUsbCtrl_atsam : public THwUsbCtrl_pre
{
public:
	HWUSBCTRL_REGS *    regs = nullptr;
	uint32_t            irq_mask = 0;

	bool InitHw();

	void HandleIrq();

	void DisableIrq();
	void EnableIrq();
	void SetDeviceAddress(uint8_t aaddr);
	virtual void SetPullUp(bool aenable);

	void ResetEndpoints();
};

#define HWUSBENDPOINT_IMPL   THwUsbEndpoint_atsam
#define HWUSBCTRL_IMPL       THwUsbCtrl_atsam

#endif // def HWUSBCTRL_ATSAM_H_
