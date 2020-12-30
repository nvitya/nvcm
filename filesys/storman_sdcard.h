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
 *  file:     storman_sdcard.h
 *  brief:    Storage Manager for SDCARDs
 *  version:  1.00
 *  date:     2020-12-29
 *  authors:  nvitya
*/

#ifndef STORMAN_SDCARD_H_
#define STORMAN_SDCARD_H_

#include "stormanager.h"
#include "hwsdcard.h"

class TStorManSdcard : public TStorManager
{
private:
	typedef TStorManager super;

protected:
	uint32_t       remaining = 0;
	uint8_t *      dataptr = nullptr;
	uint64_t       curaddr = 0;
	uint32_t       chunksize = 0;

	void           StartPartialRead();
	void           ProcessPartialRead();
	void           FinishCurTra();
	void           FinishCurTraError(int aerror);

public:
	THwSdcard *    sdcard = nullptr;

	uint64_t       sdbufaddr = 1;  // address of the buffered sector, 1 = invalid

	uint8_t        sdbuf[512] __attribute__((aligned(16)));  // buffer for the partial reads/writes

	virtual        ~TStorManSdcard() { }

	bool           Init(THwSdcard * asdcard);

	virtual void   Run(); // must be overridden
};

#endif /* STORMAN_SDCARD_H_ */
