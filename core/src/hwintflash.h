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
 *  file:     hwintflash.h
 *  brief:    Interface for Internal Flash Memory Handling
 *  version:  1.00
 *  date:     2019-02-23
 *  authors:  nvitya
*/

#ifndef _HWINTFLASH_H_PRE_
#define _HWINTFLASH_H_PRE_

#include "platform.h"
#include "hwpins.h"

#define INTFLASH_STATE_WRITEMEM  2
#define INTFLASH_STATE_ERASE     3
#define INTFLASH_STATE_COPY      8

#define INTFLASH_ERROR_NOTINIT   -1
#define INTFLASH_ERROR_BUSY      -4
#define INTFLASH_ERROR_ADDRESS   -100

class THwIntFlash_pre
{
public:	// info
	bool 					 initialized = false;

	uint32_t       bytesize = 0;       // total size
	uint32_t       erasesize = 0;      // erase unit, in bytes
	uint32_t       pagesize = 0;       // max write unit in bytes
	uint32_t       smallest_write = 0;

	uint32_t       start_address = 0;
	uint32_t       bank_count = 0;

public: // status
	bool           completed = true;
	int            errorcode = 0;

protected: // internal

	int            state = 0;
	int            phase = 0;

	uint32_t       address = 0;
	uint32_t       length = 0;

	uint32_t       remaining;
	uint32_t *     srcaddr;
	uint32_t *     dstaddr;

	uint32_t       pagemask = 0;
	uint32_t       blockmask = 0;

	uint32_t       chunksize = 0;

	uint32_t       ebchunk;
	uint32_t       ebremaining;
};

#endif // ndef _HWINTFLASH_H_PRE_

#ifndef HWINTFLASH_PRE_ONLY

//-----------------------------------------------------------------------------

#ifndef HWINTFLASH_H_
#define HWINTFLASH_H_

#include "mcu_impl.h"

#ifndef HWINTFLASH_IMPL

class THwIntFlash_noimpl : public THwIntFlash_pre
{
public: // mandatory
	bool           HwInit()  { return false; }

	void           CmdEraseBlock()       { }
	void           CmdWritePage()        { }
	void           CmdClearPageBuffer()  { }
	bool           CmdFinished()         { return true; }
};

#define HWINTFLASH_IMPL   THwIntFlash_noimpl

#endif // ndef HWINTFLASH_IMPL

//-----------------------------------------------------------------------------

class THwIntFlash : public HWINTFLASH_IMPL
{
public:
	bool           Init();

	void 					 WaitForComplete();

	void           TraceFlashInfo();

	bool 					 StartEraseMem(uint32_t aaddr, uint32_t alen);  // aaddr must be block aligned!
	bool 					 StartWriteMem(uint32_t aaddr, void * asrcptr, uint32_t alen); // alen, aaddr must be 4 aligned

	// This function includes automatic compare and erase:
	// Intended for self-flashing RAM applications
	bool           StartCopyMem(uint32_t aaddr, void * asrcptr, uint32_t alen);  // aaddr must be block aligned!

#ifndef HWINTFLASH_OWN_RUN
	void           Run();
#endif
};

extern THwIntFlash  hwintflash;

#endif // HWINTFLASH_H_

#else
  #undef HWINTFLASH_PRE_ONLY
#endif
