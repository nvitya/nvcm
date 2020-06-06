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
 *  file:     hwusbctrl_atsam_v2.h
 *  brief:    ATSAM_V2 USB Controller
 *  version:  1.00
 *  date:     2020-05-22
 *  authors:  nvitya
*/

#ifndef HWUSBCTRL_ATSAM_V2_H_
#define HWUSBCTRL_ATSAM_V2_H_

#include "platform.h"

#if defined(USB)

#define HWUSBCTRL_PRE_ONLY
#include "hwusbctrl.h"

#define USB_MAX_ENDPOINTS       8

#define USB_RX_BUFFER_SIZE    256
#define USB_TX_BUFFER_SIZE    256

class THwUsbEndpoint_atsam_v2 : public THwUsbEndpoint_pre
{
public:
	UsbDeviceEndpoint *  regs = nullptr;
	UsbDeviceDescBank *  rxdesc = nullptr;
	UsbDeviceDescBank *  txdesc = nullptr;

	uint8_t *            rxmem = nullptr;
	uint8_t *            txmem = nullptr;

	virtual ~THwUsbEndpoint_atsam_v2() { }

	bool ConfigureHwEp();
  int  ReadRecvData(void * buf, uint32_t buflen);
	int  StartSendData(void * buf, unsigned len);

	void SendAck();
  void Stall();
  void Nak();

  inline bool IsSetupRequest()  { return false; } //{ return (*csreg & UDP_CSR_RXSETUP); }

  void EnableRecv();
  void DisableRecv();
  void StopSend();
  void FinishSend();
};

class THwUsbCtrl_atsam_v2 : public THwUsbCtrl_pre
{
public:
	UsbDevice *         regs = nullptr;
	uint32_t            irq_mask = 0;

	uint16_t            rx_mem_alloc = 0;
	uint16_t            tx_mem_alloc = 0;

	bool InitHw();

	void HandleIrq();

	void DisableIrq();
	void EnableIrq();
	void SetDeviceAddress(uint8_t aaddr);
	virtual void SetPullUp(bool aenable);

	void ResetEndpoints();

protected:
	void LoadCalibration(); // ATSAM_V2 speciality
};

#define HWUSBENDPOINT_IMPL   THwUsbEndpoint_atsam_v2
#define HWUSBCTRL_IMPL       THwUsbCtrl_atsam_v2

#endif // if defined(USB)

#endif // def HWUSBCTRL_ATSAM_V2_H_
