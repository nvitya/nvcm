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

struct TFsFatDirEntry
{
	char          name[11];      // 0x00:  8 + 3 format padded with space
	uint8_t       attr;          // 0x0B
	uint8_t       attr_ex;       // 0x0C
	uint8_t       dffc_ctime_hr; // 0x0D deleted file first char / create time 10 ms
	uint16_t      ctime;         // 0x0E
	uint16_t      cdate;         // 0x10
	uint16_t      last_acc_date; // 0x12
	uint16_t      cluster_high;  // 0x14
	uint16_t      mtime;         // 0x16
	uint16_t      mdate;         // 0x18
	uint16_t      cluster_low;   // 0x1A
	uint32_t      size;          // 0x1C
};

class TFileSysFat;

class TFileFat : public TFile
{
private:
	typedef TFile super;

public:
	              TFileFat(TFileSysFat * afilesys);
};

class TFileSysFat : public TFileSystem
{
public:
	uint8_t       fatcount = 0;
	bool          fat32 = false;
	bool          fat12 = false;

	uint64_t      totalbytes = 0;
	uint64_t      databytes = 0;

	uint32_t      sysbytes = 0;
	uint32_t      reservedbytes = 0;
	uint32_t      rootdirbytes = 0;

	uint32_t      clustercount = 0;
	uint32_t      fatbytes = 0;

	uint64_t      sectoraddr = 0; // used internally
	uint64_t      sectorend = 0; // used internally

	uint64_t      bufaddr = 1; // invalid
	uint64_t      bufendaddr = 1; // invalid
	uint8_t       buf[512] __attribute__((aligned(16)));

public:
	virtual       ~TFileSysFat() { }

public: // overrides

	virtual TFile *  NewFileObj(void * astorage, unsigned astoragesize);

	virtual void     HandleInitState();

	virtual void     RunOpDirRead();
	virtual void     HandleFileRead();
	virtual void     HandleFileSeek();

protected:
	uint32_t      next_cluster = 0;  // fat resolution target

	void          FindNextCluster(uint32_t acluster);

	void          ConvertDirEntry(TFsFatDirEntry * pdire, TFileDirData * pfdata, uint64_t adirlocation);
	uint64_t      ClusterToAddr(uint32_t acluster);
	uint32_t      AddrToCluster(uint64_t aaddr);
};

#endif /* FILESYS_FAT_H_ */
