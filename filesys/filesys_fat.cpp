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
 *  file:     filesys_fat.cpp
 *  brief:    FAT File System
 *  version:  1.00
 *  date:     2020-12-29
 *  authors:  nvitya
*/

#include "string.h"
#include "stormanager.h"
#include <filesys_fat.h>
#include "traces.h"
#include <new> // required for placement new

TFileFat::TFileFat(TFileSysFat * afilesys)
  : super(afilesys)
{
	//super::TFile(afilesys);
}

//--------------------------------------------------------------------------------------------

TFile * TFileSysFat::NewFileObj(void * astorage, unsigned astoragesize)
{
	TFile * result = nullptr;
	if (astorage)
	{
		if (astoragesize < sizeof(TFileFat))
		{
			return nullptr;
		}

		uint8_t * saddr = (uint8_t *)astorage;
		unsigned salign = (unsigned(saddr) & 0xF);
		if (salign) // wrong aligned address, objects require 16 byte aligment!
		{
			if (astoragesize < sizeof(TFileFat) + (16 - salign))
			{
				return nullptr;
			}
			saddr += (16 - salign);
		}

	  TFile * pfile = (TFile *)saddr;

    result = new ((TFileFat *)pfile) TFileFat(this);
    result->allocated_on_heap = false;
	}
	else
	{
    result = new TFileFat(this);
    result->allocated_on_heap = true;
	}

	return result;
}

void TFileSysFat::HandleInitState()
{
	// called only when stra.completed == true and stra.errorcode == 0

	if (0 == initstate)  // read the boot sector
	{
		pstorman->AddTransaction(&stra, STRA_READ, firstaddr,  &buf[0], 512);
		initstate = 1;
	}
	else if (1 == initstate)  // process the boot sector
	{
		fat32 = false;
		fat12 = false;

		// file system check from ChaN's FatFS:

		bool bok = true;

		if ( (0xAA55 != *(uint16_t *)&buf[510])  // check for the valid signature
				 || ( (buf[0] != 0xE9) && ((buf[0] != 0xEB) || (buf[2] != 0x90)) )  // check jump code
				 )
		{
			bok = false;
		}

		// check file system name
		if (bok)
		{
			if ( ((*(uint32_t *)&buf[82]) & 0xFFFFFFFF) == 0x33544146 ) // FAT3 ?
			{
				fat32 = true;
			}
			else if ( ((*(uint32_t *)&buf[54]) & 0xFFFFFF)   !=   0x544146 ) // FAT ?
			{
				bok = false;
			}
		}

		// check sector size
		if (bok && (*(uint16_t *)&buf[0x0B] != 512))
		{
			bok = false;
		}

		if (bok)
		{
			fatcount = buf[0x010];
			clusterbytes = (buf[0x0D] << 9);
			clustersizeshift = 31 - __CLZ(clusterbytes);
			cluster_reminder_mask = uint64_t((1 << clustersizeshift) - 1);
			cluster_start_mask = ~cluster_reminder_mask;

			rootdirbytes = (*(uint16_t *)&buf[0x11] << 5);  // 32 byte / entry

			totalbytes = (*(uint16_t *)&buf[0x13] << 9);
			if (!totalbytes)
			{
				totalbytes = (uint64_t(*(uint32_t *)&buf[0x20]) << 9);
			}

			reservedbytes = (*(uint16_t *)&buf[0x0E] << 9);

			if (fat32)
			{
				fatbytes = (*(uint32_t *)&buf[0x24] << 9);
			}
			else
			{
				fatbytes = (*(uint16_t *)&buf[0x16] << 9);
			}

			sysbytes = reservedbytes + rootdirbytes + fatcount * fatbytes;
			rootdirstart =  firstaddr + sysbytes;
			databytes = totalbytes - sysbytes;
			clustercount = (databytes >> clustersizeshift);

			if (!fat32 && (clustercount < 0xFF5))
			{
				fat12 = true;
			}
		}

		if (reservedbytes < 512)
		{
			bok = false;
		}

		if (! bok)
		{
			fsok = false;
			initialized = true;
			return;
		}

		// no more initialization yet.
		fsok = true;
		initialized = true;
	}
}

