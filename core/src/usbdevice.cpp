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
 *  file:     usbdevice.cpp
 *  brief:    USB Device Generics
 *  version:  1.00
 *  date:     2018-05-18
 *  authors:  nvitya
*/

#include "usbdevice.h"

TUsbDevice::TUsbDevice()
{
	for (int i = 0; i < USBDEV_MAX_STRINGS; ++i)
	{
		stringtable[i] = nullptr;
	}

	// device descriptor
	devdesc.length = 18; // fix value
	devdesc.descriptor_type = USB_DESC_TYPE_DEVICE;
	devdesc.usb_version = 0x0200;
	devdesc.device_class = 0;
	devdesc.device_sub_class = 0;
	devdesc.device_protocol = 0;
	devdesc.max_packet_size = 64;
	devdesc.vendor_id = 0; // must be overridden !
	devdesc.product_id = 0;
	devdesc.device_version = 0x0200;
	devdesc.stri_manufacturer = USBD_STRIDX_MANUFACTURER;
	devdesc.stri_product = USBD_STRIDX_PRODUCT;
	devdesc.stri_serial_number = USBD_STRIDX_SERIAL;
	devdesc.num_configurations = 1;
}

bool TUsbDevice::InitDevice()  // must be overridden !
{
	return false;
}

bool TUsbDevice::Init()
{
	initialized = false;

	if (!usbctrl.Init())
	{
		return false;
	}

	desccount = 0;
	SetDesc(USB_DESC_TYPE_DEVICE, &devdesc, devdesc.length);  // sets only the pointer so the device initialization can change
	                                                          // (and must change) the fields of the devdesc
  usbctrl.AddEndpoint(&ep_ctrl,  0, 64, 64, USBEF_TYPE_CONTROL);

	if (!InitDevice())
	{
		return false;
	}

	initialized = true;

	return true;
}


void TUsbDevice::SetDesc(uint8_t aid, void * adataptr, uint8_t adatalen)
{
	TUsbDevDescRec * dd = FindDesc(aid);
	if (!dd)
	{
		dd = &desclist[desccount];
		dd->id = aid;
		++desccount;
	}

	dd->dataptr = (uint8_t *)adataptr;
	dd->datalen = adatalen;
}

TUsbDevDescRec * TUsbDevice::FindDesc(uint8_t aid)
{
	for (int i = 0; i < desccount; ++i)
	{
		if (desclist[i].id == aid)
		{
			return &desclist[i];
		}
	}
	return nullptr;
}

