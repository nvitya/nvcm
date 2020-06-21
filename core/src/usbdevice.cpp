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
 *  notes:
 *    Speical thanks to Niklas Gürtler for this tutorial: https://www.mikrocontroller.net/articles/USB-Tutorial_mit_STM32
 *    Special thanks to Craig Peacock for this tutorial: https://www.beyondlogic.org/usbnutshell/usb1.shtml
*/

#include "string.h"
#include "usbdevice.h"

#define LTRACES
#include "traces.h"

// -----------------------------------------------------------------------------------------
// TUsbEndpoint
// -----------------------------------------------------------------------------------------

bool TUsbEndpoint::Init(uint8_t aattr, bool adir_htod, uint16_t amaxlen)
{
	index = 0xFF; // invalid index

	maxlen = amaxlen;
	dir_htod = adir_htod;

	attr = aattr;

	if (HWUSB_EP_TYPE_CONTROL == (attr & HWUSB_EP_TYPE_MASK))
	{
		iscontrol = true;
	}
	else
	{
		iscontrol = false;
	}

	return true;
}

void TUsbEndpoint::SetIndex(uint8_t aindex)
{
	index = aindex;
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

int TUsbInterface::AppendConfigDesc(uint8_t * dptr, uint16_t amaxlen)
{
	uint8_t i;
	uint8_t *  dp = dptr;
	uint16_t   remaining = amaxlen;
	//uint16_t   result = 0;
	uint8_t    dsize;

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

	TUsbEndpointDesc epdesc = {0};
	dsize = sizeof(epdesc);
	epdesc.length = dsize;
	epdesc.descriptor_type = USB_DESC_TYPE_ENDPOINT;

	for (i = 0; i < epcount; ++i)
	{
		TUsbEndpoint * ep = eplist[i];

		epdesc.max_packet_size = ep->maxlen;
		epdesc.interval = ep->interval;
		// descriptor attribute:
		//   bits0..1: Transfer type, 0 = control, 1 = isochronous, 2 = bulk, 3 = interrupt
		//   bits2..3: isochronous mode sync. type, 0 = no sync, 1 = async, 2 = adaptive, 3 = synchronous
		//   bits4..5: isochronous mode usage type, 0 = data endpoint, 1 = feedback endpoint, 2 = Explicit Feedback Data Endpoint, 3 = reserved

		uint8_t eptype = (ep->attr & HWUSB_EP_TYPE_MASK);
		if (HWUSB_EP_TYPE_CONTROL == eptype)
		{
			epdesc.attributes = 0;
		}
		else if (HWUSB_EP_TYPE_INTERRUPT == eptype)
		{
			epdesc.attributes = 3;
		}
		else if (HWUSB_EP_TYPE_BULK == eptype)
		{
			epdesc.attributes = 2;
		}
		else if (HWUSB_EP_TYPE_ISO == eptype)
		{
			epdesc.attributes = 1; //TODO: implement other ISO attributes
		}

		if (ep->iscontrol || !ep->dir_htod)
		{
			epdesc.endpoint_address = (ep->index | 0x80);
			if (remaining < dsize)		return 0;
			memcpy(dp, &epdesc, dsize);
			dp += dsize;
			remaining -= dsize;
		}

		if (ep->iscontrol || ep->dir_htod)
		{
			epdesc.endpoint_address = ep->index;
			if (remaining < dsize)		return 0;
			memcpy(dp, &epdesc, dsize);
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
				LTRACE("Interface descriptor not found: 0x%02Xr\n", descid);
				device->SendControlStatus(false);
				return true;
			}

			if (psrq->length < desclen)  desclen = psrq->length;  // we can not send more than the size was requested !

			LTRACE("Sending descriptor 0x%02X, len = %i...\r\n", descid, desclen);

			// send the descriptor
			device->StartSendControlData(descptr, desclen);
			return true;
		}
		else if (0x0A == psrq->request) // get interface
		{
			device->StartSendControlData(&altsetting, 1);
			return true;
		}
		else if (0x0B == psrq->request) // set interface
		{
			altsetting = (psrq->value & 0xFF);
			device->SendControlStatus(false);

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
}

bool TUsbInterface::HandleSetupData(TUsbSetupRequest * psrq, void * adata, unsigned adatalen)
{
	return false;
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
	ep_ctrl.Init(HWUSB_EP_TYPE_CONTROL, false, 64);  // this must be always the first endpoint (id = 0)
  AddEndpoint(&ep_ctrl);

	// prepare the descriptors

	string_count = 0;

	// device descriptor
	devdesc.stri_manufacturer = AddString(manufacturer_name);
	devdesc.stri_product = AddString(device_name);
	devdesc.stri_serial_number = AddString(device_serial_number);

	// configuration descriptor assembly test, required for total_length calculation
	// actually assembled into ctrlbuf[] for debugging

	confdesc.total_length = confdesc.length;
	confdesc.num_interfaces = interface_count;

	cdlen = confdesc.length;

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

		len = intf->AppendConfigDesc(&ctrlbuf[cdlen], sizeof(ctrlbuf) - cdlen);
		if (len <= 0)
		{
			return false;
		}

		cdlen += len;
		confdesc.total_length += len;
	}

	memcpy(&ctrlbuf[0], &confdesc, confdesc.length); // copy the head, the ctrlbuf now contains the full config descriptor

	HandleReset();

	SetPullUp(true);

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
	if (string_count >= USBDEV_MAX_STRINGS)
	{
		return 0xFF;
	}

	stringtable[string_count] = (char *)astr;
	++string_count;

	return string_count;  // index + 1 !
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
	if (pif->epcount == 0)
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

void TUsbDevice::StartSendControlData() // txlen + ctrlbuf must be prepared
{
	int r;
	r = ep_ctrl.StartSendData(&ctrlbuf[0], cdlen);
	if (r < 0)
	{
		LTRACE("Error sending control data: %i\r\n", r);
		SendControlStatus(false);
		return;
	}
	ctrl_datastage_remaining = cdlen - r;
	ctrlstage = USBCTRL_STAGE_DATAIN;
/*
	if (ctrl_datastage_remaining <= 0)
	{
		ctrlstage = USBCTRL_STAGE_STATUS;
	}
*/
}

void TUsbDevice::StartSendControlData(void * asrc, unsigned alen)
{
	cdlen = alen;
	if (cdlen > sizeof(ctrlbuf))  cdlen = sizeof(ctrlbuf);
	memcpy(&ctrlbuf[0], asrc, cdlen);

	StartSendControlData();
}

void TUsbDevice::StartReceiveControlData(unsigned alen)
{
	if (alen > sizeof(ctrlbuf))  alen = sizeof(ctrlbuf);
	ctrl_datastage_remaining = alen;
	ctrlstage = USBCTRL_STAGE_DATAOUT;
}

bool TUsbDevice::HandleEpTransferEvent(uint8_t epid, bool htod)
{
	if (0 == epid)
	{
		HandleControlEndpoint(htod);
		return true;
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

void TUsbDevice::HandleControlEndpoint(bool htod)
{
	int r;

	if (ep_ctrl.IsSetupRequest())
	{
		ctrlstage = USBCTRL_STAGE_SETUP;
		cdlen = 0;
		ctrl_datastage_remaining = 0;

		if (!htod)
		{
			LTRACE("Setup Request: Invalid direction!\r\n");
			SendControlStatus(false);
			return;
		}

		r = ep_ctrl.ReadRecvData(&setuprq, sizeof(setuprq));  // this must be always 8 bytes
		if (r != sizeof(setuprq))
		{
			LTRACE("Setup Request: Invalid size: %i!\r\n", r);
			SendControlStatus(false);
			return;
		}

		ProcessSetupRequest();
	}
	else // DATA IN / DATA OUT or ACK
	{
		if (htod) // host data out
		{
			r = ep_ctrl.ReadRecvData(&ctrlbuf[cdlen], sizeof(ctrlbuf) - cdlen);
			if (r < 0)
			{
				SendControlStatus(false);
				return;
			}

			if (r == 0) // ACK from the host
			{
				//LTRACE("CTRL ACK received.\r\n");
				// there should be only an empty packet
				ctrlstage = USBCTRL_STAGE_SETUP;
				ep_ctrl.EnableRecv();
				return;
			}

			if (ctrlstage != USBCTRL_STAGE_DATAOUT)
			{
				SendControlStatus(false);
				return;
			}

			if (ctrl_datastage_remaining > 0)
			{
				ctrl_datastage_remaining -= r;
				cdlen += r;
			}

			if (ctrl_datastage_remaining > 0)
			{
				// wait the next
				ep_ctrl.EnableRecv();
				return;
			}

			// end of the data stage

			if (HandleSpecialSetupData())
			{
				return;
			}

			int i;
			if ((setuprq.rqtype & 0x1F) == 1) // interface requests
			{
				i = setuprq.index;
				if (i < interface_count)
				{
					if (interfaces[i]->HandleSetupData(&setuprq, &ctrlbuf[0], cdlen))
					{
						return;
					}
				}
				LTRACE("Unhandled interface setup data!\r\n");
				SendControlStatus(false);
				return;
			}

			LTRACE("Unhandled setup data !\r\n");
			SendControlStatus(false);
			return;
		}
		else  // dtoh = host data in
		{
			// device to host send completed

			if (USBCTRL_STAGE_STATUS == ctrlstage) // ACK sent
			{
				//LTRACE("CTRL ack sent.\r\n");
				if (set_devaddr_on_ack)
				{
					LTRACE("device address is set to %i\r\n", devaddr);
					SetDeviceAddress(devaddr);
					set_devaddr_on_ack = false;
				}
				ctrlstage = USBCTRL_STAGE_SETUP;
				ep_ctrl.EnableRecv();
				return;
			}

			if (ctrl_datastage_remaining > 0) // should we continue larger block send ?
			{
				r = ep_ctrl.StartSendData(&ctrlbuf[cdlen - ctrl_datastage_remaining], ctrl_datastage_remaining);
				if (r < 0)
				{
					SendControlStatus(false);
					return;
				}

				LTRACE("... continue send, len = %i\r\n", r);

				ctrl_datastage_remaining -= r;
				return;
			}

			ctrlstage = USBCTRL_STAGE_STATUS;  // wait ACK from the host
			ep_ctrl.EnableRecv();
			return;
		}
	}
}

void TUsbDevice::ProcessSetupRequest()
{
	int i;

#if 1
	LTRACE("SETUP request: %02X %02X  %04X %04X %04X\r\n", setuprq.rqtype, setuprq.request, setuprq.value, setuprq.index, setuprq.length);
#endif

	if (HandleSpecialSetupRequest())
	{
		return;
	}

	if ((setuprq.rqtype & 0x1F) == 0) // recipient = device
	{
		if   (0x06 == setuprq.request) // get descriptor
		{
			uint8_t descid = (setuprq.value >> 8);
			if (3 == descid) // string descriptors have special handling
			{
				uint8_t strid = (setuprq.value & 0xFF);

				if (0 == strid) // sending supported languages
				{
					//TRACE("Sending string table\r\n");
					// supported languages
					// 04 03 09 04
					cdlen = 4;
					ctrlbuf[0] = cdlen;
					ctrlbuf[1] = 3;
					ctrlbuf[2] = 0x09;
					ctrlbuf[3] = 0x04;

					StartSendControlData();
					return;
				}
				else
				{
					// string data
					const char * str = nullptr;
					if (strid < USBDEV_MAX_STRINGS)
					{
						str = stringtable[strid-1];
					}

					if (!str)
					{
						str = "???";
						LTRACE("GETSTRING: string 0x%02X not found\r\n", strid);
					}

					LTRACE("Sending string \"%s\"\r\n", str);

					uint8_t slen = strlen(str);

					if (slen > 31)  slen = 31;

					cdlen = slen * 2 + 2;
					ctrlbuf[0] = cdlen;
					ctrlbuf[1] = 3;

					// convert the string to unicode format
					char * pdst = (char *)&ctrlbuf[2];
					for (i = 0; i < slen; ++i)
					{
						*(pdst++) = *(str++);
						*(pdst++) = 0;
					}

					StartSendControlData();
					return;
				}
			}
			else // normal, static descriptors
			{
				void * descptr = nullptr;
				uint8_t desclen = 0;

				if (USB_DESC_TYPE_DEVICE == descid)
				{
					cdlen = devdesc.length;
					memcpy(&ctrlbuf[0], &devdesc, cdlen);
				}
				else if (USB_DESC_TYPE_DEVICE_QUALIFIER == descid)
				{
					SendControlStatus(false);
					return;
					//txlen = qualifierdesc.length;
					//memcpy(&ctrlbuf[0], &qualifierdesc, cdlen);
				}
				else if ((USB_DESC_TYPE_CONFIGURATION == descid) || (USB_DESC_TYPE_OTHER_SPEED_CONFIGURATION == descid))
				{
					MakeDeviceConfig(); // prepares the ctrlbuf directly
				}
				else
				{
					LTRACE("Unknown device descriptor request: 0x%02X\r\n", descid);
					SendControlStatus(false);
					return;
				}

				if (setuprq.length < cdlen)  cdlen = setuprq.length;  // we can not send more than the size was requested !

				LTRACE("Sending descriptor 0x%02X, len = %i...\r\n", descid, cdlen);

				StartSendControlData();
				return;
			}
		}
		else if (0x05 == setuprq.request) // set address
		{
			devaddr = setuprq.value;
			LTRACE("Set device address: %i\r\n", devaddr);

#ifdef HWUSB_SET_DADDR_BEFORE_ACK

			// On STM32 OTG USB the address must be set before sending the status !

			set_devaddr_on_ack = false;  // the device address must be set only after the ACK sent
			SetDeviceAddress(devaddr);

#else
			// Normally the address must be set only after the status has been sent

			set_devaddr_on_ack = true;  // the device address must be set only after the ACK sent
#endif

			SendControlStatus(true);
			return;
		}
		else if (0x09 == setuprq.request) // set configuration
		{
			SendControlStatus(true);
			SetConfiguration(setuprq.value & 0xFF);
			return;
		}
		else
		{
			LTRACE("Unhandled device setup request\r\n");
			SendControlStatus(false);
			return;
		}
	}
	else if ((setuprq.rqtype & 0x1F) == 1) // interface requests
	{
		i = setuprq.index;
		if (i < interface_count)
		{
			if (interfaces[i]->HandleSetupRequest(&setuprq))
			{
				return;
			}
		}
		LTRACE("Unhandled interface request!\r\n");
		SendControlStatus(false);
	}
	else if ((setuprq.rqtype & 0x1F) == 2) // endpoint requests
	{
		i = setuprq.index;
		if (i < epcount)
		{
			if (eplist[i]->HandleSetupRequest(&setuprq))
			{
				return;
			}
		}
		LTRACE("Unhandled endpoint request!\r\n");
		SendControlStatus(false);
	}
	else
	{
		// unknown request
		LTRACE("Unknown request!\r\n");

		SendControlStatus(false);
		return;
	}
}

void TUsbDevice::SetConfiguration(uint8_t aconfig)
{
	LTRACE("Set configuration: %u\r\n", aconfig);
  actualconfig = aconfig;

  // notify the interfaces
  for (int i = 0; i < interface_count; ++i)
  {
  	interfaces[i]->SetConfigured();
  }
}

void TUsbDevice::MakeDeviceConfig()
{
	cdlen = confdesc.length;
	memcpy(&ctrlbuf[0], &confdesc, cdlen);
	ctrlbuf[1] = (setuprq.value >> 8); // normal or other speed config
	uint8_t ind = cdlen;

	for (int i = 0; i < interface_count; ++i)
	{
		TUsbInterface * intf = interfaces[i];

		// calculate the config descriptor length

		int len = intf->AppendConfigDesc(&ctrlbuf[ind], sizeof(ctrlbuf) - ind);
		if (len <= 0)
		{
			break;
		}

		cdlen += len;
		ind += len;
	}
}

void TUsbDevice::StartSetupData()
{
	ctrlstage = USBCTRL_STAGE_DATAOUT;
	ep_ctrl.EnableRecv();
}

void TUsbDevice::SendControlAck()
{
	ep_ctrl.SendAck();
}

void TUsbDevice::SendControlStatus(bool asuccess)
{
	ctrlstage = USBCTRL_STAGE_STATUS;

	if (asuccess)
	{
		ep_ctrl.SendAck();
	}
	else
	{
		ep_ctrl.Stall();
	}
}

