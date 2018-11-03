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

#include "string.h"
#include "usbdevice.h"
#include "traces.h"

// -----------------------------------------------------------------------------------------
// TUsbEndpoint
// -----------------------------------------------------------------------------------------

bool TUsbEndpoint::Init(uint8_t aattr, uint16_t ahtod_len, uint16_t adtoh_len)
{
	index = 0xFF; // invalid index

	htod_len = ahtod_len;
	dtoh_len = adtoh_len;

	attr = aattr;

	epdesc_dtoh.attributes = aattr;
	epdesc_dtoh.max_packet_size = dtoh_len;

	epdesc_htod.attributes = aattr;
	epdesc_htod.max_packet_size = htod_len;

	return true;
}

void TUsbEndpoint::SetIndex(uint8_t aindex)
{
	index = aindex;
	if (htod_len > 0)  epdesc_htod.endpoint_address = aindex;
	if (dtoh_len > 0)  epdesc_dtoh.endpoint_address = (aindex | 0x80);
}

bool TUsbEndpoint::HandleTransferEvent(bool htod) // can be overridden
{
	return false;
}

// -----------------------------------------------------------------------------------------
// TUsbInterface
// -----------------------------------------------------------------------------------------

int TUsbInterface::AppendConfigDesc(uint8_t * dptr, uint16_t maxlen)
{
	uint8_t i;
	uint8_t *  dp = dptr;
	uint16_t   remaining = maxlen;
	uint16_t   result = 0;
	uint16_t   dsize;

	// first add the interface descriptor
	dsize = sizeof(intfdesc);
	if (remaining < sizeof(intfdesc)) 	return 0;
	memcpy(dp, &intfdesc, dsize);
	dp += dsize;
	remaining -= dsize;

	// then the other descriptors marked as USBDESCF_CONFIG
	for (i = 0; i < desccount; ++i)
	{
		TUsbDevDescRec * ddp = &desclist[i];
		if (ddp->flags & USBDESCF_CONFIG)
		{
			dsize = ddp->datalen;
			if (remaining < dsize)		return 0;
			memcpy(dp, ddp->dataptr, dsize);
			dp += dsize;
			remaining -= dsize;
		}
	}

	// and then the endpoint descriptors

	for (i = 0; i < epcount; ++i)
	{
		TUsbEndpoint * ep = eplist[i];
		if (ep->dtoh_len > 0)
		{
			dsize = sizeof(ep->epdesc_dtoh);
			if (remaining < dsize)		return 0;
			memcpy(dp, &ep->epdesc_dtoh, dsize);
			dp += dsize;
			remaining -= dsize;
		}

		if (ep->htod_len > 0)
		{
			dsize = sizeof(ep->epdesc_htod);
			if (remaining < dsize)		return 0;
			memcpy(dp, &ep->epdesc_htod, dsize);
			dp += dsize;
			remaining -= dsize;
		}
	}

	return (dp - dptr);
}

bool TUsbInterface::InitInterface() // must be overridden
{
	return false;
}

bool TUsbInterface::AddDesc(uint8_t atype, void * adataptr, uint16_t alen, uint8_t aflags)
{
	TUsbDevDescRec * dd = FindDesc(atype);
	if (!dd)
	{
		if (desccount >= USBINTF_MAX_DESCREC)
		{
			return false;
		}

		dd = &desclist[desccount];
		dd->id = atype;
		++desccount;
	}

	dd->dataptr = (uint8_t *)adataptr;
	dd->datalen = alen;

	return true;
}

TUsbDevDescRec * TUsbInterface::FindDesc(uint8_t aid)
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

void TUsbInterface::AddEndpoint(TUsbEndpoint * aep)
{
	if (epcount >= USBINTF_MAX_ENDPOINTS)
	{
		return;
	}

	aep->interface = this;
	eplist[epcount] = aep;
	++epcount;
}

bool TUsbInterface::HandleTransferEvent(TUsbEndpoint * aep, bool htod) // should be overridden
{
	return false;
}

// -----------------------------------------------------------------------------------------
// TUsbDevice
// -----------------------------------------------------------------------------------------

bool TUsbDevice::InitDevice()  // can be overridden
{
	return true;
}

