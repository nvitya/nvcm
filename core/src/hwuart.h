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
 *  file:     hwuart.h
 *  brief:    Internal UART/USART vendor independent definitions
 *  version:  1.00
 *  date:     2018-02-10
 *  authors:  nvitya
*/

#ifndef HWUART_H_PRE_
#define HWUART_H_PRE_

#include "platform.h"
#include "hwdma.h"

#define FMT_BUFFER_SIZE  256

class THwUart_pre
{
public:
	int     devnum = -1;

	bool    initialized = false;

	int     baudrate = 115200;
	int     databits = 8;
	int     halfstopbits = 2;
	bool    parity = false;
	bool    oddparity = false;

public:  // DMA
	THwDmaChannel *      txdma = nullptr;
	THwDmaChannel *      rxdma = nullptr;
};

#endif // ndef HWUART_H_PRE_

#ifndef HWUART_PRE_ONLY

//-----------------------------------------------------------------------------

#ifndef HWUART_H_
#define HWUART_H_

#include "mcu_impl.h"

#ifndef HWUART_IMPL

#warning "HWUART is not implemented!"

class THwUart_noimpl : public THwUart_pre
{
public: // mandatory
	bool Init(int adevnum)        { return false; }

	bool TrySendChar(char ach)    { return false; }
	bool TryRecvChar(char * ach)  { return false; }
	bool SendFinished()           { return true; }

	void DmaAssign(bool istx, THwDmaChannel * admach)  { }

	bool DmaStartSend(THwDmaTransfer * axfer)  { return false; }
	bool DmaStartRecv(THwDmaTransfer * axfer)  { return false; }
	bool DmaSendCompleted()  { return true; }
	bool DmaRecvCompleted()  { return false; }
};

#define HWUART_IMPL   THwUart_noimpl

#endif // ndef HWUART_IMPL

//-----------------------------------------------------------------------------

class THwUart : public HWUART_IMPL
{
public:

	void SendChar(char ach) { while (!TrySendChar(ach)) {} }

	bool DmaSendCompleted();
	bool DmaRecvCompleted();

	void printf(const char * fmt, ...);
};

#endif /* HWUART_H_ */

#else
  #undef HWUART_PRE_ONLY
#endif
