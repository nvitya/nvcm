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
 *  file:     hweth.h
 *  brief:    Ethernet vendor-independent definitions
 *  version:  1.00
 *  date:     2018-05-30
 *  authors:  nvitya
*/

#ifndef HWETH_H_PRE_
#define HWETH_H_PRE_

#include "hwpins.h"

#define HWETH_MAX_PACKET_SIZE  1524

class THwEth_pre
{
public: // settings

	uint8_t       phy_address = 0;

	uint8_t       mac_address[6] = {0x02, 0x00, 0x00, 0x00, 0x00, 0x00};

public:

	uint32_t      rx_desc_count = 0;
	uint32_t      tx_desc_count = 0;

public:
	bool          initialized = false;
	bool          link_up = false;

	uint32_t      recv_count = 0;
	uint32_t      recv_error_count = 0;

	uint8_t *     descmem = nullptr;
	uint32_t      descmemsize = 0;

	virtual ~THwEth_pre() { }
};

#endif // ndef HWETH_H_PRE_

#ifndef HWETH_PRE_ONLY

//-----------------------------------------------------------------------------

#ifndef HWETH_H_
#define HWETH_H_

#include "mcu_impl.h"

#if !defined(HWETH_IMPL)

class THwEth_noimpl : public THwEth_pre
{
public: // mandatory
	bool InitHw() { return false; }

};

#define HWETH_IMPL   THwEth_noimpl

#endif // ndef HWETH_IMPL

//-----------------------------------------------------------------------------

class THwEth : public HWETH_IMPL
{

};

#endif // ndef HWETH_H_ */

#else
  #undef HWETH_PRE_ONLY
#endif
