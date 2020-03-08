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
 *  file:     hwi2cslave_atsam_v2.h
 *  brief:    ATSAM v2 I2C / TWI Slave
 *  version:  1.00
 *  date:     2020-03-07
 *  authors:  nvitya
*/

#ifndef HWI2CSLAVE_ATSAM_V2_H_
#define HWI2CSLAVE_ATSAM_V2_H_

#define HWI2CSLAVE_PRE_ONLY
#include "hwi2cslave.h"

class THwI2cSlave_atsam_v2 : public THwI2cSlave_pre
{
public:
	SercomI2cs *   regs = nullptr;

	bool InitHw(int adevnum);

	void HandleIrq();

	void Run();

	uint8_t        runstate = 0;
};

#define HWI2CSLAVE_IMPL THwI2cSlave_atsam_v2

#endif // def HWI2CSLAVE_ATSAM_V2_H_
