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
 *  brief:    USB Controller (device only) vendor-independent implementations
 *  version:  1.00
 *  date:     2018-05-18
 *  authors:  nvitya
*/

#include "hwusbctrl.h"
#include "platform.h"

bool THwUsbEndpoint::Init(THwUsbCtrl * ausbctrl, uint8_t aid, uint16_t atxbufsize, uint16_t arxbufsize, uint32_t aflags)
{
	usbctrl = ausbctrl;
	id = aid;
	txbufsize = atxbufsize;
	rxbufsize = atxbufsize;
	flags = aflags;

	return true;
}

//-------------------------------------------------------------------------------------------

bool THwUsbCtrl::Init()
{
	int i;

	initialized = false;

	for (i = 0; i < USB_MAX_ENDPOINTS; ++i)
	{
		epbyid[i] = nullptr;
	}

	if (!InitHw())
	{
		return false;
	}

	initialized = true;
	return true;
}

bool THwUsbCtrl::AddEndpoint(THwUsbEndpoint * aep, uint8_t aid, uint16_t atxbufsize, uint16_t arxbufsize, uint32_t aflags)
{
	if (aid >= USB_MAX_ENDPOINTS)
	{
		return false;
	}

	aep->Init(this, aid, atxbufsize, arxbufsize, aflags);
	aep->Configure(); // sets registers, allocates buffer

  epbyid[aid] = aep;

  return true;
}

bool THwUsbCtrl::HandleData(uint8_t epid, bool isrx)
{
	if (epid >= USB_MAX_ENDPOINTS)
	{
		return false;
	}

	THwUsbEndpoint * ep = epbyid[epid];
	if (!ep)
	{
		return false;
	}

	// ...

#if 0
	if (isrx)
	{
		pusbdev->on_data_received(epid);

		clear_epreg_ctr_rx(epdef->preg);

		set_epreg_rx_status(epdef->preg, 3);
	}
	else
	{
		clear_epreg_ctr_tx(epdef->preg);

		if (epdef->tx_remaining_len > 0)
		{
			//strace("...continue sending: %i rem.\r\n", epdef->tx_remaining_len);
			ep_send_remaining(epid);
		}
		else
		{
			// signalize transfer end
			pusbdev->on_data_sent(epid);
		}
	}
#endif

  return true;
}

void THwUsbCtrl::HandleReset()
{
#if 0
	ResetEndpoints();

	// enable RX on the EP0:
	epdef = epbyid[0];
	set_epreg_rx_status(epdef->preg, 3); // enabled
#endif
}
