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

	uint8_t descattr = 0; // descriptor attribute
	// bits0..1: Transfer type, 0 = control, 1 = isochronous, 2 = bulk, 3 = interrupt
	// bits2..3: isochronous mode sync. type, 0 = no sync, 1 = async, 2 = adaptive, 3 = synchronous
	// bits4..5: isochronous mode usage type, 0 = data endpoint, 1 = feedback endpoint, 2 = Explicit Feedback Data Endpoint, 3 = reserved

	uint8_t eptype = (aattr & HWUSB_EP_TYPE_MASK);

	if (HWUSB_EP_TYPE_CONTROL == eptype)
	{
		descattr = 0;
	}
	else if (HWUSB_EP_TYPE_INTERRUPT == eptype)
	{
		descattr = 3;
	}
	else if (HWUSB_EP_TYPE_BULK == eptype)
	{
		descattr = 2;
	}
	else if (HWUSB_EP_TYPE_ISO == eptype)
	{
		descattr = 1; //TODO: implement other ISO attributes
	}

	epdesc_dtoh.attributes = descattr;
	epdesc_dtoh.max_packet_size = dtoh_len;

	epdesc_htod.attributes = descattr;
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

bool TUsbEndpoint::HandleSetupRequest(TUsbSetupRequest * psrq)
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

bool TUsbInterface::AddDesc(uint16_t atype, void * adataptr, uint8_t alen, uint8_t aflags)
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
	dd->flags = aflags;

	return true;
}

bool TUsbInterface::AddConfigDesc(void * adataptr, bool asubtype)
{
	uint8_t * ddata = (uint8_t *)adataptr;
	uint16_t did = ddata[1];
	if (asubtype)  did |= (ddata[2] << 8);

	TUsbDevDescRec * dd = FindDesc(did);
	if (!dd)
	{
		if (desccount >= USBINTF_MAX_DESCREC)
		{
			return false;
		}

		dd = &desclist[desccount];
		dd->id = did;
		++desccount;
	}

	dd->dataptr = (uint8_t *)adataptr;
	dd->datalen = ddata[0];
	dd->flags = (USBDESCF_CONFIG | (asubtype ? USBDESCF_SUBTYPE : 0));

	return true;
}

TUsbDevDescRec * TUsbInterface::FindDesc(uint16_t aid)
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

bool TUsbInterface::HandleSetupRequest(TUsbSetupRequest * psrq)
{
	uint8_t rqclass = ((psrq->rqtype >> 5) & 3);

	if (0 == rqclass) // standard
	{
		if (0x06 == psrq->request) // get descriptor
		{
			void * descptr = nullptr;
			uint8_t desclen = 0;

			uint8_t descid = (psrq->value >> 8);
			TUsbDevDescRec * pdr = FindDesc(descid);
			if (pdr)
			{
				descptr = pdr->dataptr;
				desclen = pdr->datalen;
			}
			else
			{
				TRACE("Interface descriptor not found: 0x%02Xr\n", descid);
				return false;
			}

			if (psrq->length < desclen)  desclen = psrq->length;  // we can not send more than the size was requested !

			TRACE("Sending descriptor 0x%02X, len = %i...\r\n", descid, desclen);

			// send the descriptor
			device->ep_ctrl.StartSend(descptr, desclen);
			return true;
		}
		else if (0x0A == psrq->request) // get interface
		{
			device->ep_ctrl.StartSend(&altsetting, 1);
			return true;
		}
		else if (0x0B == psrq->request) // set interface
		{
			altsetting = (psrq->value & 0xFF);
			device->SendControlAck();

			SetConfigured(); // notify the class driver

			return true;
		}
		else
		{
			return false;
		}
	}
	else // class and vendor requests are not handled here
	{
		return false;
	}

#if 0
	else if (
			      ((0x21 == rxbuf[0]) && (0x0A == rxbuf[1]))  // Set interface (HID)
			      ||
			      ((0x01 == rxbuf[0]) && (0x0B == rxbuf[1]))  // Set interface (CDC)
			    )
	{
		TRACE("Set interface\r\n");
		SendControlAck();

		i = rxbuf[4]; // interface index
		if (i < interface_count)
		{
			interfaces[i]->SetConfigured();
		}
	}
	else if ((0xA1 == rxbuf[0])) // vendor request...
	{
		//TRACE("Vendor Request %02X\r\n", rxbuf[1]);
		Se4ndControlAck();
	}
#endif
}

bool TUsbInterface::HandleTransferEvent(TUsbEndpoint * aep, bool htod) // should be overridden
{
	return false;
}

void TUsbInterface::SetConfigured()
{
	configured = true;
	OnConfigured();
}