bool TUsbDevice::Init()
{
	uint8_t i;
	int ctrl_rq_len;

	initialized = false;

	if (!InitDevice())
	{
		return false;
	}

	// check device descriptor
	if (devdesc.vendor_id == 0)
	{
		return false;
	}

	if (interface_count <= 0)
	{
		return false;
	}

	if (!InitHw())
	{
		return false;
	}

	// build up internal structures

	epcount = 0;
	ep_ctrl.Init(HWUSB_EP_TYPE_CONTROL, 64, 64);  // this must be always the first endpoint (id = 0)
  AddEndpoint(&ep_ctrl);

	// prepare the descriptors

	stringcount = 0;

	// device descriptor
	devdesc.stri_manufacturer = AddString(manufacturer_name);
	devdesc.stri_product = AddString(device_name);
	devdesc.stri_serial_number = AddString(device_serial_number);

	// configuration descriptor (first)

	confdesc.total_length = confdesc.length;
	confdesc.num_interfaces = interface_count;

	for (i = 0; i < interface_count; ++i)
	{
		TUsbInterface * intf = interfaces[i];

		if (!intf->InitInterface())
		{
			return false;
		}

		if (!PrepareInterface(i, intf)) // adds interface endpoints
		{
			return false;
		}

		// calculate the config descriptor length

		ctrl_rq_len = intf->AppendConfigDesc(&txbuf[0], sizeof(txbuf));
		if (ctrl_rq_len <= 0)
		{
			return false;
		}

		confdesc.total_length += ctrl_rq_len;
	}

	// initialize endpoints
	for (i = 0; i < epcount; ++i)
	{
		eplist[i]->usbctrl = this;
		eplist[i]->ConfigureHwEp();
	}

	initialized = true;

	return true;
}

void TUsbDevice::AddInterface(TUsbInterface * aintf)
{
	if (interface_count >= USBDEV_MAX_INTERFACES)
	{
		return;
	}

	interfaces[interface_count] = aintf;
	++interface_count;
}

uint8_t TUsbDevice::AddString(const char * astr)
{
	if (stringcount >= USBDEV_MAX_STRINGS)
	{
		return 0xFF;
	}

	stringtable[stringcount] = (char *)astr;
	++stringcount;

	return stringcount;  // index + 1 !
}

void TUsbDevice::AddEndpoint(TUsbEndpoint * aep)
{
	if (epcount >= USBDEV_MAX_ENDPOINTS)
	{
		aep->index = 0xFF;
		return;
	}

	aep->SetIndex(epcount);

	eplist[epcount] = aep;
	++epcount;
}

bool TUsbDevice::PrepareInterface(uint8_t ifidx, TUsbInterface * pif)
{
	if ((pif->intfdesc.interface_class == 0xFF) || (pif->epcount == 0))
	{
		return false;
	}

	pif->index = ifidx;
	pif->intfdesc.interface_number = ifidx;
	pif->intfdesc.stri_interface = AddString(pif->interface_name);
	pif->intfdesc.num_endpoints = pif->epcount;

	// add endpoints
	uint8_t i;
	for (i = 0; i < pif->epcount; ++i)
	{
		AddEndpoint(pif->eplist[i]); // updates the endpoint descriptors too
	}

	return true;
}


bool TUsbDevice::HandleEpTransferEvent(uint8_t epid, bool htod)
{
	if (0 == epid)
	{
		return HandleControlEndpoint(htod);
	}

	if (epid >= USBDEV_MAX_ENDPOINTS)
	{
		return false;
	}

	TUsbEndpoint * ep = eplist[epid];
	if (!ep)
	{
		return false;
	}

	if (ep->interface)
	{
		return ep->interface->HandleTransferEvent(ep, htod);
	}
	else
	{
		return ep->HandleTransferEvent(htod);
	}
}

bool TUsbDevice::HandleControlEndpoint(bool htod)
{
	int r;
	if (htod)
	{
		rxlen = ep_ctrl.ReadRecvData(&rxbuf[0], sizeof(rxbuf));
		if (r < 0)
		{
			return false; // the ep will stall
		}

		ProcessControlRequest();

		ep_ctrl.FinishRecv(true); // clear IRQ reason and reenable receive
		return true;
	}
	else
	{
		// device to host send completed
		ep_ctrl.FinishSend(); // clear IRQ reason
		if (ep_ctrl.SendRemaining() > 0) // should we continue larger block send ?
		{
			return true;
		}

		ProcessControlSendFinished();

		return true;
	}
}