void TFileSysFat::RunOpDirRead()
{
	// called only when stra.completed == true and stra.errorcode == 0

	if (5 == opstate) // wait for FAT resolution
	{
		TRACE("FAT next cluster = %u\r\n", next_cluster);
		if ((next_cluster < 2) || (next_cluster >= clustercount))
		{
			FinishCurTra(FSRESULT_EOF);
			return;
		}

		op_location = ClusterToAddr(next_cluster);
		op_cluster_end = op_location + clusterbytes;
		opstate = 0; // go on with sector read
	}

	if (0 == opstate)
	{
		if (op_location < op_cluster_end)
		{
			// cache the 512 byte of the directory sector
			// todo: handle long file names

			sectoraddr = (op_location & sector_base_mask);

			if (bufaddr != sectoraddr)
			{
				// read the sector
				//TRACE("[r %08X]", uint32_t(sectoraddr));
				pstorman->AddTransaction(&stra, STRA_READ, sectoraddr,  &buf[0], 512);
				opstate = 1;
				return;
			}

			opstate = 2;  // the sector is already there
		}
		else // cluster end reached
		{
			// resolve FAT chain
			uint32_t curcluster = AddrToCluster(op_location) - 1;
			TRACE("FAT find next cluster of %u\r\n", curcluster);

			FindNextCluster(curcluster);
			opstate = 5;
			return;
		}
	}
	else if (1 == opstate)  // wait for sector read
	{
		bufaddr = sectoraddr;
		opstate = 2;
	}

	if (2 == opstate) // the sector is in buf[]
	{
		bufendaddr = bufaddr + 512;

		while (op_location < bufendaddr)
		{
			uint64_t dirlocation = op_location;
			TFsFatDirEntry * pdire = (TFsFatDirEntry *)&buf[dirlocation & 0x1FF];
			if (0 == pdire->name[0])
			{
				// 0 at signalizes the end of the directory (and a free entry)
				FinishCurOp(FSRESULT_EOF);
				return;
			}

			op_location += sizeof(TFsFatDirEntry);  // advance to the next location

			bool bok = true;
			if ((0x05 == pdire->name[0]) || (0xE5 == pdire->name[0])) // is the entry deleted ?
			{
				bok = false;
			}
			else if (0x0F == pdire->attr) // is it a long file name chunk ?
			{
				bok = false;
			}

			if (bok)
			{
				// convert the directory entry to the unified format
				ConvertDirEntry(pdire, &fdata, dirlocation);
				FinishCurOp(0);
				return;
			}
		}

		// this sector is exhausted, load the next one.

		opstate = 0;
	}
}

void TFileSysFat::HandleFileRead()
{
	// called only when curop == FSOP_IDLE and stra.competed without error

	if (1 == trastate) // wait for chunk read finish
	{
		curtra->curlocation += chunksize;
		curtra->dataptr += chunksize;
		curtra->transferlen += chunksize;
		curtra->filepos += chunksize;
		curtra->remaining -= chunksize;
	}

	if (0 == curtra->remaining)
	{
		FinishCurTra(0);
		return;
	}

	if (5 == trastate) // wait for FAT next cluster
	{
		TRACE("FAT next cluster = %u\r\n", next_cluster);
		if ((next_cluster < 2) || (next_cluster >= clustercount))
		{
			FinishCurTra(FSRESULT_EOF);
			return;
		}

		curtra->curlocation = ClusterToAddr(next_cluster);
		curtra->cluster_end = curtra->curlocation + clusterbytes;
		trastate = 0; // go on with normal read
	}

	// process the next chunk
	// the curlocation must point to a valid file segment !
	chunksize = (curtra->cluster_end - curtra->curlocation);
	if (0 == chunksize)
	{
		// FAT chain resolution required
		uint32_t curcluster = AddrToCluster(curtra->curlocation) - 1;
		TRACE("FAT find next cluster of %u\r\n", curcluster);

		FindNextCluster(curcluster);
		trastate = 5;
		return;
	}

	if (chunksize > curtra->remaining)
	{
		chunksize = curtra->remaining;
	}

	pstorman->AddTransaction(&stra, STRA_READ, curtra->curlocation,  curtra->dataptr, chunksize);
	trastate = 1;
	return;
}

