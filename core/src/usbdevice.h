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
 *  file:     usbdevice.h
 *  brief:    USB Device Generics
 *  version:  1.00
 *  date:     2018-05-18
 *  authors:  nvitya
*/

#ifndef USBDEVICE_H_
#define USBDEVICE_H_

#include "hwusbctrl.h"

#define USBDEV_CTRL_BUF_SIZE   64
#define USBDEV_MAX_STRINGS     16
#define USBDEV_MAX_DESCREC     8

#define USB_DESC_TYPE_DEVICE                           1
#define USB_DESC_TYPE_CONFIGURATION                    2
#define USB_DESC_TYPE_STRING                           3
#define USB_DESC_TYPE_INTERFACE                        4
#define USB_DESC_TYPE_ENDPOINT                         5
#define USB_DESC_TYPE_DEVICE_QUALIFIER                 6
#define USB_DESC_TYPE_OTHER_SPEED_CONFIGURATION        7
#define USB_DESC_TYPE_BOS                              0x0F

#define USB_LEN_DEV_QUALIFIER_DESC                     0x0A
#define USB_LEN_DEV_DESC                               0x12
#define USB_LEN_CFG_DESC                               0x09
#define USB_LEN_IF_DESC                                0x09
#define USB_LEN_EP_DESC                                0x07
#define USB_LEN_OTG_DESC                               0x03
#define USB_LEN_LANGID_STR_DESC                        0x04
#define USB_LEN_OTHER_SPEED_DESC_SIZ                   0x09


#define USBD_STRIDX_LANGID        0x00
#define USBD_STRIDX_MANUFACTURER  0x01
#define USBD_STRIDX_PRODUCT       0x02
#define USBD_STRIDX_SERIAL        0x03
#define USBD_STRIDX_CONFIG        0x04
#define USBD_STRIDX_INTERFACE     0x05

typedef struct TUsbDeviceDescriptor
{
	uint8_t		length;
	uint8_t 	descriptor_type;
	uint16_t 	usb_version;
	uint8_t  	device_class;
	uint8_t  	device_sub_class;
	uint8_t  	device_protocol;
	uint8_t  	max_packet_size;
	uint16_t 	vendor_id;
	uint16_t 	product_id;
	uint16_t 	device_version;
	uint8_t  	stri_manufacturer;
	uint8_t  	stri_product;
	uint8_t  	stri_serial_number;
	uint8_t  	num_configurations;
//
} __attribute__((__packed__)) TUsbDeviceDescriptor;

typedef struct TUsbDevDescRec
{
	uint8_t					id;
	uint8_t      		datalen;
	uint8_t  *      dataptr;
//
} TUsbDevDescRec;

class TUsbInterface
{
//public:
};

class TUsbDevice
{
public:
	THwUsbCtrl            usbctrl;

	bool                  initialized = false;

	TUsbDeviceDescriptor  devdesc;

	char *          			stringtable[USBDEV_MAX_STRINGS];

	TUsbDevDescRec        desclist[USBDEV_MAX_DESCREC];
	int                   desccount = 0;

	void                  SetDesc(uint8_t aid, void * adataptr, uint8_t adatalen);
	TUsbDevDescRec *      FindDesc(uint8_t aid);

	THwUsbEndpoint        ep_ctrl;

	TUsbDevice();
	virtual ~TUsbDevice() { }

	bool Init();

  virtual bool InitDevice();
};

#endif /* USBDEVICE_H_ */
