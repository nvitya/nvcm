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
 *  file:     filesys_fat.h
 *  brief:    FAT File System
 *  version:  1.00
 *  date:     2020-12-29
 *  authors:  nvitya
*/

#ifndef FILESYS_FAT_H_
#define FILESYS_FAT_H_

#include "filesystem.h"

class TFileSysFat : public TFileSystem
{
public:
	uint8_t       fatcount = 0;
	bool          fat32 = false;
	bool          fat12 = false;
	uint8_t       clustersizeshift = 0;

	uint64_t      totalbytes = 0;
	uint64_t      databytes = 0;

	uint32_t      sysbytes = 0;
	uint32_t      reservedbytes = 0;
	uint32_t      rootdirbytes = 0;

	uint32_t      clustercount = 0;
	uint32_t      clusterbytes = 0;
	uint32_t      fatbytes = 0;

	uint8_t       buf[512] __attribute__((aligned(16)));

	virtual ~TFileSysFat() { }

	virtual void  HandleTransactions();
	virtual void  HandleInitState();
};

#endif /* FILESYS_FAT_H_ */