void TFileSysFat::HandleFileSeek()
{
	// called only when curop == FSOP_IDLE and stra.competed without error

	if (5 == trastate) // wait for FAT next cluster
	{
		TRACE("FAT next cluster = %u\r\n", next_cluster);
		if ((next_cluster < 2) || (next_cluster >= clustercount))
		{
			FinishCurTra(FSRESULT_EOF);
			return;
		}

		curtra->filepos += clusterbytes;
		curtra->curlocation = ClusterToAddr(next_cluster);
		curtra->cluster_end = curtra->curlocation + clusterbytes;
	}

	if (curtra->filepos + clusterbytes >= curtra->targetpos)  // include the cluster end
	{
		curtra->curlocation += (curtra->targetpos & cluster_reminder_mask);
		curtra->filepos = curtra->targetpos;
		FinishCurTra(0);
		return;
	}

	// go to the next cluster
	uint32_t curcluster = AddrToCluster(curtra->curlocation);
	TRACE("FAT find next cluster of %u\r\n", curcluster);
	FindNextCluster(curcluster);
	trastate = 5;
}

void TFileSysFat::ConvertDirEntry(TFsFatDirEntry * pdire, TFileDirData * pfdata, uint64_t adirlocation)
{
	pfdata->size = pdire->size;
	pfdata->location = ClusterToAddr(pdire->cluster_low + (pdire->cluster_high << 16));
	pfdata->dirlocation = adirlocation;

	// attributes
	pfdata->attributes = (pdire->attr << 16); // keep the original attributes
	if (pdire->attr & 0x10)
	{
		pfdata->attributes |= FSATTR_DIR;
	}

	if (pdire->attr & 0x08)
	{
		pfdata->attributes |= FSATTR_VOLLABEL;
	}

	//pfdata->create_time = 0; // todo: implement
	//pfdata->modif_time = 0; // todo: implement

	// 8+3 name:
	char * dp = &pfdata->name[0];
	char * sp = &pdire->name[0];
	char * endp = &pdire->name[8];
	while ((sp < endp) && (*sp > 32))
	{
		*dp++ = *sp++;
	}
	sp = &pdire->name[8];
	endp = &pdire->name[11];
	if (*sp > 32)
	{
		*dp++ = '.';
	}
	while ((sp < endp) && (*sp > 32))
	{
		*dp++ = *sp++;
	}
	*dp = 0; // zero terminate
}

uint64_t TFileSysFat::ClusterToAddr(uint32_t acluster)
{
	if ((acluster < 2) || (acluster >= clustercount))
	{
		return FS_INVALID_ADDR;
	}
	uint64_t res = firstaddr + sysbytes + (uint64_t(acluster - 2) << clustersizeshift);
	//TRACE("ClusterToAddr(%u)=%llu\r\n", acluster, res);
	return res;
}

void TFileSysFat::FindNextCluster(uint32_t acluster)
{
	pstorman->AddTransaction(&stra, STRA_READ, firstaddr + reservedbytes + (acluster << 2),  &next_cluster, 4);
}

uint32_t TFileSysFat::AddrToCluster(uint64_t aaddr)
{
	if (aaddr < firstaddr + sysbytes)
	{
		return 0;
	}
	uint32_t res = 2 + ((aaddr - (firstaddr + sysbytes)) >> clustersizeshift);
	if (res > clustercount)
	{
		return 0;
	}
	//TRACE("AddrToCluster(%llu)=%u\r\n", aaddr, res);
	return res;
}


