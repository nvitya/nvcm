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
 *  file:     hwusbctrl.h
 *  brief:    USB Controller (device only) vendor-independent definitions
 *  version:  1.00
 *  date:     2018-05-18
 *  authors:  nvitya
*/

#ifndef HWUSBCTRL_H_PRE_
#define HWUSBCTRL_H_PRE_

#include "hwpins.h"

//#define USB_EP_TYPE_MASK        0x0007
#define USB_EP_TYPE_CONTROL      0x0001
#define USB_EP_TYPE_INTERRUPT    0x0002
#define USB_EP_TYPE_BULK         0x0003
#define USB_EP_TYPE_ISO          0x0004

// USB Endpoint Flags
#define USBEF_TYPE_CONTROL       0x0001
#define USBEF_TYPE_INTERRUPT     0x0002
#define USBEF_TYPE_BULK          0x0003
#define USBEF_TYPE_ISO           0x0004
#define USBEF_TYPE_MASK          0x000F

// Error codes
#define USBERR_INVALID_EP            -1
#define USBERR_INVALID_BUFFER_SIZE   -2  // must be dividible by 2
#define USBERR_BUFFER_TOO_SMALL      -3
#define USBERR_TX_OVERWRITE          -4  // there were remaining data in the tx buffer

class THwUsbCtrl; // forward declaration
class THwUsbEndpoint;

class THwUsbEndpoint_pre
{
public:
	THwUsbCtrl *    usbctrl;
	uint8_t         id = 0;
	uint16_t        txbufsize = 0;
	uint16_t        rxbufsize = 0;
	uint32_t        flags = 0;

	uint8_t *       dataptr = nullptr;  // data send
	unsigned        datalen = 0;
};

class THwUsbCtrl_pre
{
public:
	bool              initialized = false;

	virtual ~THwUsbCtrl_pre() { }
	virtual void HandleReset() { }
	virtual bool HandleData(uint8_t epid, bool isrx) { return false; }
};

#endif // ndef HWUSBCTRL_H_PRE_

#ifndef HWUSBCTRL_PRE_ONLY

//-----------------------------------------------------------------------------

#ifndef HWUSBCTRL_H_
#define HWUSBCTRL_H_

#include "mcu_impl.h"

#if !defined(HWUSBCTRL_IMPL) || !defined(HWUSBENDPOINT_IMPL)

#define USB_MAX_ENDPOINTS 8

class THwUsbEndpoint_noimpl : public THwUsbEndpoint_pre
{
public: // mandatory
	bool Configure()   { return false; }  // based on previously set fields

};

class THwUsbCtrl_noimpl : public THwUsbCtrl_pre
{
public: // mandatory
	bool InitHw() { return false; }

};

#define HWUSBENDPOINT_IMPL   THwUsbEndpoint_noimpl
#define HWUSBCTRL_IMPL   THwUsbCtrl_noimpl

#endif // ndef HWUSBCTRL_IMPL

//-----------------------------------------------------------------------------

class THwUsbEndpoint : public HWUSBENDPOINT_IMPL
{
public: // mandatory
	bool    Init(THwUsbCtrl * ausbctrl, uint8_t aid, uint16_t atxbufsize, uint16_t arxbufsize, uint32_t aflags);

	int     Recv(void * buf, unsigned len, unsigned flags);
	int     Send(void * buf, unsigned len, unsigned flags);
};

class THwUsbCtrl : public HWUSBCTRL_IMPL
{
public:
	THwUsbEndpoint *  epbyid[USB_MAX_ENDPOINTS];

	bool Init();
	bool AddEndpoint(THwUsbEndpoint * aep, uint8_t aid, uint16_t atxbufsize, uint16_t arxbufsize, uint32_t aflags);

	virtual bool HandleData(uint8_t epid, bool isrx);
	virtual void HandleReset();
};

#endif // ndef HWUSBCTRL_H_ */

#else
  #undef HWUSBCTRL_PRE_ONLY
#endif
