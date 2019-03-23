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
 *  file:     hwi2c_xmc.h
 *  brief:    XMC I2C
 *  version:  1.00
 *  date:     2019-03-23
 *  authors:  nvitya
*/

#ifndef HWI2C_XMC_H_
#define HWI2C_XMC_H_

#define HWI2C_PRE_ONLY
#include "hwi2c.h"

#define HWI2C_SDA_DX0A  0
#define HWI2C_SDA_DX0B  1
#define HWI2C_SDA_DX0C  2
#define HWI2C_SDA_DX0D  3
#define HWI2C_SDA_DX0E  4
#define HWI2C_SDA_DX0F  5
#define HWI2C_SDA_DX0G  6
#define HWI2C_SDA_DX0H  7

#define HWI2C_SCL_DX1A  0
#define HWI2C_SCL_DX1B  1
#define HWI2C_SCL_DX1C  2
#define HWI2C_SCL_DX1D  3
#define HWI2C_SCL_DX1E  4
#define HWI2C_SCL_DX1F  5
#define HWI2C_SCL_DX1G  6
#define HWI2C_SCL_DX1H  7


class THwI2c_xmc : public THwI2c_pre
{
public:
	int                 usicnum = 0;
	int                 chnum = 0;
	int                 inputpin_scl = 0;
	int                 inputpin_sda = 0;

	USIC_CH_TypeDef *   regs = nullptr;

	bool Init(int ausicnum, int achnum, int ainputpin_scl, int ainputpin_sda);

	int  StartReadData(uint8_t  adaddr, unsigned aextra, void * dstptr, unsigned len);
	int  StartWriteData(uint8_t adaddr, unsigned aextra, void * srcptr, unsigned len);
	void Run();

	unsigned       runstate = 0;
	uint8_t        devaddr = 0;
	uint8_t        extradata[4];
	unsigned       extracnt = 0;
	unsigned       extraremaining = 0;
};

#define HWI2C_IMPL THwI2c_xmc

#endif // def HWI2C_XMC_H_
