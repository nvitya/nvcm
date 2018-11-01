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

//-------------------------------------------------------------------------------------------

void THwUsbCtrl::HandleReset()
{
#if 0
	ResetEndpoints();

	// enable RX on the EP0:
	epdef = epbyid[0];
	set_epreg_rx_status(epdef->preg, 3); // enabled
#endif
}

int THwUsbEndpoint::StartSend(void * buf, unsigned len)
{
	if (tx_remaining_len > 0)
	{
		return USBERR_TX_OVERWRITE;
	}

	tx_remaining_dataptr = (uint8_t *)buf;
	tx_remaining_len = len;

	return SendRemaining();
}

