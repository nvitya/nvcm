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

void TFileSysFat::HandleInitState()
{
	if (!stra.completed)
	{
		return;
	}

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

void TFileSysFat::HandleTransactions()
{
}
