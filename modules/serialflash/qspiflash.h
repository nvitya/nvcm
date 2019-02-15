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
 *  file:     qspiflash.h
 *  brief:    QSPI Flash Memory implementation
 *  version:  1.00
 *  date:     2018-02-10
 *  authors:  nvitya
*/


#ifndef QSPIFLASH_H_
#define QSPIFLASH_H_

#include "platform.h"
#include "hwpins.h"
#include "hwqspi.h"
#include "hwdma.h"
#include "serialflash.h"
#include "errors.h"

class TQspiFlash : public TSerialFlash
{
public:
	typedef TSerialFlash super;

	// Required HW resources
	THwQspi        qspi;

	// overrides
	virtual bool   InitInherited();
	virtual bool   ReadIdCode();
	virtual void   Run();

protected:
	// smaller buffers for simple things
	unsigned char  txbuf[8]  __attribute__((aligned(4)));
	unsigned char  rxbuf[8]  __attribute__((aligned(4)));

	unsigned       curcmdlen = 0;

	unsigned       statusreg;

	void StartReadStatus();
	void StartWriteEnable();
};

#endif /* QSPIFLASH_H_ */
