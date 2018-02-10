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
 *  file:     hwspi.cpp
 *  brief:    Internal SPI vendor-independent implementations
 *  version:  1.00
 *  date:     2018-02-10
 *  authors:  nvitya
 *
 *  notes:
 *     only SPI master mode supported so far
*/

#include "platform.h"
#include "hwspi.h"

void THwSpi::BeginTransaction()
{
	if (manualcspin)
	{
		manualcspin->Set0();
	}
}

void THwSpi::EndTransaction()
{
	if (manualcspin)
	{
		manualcspin->Set1();
	}
}

void THwSpi::WaitSendFinish()
{
	while (!SendFinished())
	{
		// wait
	}
}

int THwSpi::RunTransfer(TSpiTransfer * axfer)
{
	int result = 0;

	TSpiTransfer * xfer = axfer;

	while (xfer)
	{
		if (xfer->length > 0)
		{
			// sending and receiving
			unsigned char * psrc = (unsigned char *)xfer->src;
			unsigned char * pdst = (unsigned char *)xfer->dst;
			unsigned txremaining = xfer->length;
			unsigned rxremaining = xfer->length;
			unsigned short rxdata;

			while ((txremaining > 0) || (rxremaining > 0))
			{
				if (txremaining > 0)
				{
					if (psrc)
					{
						if (TrySendData(*psrc))
						{
							++psrc;
							--txremaining;
						}
					}
					else if (TrySendData(0))
					{
						--txremaining;
					}
				}

				if ((rxremaining > 0) && (TryRecvData(&rxdata)))
				{
					if (pdst)
					{
						*pdst = rxdata;
						++pdst;
					}
					--rxremaining;
				}
			}

			result += xfer->length;
		}
		xfer = xfer->next;
	}

	return result;
}
