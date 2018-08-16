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
 *  file:     hwqspi.h
 *  brief:    Internal QSPI/SPIFI vendor-independent definitions
 *  version:  1.00
 *  date:     2018-02-10
 *  authors:  nvitya
*/

#ifndef _HWQSPI_H_PRE_
#define _HWQSPI_H_PRE_

#include "platform.h"
#include "hwpins.h"
#include "hwdma.h"
#include "errors.h"

// line format S = single line, M = multi line
#define QSPICM_SSS        0x00000000  // S command, S address + dummy, S data (default)
#define QSPICM_SSM        0x00000800  // S command, S address + dummy, M data
#define QSPICM_SMM        0x00000E00  // S command, M address + dummy, M data
#define QSPICM_MMM        0x00000F00  // M command, M address + dummy, M data

// Addres byte count
#define QSPICM_ADDR       0x00080000  // send address with the default size
#define QSPICM_ADDR0      0x00000000  // do not send address (default)
#define QSPICM_ADDR1      0x00010000
#define QSPICM_ADDR2      0x00020000
#define QSPICM_ADDR3      0x00030000
#define QSPICM_ADDR4      0x00040000
#define QSPICM_ADDR_MASK  0x000F0000

// dummy byte count
#define QSPICM_DUMMY      0x00800000  // send dummy with the default size
#define QSPICM_DUMMY0     0x00000000  // do not send dummy (default)
#define QSPICM_DUMMY1     0x00100000
#define QSPICM_DUMMY2     0x00200000
#define QSPICM_DUMMY3     0x00300000
#define QSPICM_DUMMY4     0x00400000
#define QSPICM_DUMMY_MASK 0x00F00000

class THwQspi_pre
{
public:	// settings
	bool 					 initialized = false;

	unsigned       speed = 8000000;  // default speed = 8MHz
	bool           idleclk_high = true;

	int            addrlen = 3;
	unsigned       dummydata = 0;
	unsigned       dummysize = 1; // default dummy size = 1 byte

	unsigned       multi_line_count = 2;  // 4 = quad, 2 = dual mode, 1 = disable multi line mode

public: // Required HW resources
	THwDmaChannel  txdma;
	THwDmaChannel  rxdma;

	THwDmaTransfer xfer;

public: // run state
	bool           istx = false;
	bool           dmaused = false;
	uint8_t *      dataptr = nullptr;
	unsigned       datalen = 0;
	unsigned       remainingbytes = 0;

	bool           busy = false;

public:
	virtual        ~THwQspi_pre() { } // virtual destructor to avoid compiler warning

	virtual bool   InitInterface() { return true; }
};

#endif // ndef HWQSPI_H_PRE_

#ifndef HWQSPI_PRE_ONLY

//-----------------------------------------------------------------------------

#ifndef HWQSPI_H_
#define HWQSPI_H_

#include "mcu_impl.h"

#ifndef HWQSPI_IMPL

//#warning "HWQSPI is not implemented!"

class THwQspi_noimpl : public THwQspi_pre
{
public: // mandatory
	bool Init()                   { return false; }

	int  StartReadData(unsigned acmd, unsigned address, void * dstptr, unsigned len)   { return ERROR_NOTIMPL; }
	int  StartWriteData(unsigned acmd, unsigned address, void * srcptr, unsigned len)  { return ERROR_NOTIMPL; }
	void Run()  { }
};

#define HWQSPI_IMPL   THwQspi_noimpl

#endif // ndef HWQSPI_IMPL

//-----------------------------------------------------------------------------

class THwQspi : public HWQSPI_IMPL
{
public:
	bool Finished();
	int WaitFinish();
};

#endif // HWQSPI_H_

#else
  #undef HWQSPI_PRE_ONLY
#endif
