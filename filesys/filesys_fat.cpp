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

#include <filesys_fat.h>
#include "traces.h"

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
			cluster_base_mask = ~uint64_t((1 << clustersizeshift) - 1);

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

void TFileSysFat::HandleDirRead()
{
	// called only when stra.completed == true and stra.errorcode == 0

	TFsTransDir * tradir = (TFsTransDir *)curtra;

	if (5 == state) // wait for FAT resolution
	{
		TRACE("FAT next cluster = %u\r\n", next_cluster);
		if ((next_cluster < 2) || (next_cluster >= clustercount))
		{
			FinishCurTra(FSRESULT_EOF);
			return;
		}

		tradir->curlocation = ClusterToAddr(next_cluster);
		tradir->cluster_end = tradir->curlocation + clusterbytes;
		state = 0; // go on with sector read
	}

	if (0 == state)
	{
		if (tradir->curlocation < tradir->cluster_end)
		{
			// cache the 512 byte of the directory sector
			// todo: handle cluster boundaries and fat chains
			// todo: handle long file names

			sectoraddr = (tradir->curlocation & sector_base_mask);

			if (bufaddr != sectoraddr)
			{
				// read the sector
				//TRACE("[r %08X]", uint32_t(sectoraddr));
				pstorman->AddTransaction(&stra, STRA_READ, sectoraddr,  &buf[0], 512);
				state = 1;
				return;
			}

			state = 2;  // the sector is already there
		}
		else // cluster end reached
		{
			// resolve FAT chain
			uint32_t curcluster = AddrToCluster(tradir->curlocation) - 1;
			TRACE("FAT find next cluster of %u\r\n", curcluster);

			FindNextCluster(curcluster);
			state = 5;
			return;
		}
	}
	else if (1 == state)  // wait for sector read
	{
		bufaddr = sectoraddr;
		state = 2;
	}

	if (2 == state) // the sector is in buf[]
	{
		bufendaddr = bufaddr + 512;

		while (tradir->curlocation < bufendaddr)
		{
			uint64_t dirlocation = tradir->curlocation;
			TFsFatDirEntry * pdire = (TFsFatDirEntry *)&buf[dirlocation & 0x1FF];
			if (0 == pdire->name[0])
			{
				// 0 at signalizes the end of the directory (and a free entry)
				FinishCurTra(FSRESULT_EOF);
				return;
			}

			tradir->curlocation += sizeof(TFsFatDirEntry);  // advance to the next location

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
				ConvertDirEntry(pdire, &tradir->fdata, dirlocation);
				FinishCurTra(0);
				return;
			}
		}

		// this sector is exhausted, load the next one.

		state = 0;
	}
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
