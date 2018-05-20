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

bool THwUsbEndpoint::Init(THwUsbCtrl * ausbctrl, uint8_t aid, uint16_t abufsize, uint32_t aflags)
{
	usbctrl = ausbctrl;
	id = aid;
	bufsize = abufsize;
	flags = aflags;
	dir_hido = ((aflags & USBEF_DIR_HIDO) != 0);

	return true;
}

int THwUsbEndpoint::Recv(void* buf, unsigned len, unsigned flags)
{
}

int THwUsbEndpoint::Send(void* buf, unsigned len, unsigned flags)
{
}

//-------------------------------------------------------------------------------------------

bool THwUsbCtrl::AssignEndpoint(THwUsbEndpoint * aep, uint8_t aid, uint16_t abufsize, uint32_t aflags)
{
	if (aid >= USB_MAX_ENDPOINTS)
	{
		return false;
	}

	aep->Init(this, aid, abufsize, aflags);

	//ConfigureEndpoint(aep);

	if (aflags & USBEF_DIR_HIDO)  // only for HIDO can be tested !
	{
		ep_hin[aid] = aep;
	}
	else
	{
		ep_hout[aid] = aep;
	}
}