void TUsbInterface::OnConfigured() // can be overridden
{
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
	int len;

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

	// configuration descriptor assembly test, required for total_length calculation
	// actually assembled into txbuf[] for debugging

	confdesc.total_length = confdesc.length;
	confdesc.num_interfaces = interface_count;

	txlen = confdesc.length;

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

		len = intf->AppendConfigDesc(&txbuf[txlen], sizeof(txbuf) - txlen);
		if (len <= 0)
		{
			return false;
		}

		txlen += len;
		confdesc.total_length += len;
	}

	memcpy(&txbuf[0], &confdesc, confdesc.length); // copy the head, the txbuf now contains the full config descriptor

	HandleReset();

	EnableIrq();

	initialized = true;

	return true;
}

void TUsbDevice::AddInterface(TUsbInterface * aintf)
{
	if (interface_count >= USBDEV_MAX_INTERFACES)
	{
		return;
	}

	aintf->device = this;
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

void TUsbDevice::HandleReset()
{
	// (Re-)initialize endpoints

	ResetEndpoints();

	for (int i = 0; i < epcount; ++i)
	{
		eplist[i]->usbctrl = this;
		eplist[i]->ConfigureHwEp();
	}

	ep_ctrl.EnableRecv(); // activate the EP0 to receive control requests
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
		if (ep_ctrl.tx_remaining_len > 0) // should we continue larger block send ?
		{
			r = ep_ctrl.SendRemaining();
			TRACE("... continue send, len = %i\r\n", r);
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
	if (0 == rxlen)
	{
		TRACE("ACK\r\n");
	}
	else
	{
		unsigned char * cp = rxbuf;
		for (i = 0; i < rxlen; ++i)
		{
			TRACE(" %02X", *cp);
			++cp;
		}
		TRACE("\r\n");
	}

	// parsing CTRL request

	if (0 == rxlen) // ACK
	{
		return true;
	}
	else if (8 == rxlen) // setup packet
	{
		TUsbSetupRequest * psrq = (TUsbSetupRequest *)&rxbuf[0];

		if ((psrq->rqtype & 0x1F) == 0) // recipient = device
		{
			if   (0x06 == psrq->request) // get descriptor
			{
				uint8_t descid = (psrq->value >> 8);
				if (3 == descid) // string descriptors have special handling
				{
					uint8_t strid = (psrq->value & 0xFF);

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
						return true;
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

						// convert the string to unicode format
						char * pdst = (char *)&txbuf[2];
						for (i = 0; i < slen; ++i)
						{
							*(pdst++) = *(str++);
							*(pdst++) = 0;
						}

						ep_ctrl.StartSend(&txbuf[0], txlen);
						return true;
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
					}
					else
					{
						TRACE("Unknown device descriptor request: 0x%02Xr\n", descid);
						return false;
					}

					if (psrq->length < desclen)  desclen = psrq->length;  // we can not send more than the size was requested !

					TRACE("Sending descriptor 0x%02X, len = %i...\r\n", descid, desclen);

					// send the descriptor
					ep_ctrl.StartSend(descptr, desclen);
					return true;
				}

			}
			else if (0x05 == psrq->request) // set address
			{
				devaddr = rxbuf[2];
				set_devaddr_on_ack = true;
				TRACE("Set device address: %i\r\n", devaddr);
				SendControlAck();
				//SetDeviceAddress(devaddr);
				return true;
			}
			else if (0x09 == psrq->request) // set configuration
			{
				SendControlAck();
				SetConfiguration(psrq->value & 0xFF);
				return true;
			}
			else
			{
				TRACE("Unhandled device setup request\r\n");
				return false;
			}
		}
		else if ((psrq->rqtype & 0x1F) == 1) // interface requests
		{
			i = psrq->index;
			if (i < interface_count)
			{
				if (interfaces[i]->HandleSetupRequest(psrq))
				{
					return true;
				}
			}
			TRACE("Unhandled interface request!\r\n");
			return false;
		}
		else if ((psrq->rqtype & 0x1F) == 2) // endpoint requests
		{
			i = psrq->index;
			if (i < epcount)
			{
				if (eplist[i]->HandleSetupRequest(psrq))
				{
					return true;
				}
			}
			TRACE("Unhandled endpoint request!\r\n");
			return false;
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
}

void TUsbDevice::SetConfiguration(uint8_t aconfig)
{
	TRACE("Set configuration: %u\r\n", aconfig);
  actualconfig = aconfig;

  // notify the interfaces
  for (int i = 0; i < interface_count; ++i)
  {
  	interfaces[i]->SetConfigured();
  }
}

void TUsbDevice::ProcessControlSendFinished()
{
	if (set_devaddr_on_ack)
	{
		SetDeviceAddress(devaddr);
		set_devaddr_on_ack = false;
	}
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
	ep_ctrl.SendAck();
}
