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

#include "string.h"
#include "stormanager.h"
#include "traces.h"

void TStorManager::Init(uint8_t * apbuf, unsigned abufsize)
{
  pbuf = apbuf;
  bufsize = abufsize;

	firsttra = nullptr;
	lasttra = nullptr;
	state = 0;
}

void TStorManager::AddTransaction(TStorTrans * atra)
{
	atra->next = nullptr;
	atra->completed = false;
	atra->errorcode = 0;

	if (lasttra)
	{
		lasttra->next = atra;
		lasttra = atra;
	}
	else
	{
		// set as first
		firsttra = atra;
		lasttra = atra;
	}
}

void TStorManager::AddTransaction(TStorTrans * atra, TStorTransType atype, uint64_t aaddr,
		                             void * adataptr, uint32_t adatalen)
{
	atra->trtype = atype;
	atra->address = aaddr;
	atra->dataptr = (uint8_t *)adataptr;
	atra->datalen = adatalen;

	AddTransaction(atra);
}

void TStorManager::WaitTransaction(TStorTrans * atra)
{
	while (!atra->completed)
	{
		Run();
	}
}

void TStorManager::Run()
{
	// must be overridden

}

void TStorManager::ExecCallback(TStorTrans * atra)
{
	if (atra->callback)
	{
		(* (atra->callback))(atra->callbackarg);
	}
}
