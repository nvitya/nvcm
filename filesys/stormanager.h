/* -----------------------------------------------------------------------------
 * This file is a part of the NVCM Tests project: https://github.com/nvitya/nvcmtests
 * Copyright (c) 2020 Viktor Nagy, nvitya
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
 *  file:     stormanager.cpp
 *  brief:    Storage Manager, transaction manager for non-volatile storage devices (SDCARD, Serial FLASH)
 *  version:  1.00
 *  date:     2020-12-29
 *  authors:  nvitya
*/

#ifndef STORMANAGER_H_
#define STORMANAGER_H_

#include "clockcnt.h"

#define ESTOR_NOTIMPL    1
#define ESTOR_INV_SIZE   2

typedef enum
{
	STRA_READ,
	STRA_WRITE,
	STRA_ERASE
//
} TStorTransType;

typedef void (* PStorCbFunc)(void * arg);

struct TStorTrans
{
	bool              completed;
	TStorTransType    trtype;

	uint8_t *         dataptr;  // usually DMA target, so better be 8-byte aligned (IXMRT QSPI requirement)
	unsigned          datalen;
	uint64_t          address;

	int               errorcode;

	PStorCbFunc       callback = nullptr;
	void *            callbackarg = nullptr;

	TStorTrans *      next = nullptr;
//
};

class TStorManager
{
public:
	int               state = 0;

	TStorTrans *      firsttra = nullptr;
	TStorTrans *      lasttra = nullptr;

	TStorTrans *      curtra = nullptr;

	uint8_t *         pbuf = nullptr;
	unsigned          bufsize = 0;

	unsigned          erase_unit = 4096;
	unsigned          smallest_block = 1;  // must be power of two !

	virtual ~TStorManager() { }

	void Init(uint8_t * apbuf, unsigned abufsize);

	void AddTransaction(TStorTrans * atra);

	void AddTransaction(TStorTrans * atra, TStorTransType atype, uint64_t aaddr,
			                void * adataptr, uint32_t adatalen);

	void WaitTransaction(TStorTrans * atra); // blocking, only for initializations!

public:  // virtual functions

	virtual void  Run(); // must be overridden

protected:

	unsigned          trastarttime = 0;

	void              ExecCallback(TStorTrans * atra);
};

#endif /* STORMANAGER_H_ */
