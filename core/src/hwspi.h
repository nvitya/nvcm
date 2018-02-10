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
 *  file:     hwspi.h
 *  brief:    Internal SPI vendor-independent definitions
 *  version:  1.00
 *  date:     2018-02-10
 *  authors:  nvitya
 *
 *  notes:
 *     only SPI master mode supported so far
*/

#ifndef HWSPI_H_PRE_
#define HWSPI_H_PRE_

#include "hwpins.h"
#include "hwdma.h"

class TSpiTransfer
{
public:
	unsigned        length;
	void *          src;
	void *          dst;
	TSpiTransfer *  next = nullptr;
};

class THwSpi_pre
{
public:
	int      				devnum = -1;

	unsigned        speed = 1000000;  // default speed = 1MHz
	unsigned char   databits = 8; // frame length
	bool            lsb_first = false;
	bool            idleclk_high = false;
	bool            datasample_late = false;
	bool            inter_frame_pulse = false; // CS pulse between frames

	TGpioPin *			manualcspin = nullptr;

	THwDmaChannel * txdma = nullptr;
	THwDmaChannel * rxdma = nullptr;
};

#endif // ndef HWSPI_H_PRE_

#ifndef HWSPI_PRE_ONLY

//-----------------------------------------------------------------------------

#ifndef HWSPI_H_
#define HWSPI_H_

#include "mcu_impl.h"

#ifndef HWSPI_IMPL

#warning "HWSPI is not implemented!"

class THwSpi_noimpl : public THwSpi_pre
{
public: // mandatory
	bool Init(int adevnum)        { return false; }

	bool TrySendData(unsigned short adata)     { return false; }
	bool TryRecvData(unsigned short * dstptr)  { return false; }
	bool SendFinished()                        { return true; }

	void DmaAssign(bool istx, THwDmaChannel * admach)  { }

	bool DmaStartSend(THwDmaTransfer * axfer)  { return false; }
	bool DmaStartRecv(THwDmaTransfer * axfer)  { return false; }
	bool DmaSendCompleted()  { return true; }
	bool DmaRecvCompleted()  { return false; }
};

#define HWSPI_IMPL   THwSpi_noimpl

#endif // ndef HWSPI_IMPL

//-----------------------------------------------------------------------------

class THwSpi : public HWSPI_IMPL
{
public:
	void            BeginTransaction(); // puts the chip select low
	void            EndTransaction(); // puts the chip select high

	int             RunTransfer(TSpiTransfer * axfer);

	inline void     SendData(unsigned short adata) { while (!TrySendData(adata)) {} }; // wait until it successfully started, but does not wait to finish !
	void            WaitSendFinish();
};

#endif // ndef HWSPI_H_ */

#else
  #undef HWUART_PRE_ONLY
#endif
