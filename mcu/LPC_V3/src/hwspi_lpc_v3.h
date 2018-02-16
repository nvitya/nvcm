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
 *  file:     hwspi_lpc_v3.h
 *  brief:    LPC_V3 SPI (master only)
 *  version:  1.00
 *  date:     2018-02-10
 *  authors:  nvitya
*/

#ifndef HWSPI_LPC_V3_H_
#define HWSPI_LPC_V3_H_

#define HWSPI_PRE_ONLY
#include "hwspi.h"

class THwSpi_lpc_v3 : public THwSpi_pre
{
public:
	bool Init(int adevnum);

	bool TrySendData(uint16_t adata);
	bool TryRecvData(uint16_t * dstptr);
	bool SendFinished();

	void DmaAssign(bool istx, THwDmaChannel * admach);

	bool DmaStartSend(THwDmaTransfer * axfer);
	bool DmaStartRecv(THwDmaTransfer * axfer);
	bool DmaSendCompleted();
	bool DmaRecvCompleted();

public:
	unsigned  					basespeed;
	HW_SPI_REGS * 			regs;
};


#define HWSPI_IMPL THwSpi_lpc_v3

#endif // def HWSPI_LPC_V3_H_