bool TUsbDevice::ProcessControlRequest()
{
	int i;

	TRACE("CTRL request: ");
	unsigned char * cp = rxbuf;
	for (i = 0; i < rxlen; ++i)
	{
		TRACE(" %02X", *cp);
		++cp;
	}
	TRACE("\r\n");

	// parsing CTRL request

	if (0 == rxlen) // ACK
	{
		if (set_configured_on_ack)
		{
			set_configured_on_ack = false;
			SetConfigured();
		}

		TRACE("(ACK)\r\n");
		return true;
	}
	else if (8 == rxlen) // setup packet
	{
		// << RX(8):  80 06 00 01 00 00 12 00  // request main config

		if ((rxbuf[0] & 0x80) && (0x06 == rxbuf[1])) // Get Descriptor
		{
			uint8_t descid = rxbuf[3];
			if (3 == descid) // string descriptors have special handling
			{
				uint8_t strid = rxbuf[2];

				if (0 == strid) // sending supported languages
				{
					//TRACE("Sending string table\r\n");
					// supported languages
					// 04 03 09 04
					txlen = 4;
					txbuf[0] = txlen;
					txbuf[1] = 3;
					txbuf[2] = 0x09;
					txbuf[3] = 0x04;

					ep_ctrl.StartSend(&txbuf[0], txlen);
				}
				else
				{
					// string data
					const char * str = nullptr;
					if (strid < USBDEV_MAX_STRINGS)
					{
						str = stringtable[strid];
					}

					if (!str)
					{
						str = "???";
						TRACE("GETSTRING: string 0x%02X not found\r\n", strid);
					}

					//TRACE("Sending string \"%s\"\r\n", str);

					uint8_t slen = strlen(str);

					if (slen > 31)  slen = 31;

					txlen = slen * 2 + 2;
					txbuf[0] = txlen;
					txbuf[1] = 3;

					char * pdst = (char *)&txbuf[2];
					for (i = 0; i < slen; ++i)
					{
						*(pdst++) = *(str++);
						*(pdst++) = 0;
					}

					ep_ctrl.StartSend(&txbuf[0], txlen);
				}
			}
			else // normal, static descriptors
			{
				void * descptr = nullptr;
				uint8_t desclen = 0;

				if (USB_DESC_TYPE_DEVICE == descid)
				{
					descptr = &devdesc;
					desclen = devdesc.length;
				}
				else if (USB_DESC_TYPE_CONFIGURATION == descid)
				{
					// special case, assembly the config descriptor into the txbuf, can be bigger than 64 bytes
					MakeDeviceConfig();
					descptr = &txbuf[0];
					desclen = txlen;
					return true;
				}
				else
				{
					// otherwise the descriptor must be registered in an interface

					TUsbDevDescRec * pdr = nullptr;

					for (i = 0; i < interface_count; ++i)
					{
						pdr = interfaces[i]->FindDesc(descid);
						if (pdr)
						{
							descptr = pdr->dataptr;
							desclen = pdr->datalen;
							break;
						}
					}
				}

				if (!descptr)
				{
					TRACE("GETDESCRIPTOR: Descriptor 0x%02X not found\r\n", descid);
					return false;
				}

				uint16_t rqlen = (rxbuf[6] + (rxbuf[7] << 8));
				TRACE("Sending descriptor 0x%02X, ctrl_rq_len = %i...\r\n", descid, rqlen);

				if (rqlen < desclen)  desclen = rqlen;  // we can not send more than the size was requested !

				// send the descriptor
				ep_ctrl.StartSend(descptr, desclen);

				return true;
			}
		}
		else if ((0x00 == rxbuf[0]) && (0x05 == rxbuf[1])) // Set address
		{
			devaddr = rxbuf[2];
			TRACE("Set device address: %i\r\n", devaddr);

			set_devaddr_on_ack = true;
			SendControlAck();
		}
		else if ((0x00 == rxbuf[0]) && (0x09 == rxbuf[1])) // Set configuration
		{
			TRACE("Set configuration\r\n");
			SendControlAck();
		}
		else if ((0x21 == rxbuf[0]) && (0x0A == rxbuf[1])) // Set interface ???
		{
			TRACE("Set interface\r\n");
			SendControlAck();

			SetConfigured();
		}
		else if ((0xA1 == rxbuf[0])) // vendor request...
		{
			//TRACE("Vendor Request %02X\r\n", rxbuf[1]);
			SendControlAck();
		}
		else
		{
			// unknown request
			TRACE("Unknown request!\r\n");

			SendControlAck();
			return false;
		}
	}
	else
	{
		// unknown control request
		TRACE("Invalid control request, len = %i\r\n", rxlen);
		return false;
	}

	return true;
}

void TUsbDevice::ProcessControlSendFinished()
{
	if (set_devaddr_on_ack)
	{
		SetDeviceAddress(devaddr);
		set_devaddr_on_ack = false;
	}
}

void TUsbDevice::SetConfigured()
{
	OnConfigured();
	configured = true;
}

void TUsbDevice::MakeDeviceConfig()
{
	txlen = confdesc.length;
	memcpy(&txbuf[0], &confdesc, txlen);
	uint8_t ind = txlen;

	for (int i = 0; i < interface_count; ++i)
	{
		TUsbInterface * intf = interfaces[i];

		// calculate the config descriptor length

		int len = intf->AppendConfigDesc(&txbuf[ind], sizeof(txbuf) - ind);
		if (len <= 0)
		{
			break;
		}

		txlen += len;
	}
}

void TUsbDevice::SendControlAck()
{
	ep_ctrl.StartSend(&txbuf[0], 0); // active tx buffer with 0 length means acknowledge
}

void TUsbDevice::OnConfigured() // can be overridden
{
}
