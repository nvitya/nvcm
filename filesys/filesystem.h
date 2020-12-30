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
 *  file:     filesystem.h
 *  brief:    File System Base Class
 *  version:  1.00
 *  date:     2020-12-29
 *  authors:  nvitya
*/

#ifndef FILESYSTEM_H_
#define FILESYSTEM_H_

#include "stdint.h"
#include "stormanager.h"

struct TMbrPtEntry
{
	uint8_t      status;
	uint8_t      first_h;
	uint8_t      first_s;
	uint8_t      first_c;
	uint8_t      ptype;
	uint8_t      last_h;
	uint8_t      last_s;
	uint8_t      last_c;

	uint32_t     first_lba;     // warning, misaligned for 32 bit read (exception on cortex-m0)
	uint32_t     sector_count;  // warning, misaligned for 32 bit read (exception on cortex-m0)
//
} __attribute__((packed));

struct TMasterBootRecord // aka MBR
{
	uint8_t      bootstrap_code[446];

	TMbrPtEntry  ptentry[4];  // warning: 16 byte / entry but misaligned

	uint16_t     signature;   // should be 0xAA55
//
} __attribute__((packed));


class TFileSystem
{
public:
	bool             initialized = false;
	bool             fsok = false;

	int              initstate = 0;

	TStorManager *   pstorman = nullptr;
	uint64_t         firstaddr = 0;
	uint64_t         maxsize = 0;

	TStorTrans       stra;

	virtual ~TFileSystem() { }

	void Init(TStorManager * astorman, uint64_t afirstaddr, uint64_t amaxsize);

	virtual void Run();

	virtual void HandleTransactions();
	virtual void HandleInitState();
};

#endif /* FILESYSTEM_H_ */
