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
 *  file:     hwspi_xmc.h
 *  brief:    XMC SPI (master only)
 *  version:  1.00
 *  date:     2018-05-09
 *  authors:  nvitya
*/

#ifndef HWSPI_XMC_H_
#define HWSPI_XMC_H_

#define HWSPI_PRE_ONLY

#include "hwdma.h"
#include "hwspi.h"

class THwSpi_xmc : public THwSpi_pre
{
public:
	// additional XMC specific settings
	uint8_t             selonum = 0;
	int                 inputpin = 0;

	bool Init(int ausicnum, int achnum, int ainputpin);

	inline bool TrySendData(uint16_t adata)
	{
		if (regs->TRBSR & (1 << 12))  // is the Transmit FIFO full?
		{
			return false;
		}
		regs->IN[0] = adata; // put the character into the transmit fifo
		return true;
	}

	inline bool TryRecvData(uint16_t * dstptr)
	{
		if (regs->TRBSR & (1 << 3))  // is Receive buffer empty?
		{
			return false;
		}
		*dstptr = regs->OUTR;
		return true;
	}

	bool SendFinished();

	bool DmaStartSend(THwDmaTransfer * axfer) { return false; }
	bool DmaStartRecv(THwDmaTransfer * axfer) { return false; }

public:
	int                 usicnum = 0;
	int                 chnum = 0;


	HW_SPI_REGS * 			regs;
};


#define HWSPI_IMPL THwSpi_xmc

#endif // def HWSPI_XMC_H_
