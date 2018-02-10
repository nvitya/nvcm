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
 *  file:     serialflash.h
 *  brief:    Serial flash memory base class
 *  version:  1.00
 *  date:     2018-02-10
 *  authors:  nvitya
*/

#ifndef SERIALFLASH_H_
#define SERIALFLASH_H_

#include "platform.h"
#include "hwpins.h"
#include "hwspi.h"
#include "hwdma.h"
#include "errors.h"

#define SERIALFLASH_STATE_READMEM   1
#define SERIALFLASH_STATE_WRITEMEM  2
#define SERIALFLASH_STATE_ERASE     3
#define SERIALFLASH_STATE_ERASEALL  4

class TSerialFlash
{
public:

public: // settings
	unsigned       has4kerase = false;

public:
	unsigned       idcode = 0;
	unsigned       bytesize = 0; // auto-detected from JEDEC ID

	bool 					 initialized = false;
	bool           completed = true;
	int            errorcode = 0;

	virtual        ~TSerialFlash() {} // to avoid warnings (never destructed)

	bool   				 Init();
	virtual bool   InitInherited();
	virtual bool   InitInterface();

	bool 					 StartReadMem(unsigned aaddr, void * adstptr, unsigned alen);
	bool 					 StartEraseMem(unsigned aaddr, unsigned alen);
	bool 					 StartEraseAll();
	bool 					 StartWriteMem(unsigned aaddr, void * asrcptr, unsigned alen); // must be erased before

	virtual bool   ReadIdCode();

	virtual void   Run(); // must be overridden !
	void 					 WaitForComplete();

protected:
	// state machine
	int            state = 0;
	int            phase = 0;

	unsigned       chunksize = 0;
	unsigned       maxchunksize = HW_DMA_MAX_COUNT;

	uint8_t *      dataptr = nullptr;
	unsigned       datalen = 0;
	unsigned       address = 0;
	unsigned       remaining = 0;
	unsigned       erasemask = 0;

};


#endif /* SERIALFLASH_H_ */
